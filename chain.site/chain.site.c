/**
 @file
 chain.site - a simple chain_site object
 
 @ingroup    maxchain
 */
#include "chain.site.h"

#include "libwebsockets.h"

#include "messages.h"
#include "queries.h"
#include "chainquery.h"
#include "chainwebsocket.h"

#define URL_SIZE 1024
#define LOOKAHEAD_TIME 1000
#define CHUNK_LENGTH 2000
#define LOOKAHEAD_SLEEP_TIME 1000
typedef struct {
    t_chain_site *x;
    t_chain_event *e;
    void *clk;
} t_chain_site_and_event;

// Create and Destroy
void *chain_site_new(t_symbol *s, long argc, t_atom *argv);
void chain_site_free(t_chain_site *x);

// Methods
void chain_site_load(t_chain_site *x);
void chain_site_bang(t_chain_site *x);
void chain_site_int(t_chain_site *x, long n);
void chain_site_start(t_chain_site *x, t_symbol *s, long argc, t_atom *argv);
void chain_site_stop(t_chain_site *x);

// Attibute Setters
void chain_site_release_site(t_chain_site *x);
void chain_site_set_site_name(t_chain_site *x, t_symbol *site_name);
void chain_site_set_url(t_chain_site *x, void *attr, long argc, t_atom *argv);

// Threads
void *chain_site_load_threadproc(t_chain_site *x);
void *chain_site_play_threadproc(t_chain_site *x);

void chain_site_play_historical(t_chain_site *x);
void chain_site_play_live(t_chain_site *x);
void chain_site_clock_method(t_chain_site_and_event *xe);
void chain_site_free_update(t_chain_site_and_event *xe);

static t_class *s_chain_site_class = NULL;

t_symbol *ps_url, *ps_db, *ps_maxchain;

int C74_EXPORT main(void)
{
    ps_url = gensym("url");
    ps_db = gensym("db");
    ps_maxchain = gensym("maxchain");
    
    t_class *c;

    c = class_new("chain.site", (method)chain_site_new, (method)chain_site_free, \
                  sizeof(t_chain_site), (method)NULL, A_GIMME, 0);
    
    class_addmethod(c, (method)chain_site_load, "load", 0);
    class_addmethod(c, (method)chain_site_int, "int", A_LONG, 0);
    class_addmethod(c, (method)chain_site_bang, "bang", 0);
    class_addmethod(c, (method)chain_site_start, "start", A_GIMME, 0);
    class_addmethod(c, (method)chain_site_stop, "stop", 0);

    CLASS_ATTR_SYM(c, "name", ATTR_SET_OPAQUE_USER, t_chain_site, s_site_name);

    CLASS_ATTR_SYM(c, "url", 0, t_chain_site, s_url);
    CLASS_ATTR_ACCESSORS(c, "url", NULL, chain_site_set_url);

    class_register(CLASS_BOX, c);
    
    s_chain_site_class = c;

    return 0;
}

void *chain_site_new(t_symbol *s, long argc, t_atom *argv)
{
    t_chain_site *x = (t_chain_site *)object_alloc(s_chain_site_class);

    long attrstart = attr_args_offset(argc, argv);
    t_symbol *site_name = NULL;
    if (attrstart && atom_gettype(argv) == A_SYM){
        site_name = atom_getsym(argv);
    }
    if (!site_name){
        site_name = gensym("default_site");
    }
    chain_site_set_site_name(x, site_name);

    x->s_outlet_busy = outlet_new(x, NULL);
    x->s_outlet = outlet_new(x, NULL);
    x->s_systhread_load = NULL;
    x->s_systhread_play = NULL;
    x->s_play_cancel = false;
    x->s_historical_ts = 1.0;
    x->s_historical_start = 1436877533; //Arbitrary choice
    x->s_historical_scheduler = false;

    attr_args_process(x, argc, argv);

    return x;
}

void chain_site_free(t_chain_site *x)
{
    unsigned int ret;
    if (x->s_systhread_load){
        systhread_join(x->s_systhread_load, &ret);
        x->s_systhread_load = NULL;
    }

    unsigned int ret2;
    if (x->s_systhread_play){
        x->s_play_cancel = true;
        systhread_join(x->s_systhread_play, &ret2);
        x->s_systhread_play = NULL;
    }
    chain_site_release_site(x);    
}

