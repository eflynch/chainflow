/**
 @file
 chain.site - a simple chain_site object
 
 @ingroup    maxchain
 */
#include "chain.site.h"

#include "libwebsockets.h"

#include "messages.h"
#include "queries.h"
#include "chainsummary.h"
#include "chainwebsocket.h"

#define URL_SIZE 1024

void *chain_site_new(t_symbol *s, long argc, t_atom *argv);
void chain_site_free(t_chain_site *x);
void chain_site_load(t_chain_site *x);
void chain_site_bang(t_chain_site *x);
void chain_site_int(t_chain_site *x, long n);
void chain_site_set_site_name(t_chain_site *x, void *attr, long argc, t_atom *argv);
void chain_site_set_url(t_chain_site *x, void *attr, long argc, t_atom *argv);
void *chain_site_load_threadproc(t_chain_site *x);
void *chain_site_play_threadproc(t_chain_site *x);

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

    CLASS_ATTR_SYM(c, "name", 0, t_chain_site, s_site_name);
    CLASS_ATTR_ACCESSORS(c, "name", NULL, chain_site_set_site_name);

    CLASS_ATTR_SYM(c, "url", 0, t_chain_site, s_url);
    CLASS_ATTR_ACCESSORS(c, "url", NULL, chain_site_set_url);

    class_register(CLASS_BOX, c);
    
    s_chain_site_class = c;
    
    return 0;
}

void chain_site_bang(t_chain_site *x)
{
}

void chain_site_int(t_chain_site *x, long n)
{
    if(!x->s_db){
        chain_error("database not open");
        return;
    }
    if(!x->s_url){
        chain_error("no url set");
        return;
    }
    unsigned int ret;
    if (x->s_systhread_play != NULL && n == 0){
        x->s_play_cancel = true;
        systhread_join(x->s_systhread_play, &ret);
        x->s_systhread_play = NULL;
        x->s_play_cancel = false;
        return;
    }

    if (x->s_systhread_play == NULL){
        systhread_create((method) chain_site_play_threadproc, x, 0, 0, 0, &x->s_systhread_play);
    } else {
        chain_error("Already playing");
    }
}

void *chain_site_new(t_symbol *s, long argc, t_atom *argv)
{
    t_chain_site *x = (t_chain_site *)object_alloc(s_chain_site_class);

    long attrstart = attr_args_offset(argc, argv);
    t_symbol *site_name = NULL;

    if (attrstart && atom_gettype(argv) == A_SYM)
        site_name = atom_getsym(argv);

    if (!x->s_site_name) {
        if (site_name)
            object_attr_setsym(x, gensym("name"), site_name);
        else
            object_attr_setsym(x, gensym("name"), symbol_unique());
    }

    attr_args_process(x, argc, argv);

    x->s_outlet_busy = outlet_new(x, NULL);
    x->s_outlet = outlet_new(x, NULL);
    x->s_systhread_load = NULL;
    x->s_systhread_play = NULL;
    x->s_play_cancel = false;

    return x;
}

void chain_site_set_site_name(t_chain_site *x, void *attr, long argc, t_atom *argv)
{
    t_symbol *site_name = atom_getsym(argv);
    if (!x->s_site_name || !site_name || x->s_site_name!=site_name){
        if (x->s_dictionary)
            object_free(x->s_dictionary);
        t_dictionary *temp = dictobj_findregistered_retain(site_name);
        if(temp){
            dictobj_release(temp);
            chain_error("Chain site %s already defined", site_name->s_name);
            return;
        }
        x->s_dictionary = dictionary_new();
        x->s_dictionary = dictobj_register(x->s_dictionary, &site_name);
        x->s_site_name = site_name;

        if(x->s_db)
            db_close(&x->s_db);
        db_open(site_name, NULL, &x->s_db);
        query_init_database(x->s_db);

        dictionary_deleteentry(x->s_dictionary, ps_db);
        dictionary_appendobject(x->s_dictionary, ps_db, x->s_db);

        // Register object
        object_register(ps_maxchain, site_name, x);
    }
}

void chain_site_set_url(t_chain_site *x, void *attr, long argc, t_atom *argv)
{
    t_symbol *url_sym = atom_getsym(argv);

    if (!x->s_url || !url_sym || x->s_url!=url_sym){
        dictionary_deleteentry(x->s_dictionary, ps_url);
        dictionary_appendsym(x->s_dictionary, ps_url, url_sym);
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
        outlet_int(x->s_outlet_busy, 1);
        systhread_create((method) chain_site_load_threadproc, x, 0, 0, 0, &x->s_systhread_load);
    } else {
        chain_error("BUSY");
    }
}

void chain_site_free(t_chain_site *x)
{
    if (x->s_dictionary)
        object_free(x->s_dictionary);

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
}


void *chain_site_load_threadproc(t_chain_site *x)
{
    const char *wshref;
    chain_load_summary(x->s_url->s_name, x->s_db); 
    chain_load_websocket(x->s_url->s_name, &wshref);
    outlet_int(x->s_outlet_busy, 0);

    if(!wshref){
        chain_error("Could not get websocket address.");
    } else {
        x->s_wshref = gensym(wshref);
        free(wshref);
    }

    x->s_systhread_load = NULL;
    systhread_exit(0);
    return NULL;
}

void *chain_site_play_threadproc(t_chain_site *x)
{
    struct libwebsocket_context *context = chain_websocket_connect(x);

    if(!context){
        systhread_exit(0);
        return NULL;
    }

    int n = 0;
    while(n>=0){
        n = libwebsocket_service(context, 2000);
        if (x->s_play_cancel)
            break;
    }

    systhread_exit(0);
    return NULL;
}