void chain_site_set_site_name(t_chain_site *x, t_symbol *site_name)
{
    // Check if site_name already being used
    t_dictionary *temp = dictobj_findregistered_retain(site_name);
    if(temp){
        dictobj_release(temp);
        chain_error("Chain site %s already defined", site_name->s_name);
        return;
    }

    // Create and register dictionary
    x->s_dictionary = dictionary_new();
    x->s_dictionary = dictobj_register(x->s_dictionary, &site_name);
    x->s_site_name = site_name;


    // Create database and add to dictionary
    db_open(site_name, NULL, &x->s_db);
    query_init_database(x->s_db);

    dictionary_appendobject(x->s_dictionary, ps_db, x->s_db);

    // Register object
    x->s_reg_ptr = object_register(ps_maxchain, site_name, x);
}

void chain_site_release_site(t_chain_site *x)
{
    if (x->s_dictionary){
        object_free(x->s_dictionary);
        x->s_dictionary = NULL;
        object_unregister(x->s_reg_ptr);
    }
}

void chain_site_set_url(t_chain_site *x, void *attr, long argc, t_atom *argv)
{
    t_max_err err = MAX_ERR_NONE;

    t_symbol *url_sym = atom_getsym(argv);
    
    if(!x->s_dictionary){
        chain_error("Cannot set url, bad dictionary");
        return;
    }

    if (!x->s_url || !url_sym || x->s_url!=url_sym){
        err = dictionary_deleteentry(x->s_dictionary, ps_url);
        err = dictionary_appendsym(x->s_dictionary, ps_url, url_sym);
        x->s_url = url_sym;
        char url_str[URL_SIZE];
        strncpy_zero(url_str, url_sym->s_name, sizeof(url_str));
    }
}

void chain_site_load(t_chain_site *x)
{
    if(!x->s_db){
        chain_error("database not open");
        return;
    }
    if(!x->s_url){
        chain_error("no url set");
        return;
    }
    if (x->s_systhread_load == NULL){
        systhread_create((method) chain_site_load_threadproc, x, 0, 0, 0, &x->s_systhread_load);
    } else {
        chain_error("BUSY");
    }
}

int chain_site_update_sensors(t_chain_site *x, t_chain_event *e)
{
    t_db_result *db_result = NULL;
    query_update_sensor(x->s_db, e->s_href, e->s_timestamp, e->s_value, &db_result);

    if (!db_result_numrecords(db_result)){
        chain_error("Failed to aquire device name");
        return 1;
    }
    const char *device_name = db_result_string(db_result, 0, 0);

    t_symbol *device_name_sym;
    device_name_sym = gensym(device_name);
    object_notify(x, device_name_sym, (void *)e->s_href);

    return 0;
}

void chain_site_bang(t_chain_site *x)
{
}

void chain_site_int(t_chain_site *x, long n)
{
}

void chain_site_start(t_chain_site *x, t_symbol *s, long argc, t_atom *argv)
{
    if(!x->s_db){
        chain_error("database not open");
        return;
    }
    if(!x->s_url){
        chain_error("no url set");
        return;
    }
    if (x->s_systhread_play != NULL){
        chain_site_stop(x);
    }

    // Set up play environment
    if (argc == 0){
        x->s_live = 1;
        chain_info("Starting live");
    } else if (argc == 1){
        if(atom_gettype(argv) != A_LONG){
            chain_error("Bad start time");
            return;
        }
        x->s_live = 0;
        x->s_historical_start = atom_getlong(argv);
        x->s_historical_ts = 1.0;
        chain_info("Starting historical from %ld", atom_getlong(argv));
    } else if (argc == 2){
        if(atom_gettype(argv) != A_LONG){
            chain_error("Bad start time");
            return;
        }
        if(atom_gettype(argv + 1) != A_FLOAT){
            chain_error("Bad time scale factor");
            return;
        }
        x->s_live = 0;
        x->s_historical_start = atom_getlong(argv);
        x->s_historical_ts = atom_getfloat(argv + 1);
        chain_info("Starting historical from %ld at %f", atom_getlong(argv), atom_getfloat(argv+1));
    } else {
        chain_error("start expected with no more than 2 arguments, got %ld", argc);
        return;
    }
    systhread_create((method) chain_site_play_threadproc, x, 0, 0, 0, &x->s_systhread_play);
}

void chain_site_stop(t_chain_site *x)
{
    unsigned int ret;
    if (x->s_systhread_play != NULL){
        x->s_play_cancel = true;
        systhread_join(x->s_systhread_play, &ret);
        x->s_systhread_play = NULL;
        x->s_play_cancel = false;
    }
}

void *chain_site_load_threadproc(t_chain_site *x)
{
    const char *wshref;
    chain_load_summary(x->s_url->s_name, x->s_db); 
    chain_get_websocket(x->s_url->s_name, &wshref);
    outlet_bang(x->s_outlet_busy);

    if(!wshref){
        chain_error("Could not get websocket address.");
    } else {
        x->s_wshref = gensym(wshref);
        free((void *)wshref);
    }

    x->s_systhread_load = NULL;
    systhread_exit(0);
    return NULL;
}

// Run through websocket .. see chainwebsocket.c
void chain_site_play_live(t_chain_site *x){
    while (!x->s_wshref){
        if (x->s_play_cancel){
            return;
        }
        systhread_sleep(1000);
    }
    struct libwebsocket_context *context = chain_websocket_connect(x);

    if(!context){
        return;
    }

    int n = 0;
    while(n>=0){
        n = libwebsocket_service(context, 2000);
        outlet_int(x->s_outlet, local_now());
        if (x->s_play_cancel)
            break;
    }
}

long chain_site_pseudo_now(t_chain_site *x){
    if (!x->s_historical_scheduler){
        return x->s_historical_start;
    }
    long t = gettime();
    t -= x->s_historical_scheduler;
    double elapsed_seconds = ((double) t) * x->s_historical_ts / 1000.;
    return x->s_historical_start + (long)elapsed_seconds;
}

void chain_site_schedule_update(t_chain_site *x, t_chain_event *e)
{
    if(!x->s_historical_clklist){
        chain_error("No linked list to take clocks");
        free(e);
        return;
    }
    long pseudo_now = chain_site_pseudo_now(x);
    outlet_int(x->s_outlet, pseudo_now);
    long delay = ((long)e->s_time - pseudo_now) * 1000;

    if (delay < 0){
        free(e);
        return;
    }

    t_chain_site_and_event *xe = malloc(sizeof(*xe));
    xe->x = x;
    xe->e = e;
    xe->clk = clock_new(xe, (method)chain_site_clock_method);
    clock_delay(xe->clk, delay);
    linklist_append(x->s_historical_clklist, xe);
}

void chain_site_clock_method(t_chain_site_and_event *xe){
    chain_site_update_sensors(xe->x, xe->e);
    linklist_chuckobject(xe->x->s_historical_clklist, xe);
    chain_site_free_update(xe);
}

void chain_site_free_update(t_chain_site_and_event *xe){
    clock_unset(xe->clk);
    freeobject((t_object *)xe->clk);
    free(xe->e);
    free(xe);
}

void chain_site_play_historical(t_chain_site *x){
    // Initialize
    x->s_historical_scheduler = gettime();
    x->s_historical_clklist = linklist_new();

    time_t highest_time_checked = x->s_historical_start;

    t_db_result *db_result = NULL;
    query_list_sensors(x->s_db, &db_result);
    long num_records = db_result_numrecords(db_result);

    // Loop
    while(1)
    {
        if(x->s_play_cancel){
            break;
        }
        long pseudo_now = chain_site_pseudo_now(x);
        outlet_int(x->s_outlet, pseudo_now);
        if (pseudo_now >= local_now()){
            break;
        }
        if (pseudo_now < highest_time_checked - LOOKAHEAD_TIME){
            systhread_sleep(LOOKAHEAD_SLEEP_TIME);
            continue;
        }

        time_t start = highest_time_checked;
        time_t end = highest_time_checked + CHUNK_LENGTH;

        // For each sensor
        for (int sensor_idx=0; sensor_idx < num_records; sensor_idx++){
            if (x->s_play_cancel){
                break;
            }
            const char *url = db_result_string(db_result, sensor_idx, 0);
            long num_events = 0;
            t_chain_event *events;
            chain_get_data(url, start, end, &events, &num_events);
            for (int datum_idx=0; datum_idx<num_events; datum_idx++){
                // put event on heap
                t_chain_event *e = malloc(sizeof(*e));
                memcpy(e, (events + datum_idx), sizeof(*e));

                // schedule event
                chain_site_schedule_update(x, e);
            }
            free(events);
        }

        highest_time_checked = end;
    }

    // Clean up
    linklist_funall(x->s_historical_clklist, (method)chain_site_free_update, NULL);
    linklist_chuck(x->s_historical_clklist);
}

// Play data
void *chain_site_play_threadproc(t_chain_site *x)
{
    if (x->s_live){
        chain_site_play_live(x);
    } else {
        chain_site_play_historical(x);
    }
    
    systhread_exit(0);
    return NULL;
}

