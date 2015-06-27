/**
 @file
 chain.device - 
 
 @ingroup    maxchain
 */

#include "ext.h"
#include "ext_common.h"
#include "ext_dictobj.h"
#include "ext_database.h"

#include "messages.h"
#include "queries.h"
#include "chainlib.h"

typedef struct chain_device
{
    t_object s_obj;
    t_systhread s_systhread_setup;
    int s_setup_cancel;
    t_symbol *s_site_name;
    t_symbol *s_device_name;
    t_dictionary *s_dictionary;
    t_database *s_db;
    t_object *s_view;
    long s_live_flag;
    void *s_outlet;
    void *s_outlet2;
} t_chain_device;

void *chain_device_new(t_symbol *s, long argc, t_atom *argv);
void chain_device_free(t_chain_device *x);
void chain_device_int(t_chain_device *x, long n);
void chain_device_bang(t_chain_device *x);
void chain_device_metric(t_chain_device *x, t_symbol *metric);
void chain_device_sensors(t_chain_device *x);
void chain_device_geoLocation(t_chain_device *x);
void chain_device_location(t_chain_device *x);
void chain_device_notify(t_chain_device *x, t_symbol *s, t_symbol *msg, void *sender, void *data);
void chain_device_set_site_name(t_chain_device *x, void *attr, long argc, t_atom *argv);
void chain_device_set_device_name(t_chain_device *x, void *attr, long argc, t_atom *argv);
void *chain_device_setup_threadproc(t_chain_device *x);

void chain_device_set_query(t_chain_device *x);
void chain_device_send_all(t_chain_device *x);
void chain_device_send_sensor(t_chain_device *x, const char *href);
int chain_device_get_dict(t_chain_device *x);


static t_class *s_chain_device_class = NULL;

t_symbol *ps_name, *ps_db, *ps_maxchain, *ps_location, *ps_geoLocation, *ps_sensors;

int C74_EXPORT main(void)
{
    t_class *c;
    
    c = class_new("chain.device", (method)chain_device_new, (method)chain_device_free,
                  sizeof(t_chain_device), (method)0L, A_GIMME, 0);
    
    class_addmethod(c, (method)chain_device_bang, "bang", 0);
    class_addmethod(c, (method)chain_device_int, "int", A_LONG, 0);
    class_addmethod(c, (method)chain_device_metric, "metric", A_SYM, 0);
    class_addmethod(c, (method)chain_device_notify, "notify", A_CANT, 0);
    class_addmethod(c, (method)chain_device_sensors, "sensors", 0);
    class_addmethod(c, (method)chain_device_geoLocation, "geoLocation", 0);
    class_addmethod(c, (method)chain_device_location, "location", 0);

    CLASS_ATTR_SYM(c, "name", 0, t_chain_device, s_site_name);
    CLASS_ATTR_ACCESSORS(c, "name", NULL, (method)chain_device_set_site_name);

    CLASS_ATTR_SYM(c, "device_name", 0, t_chain_device, s_device_name);
    CLASS_ATTR_ACCESSORS(c, "device_name", NULL, (method)chain_device_set_device_name);

    CLASS_ATTR_LONG(c, "live", 0, t_chain_device, s_live_flag);
    
    class_register(CLASS_BOX, c);
    s_chain_device_class = c;

    ps_name = gensym("name");
    ps_db = gensym("db");
    ps_maxchain = gensym("maxchain");
    ps_sensors = gensym("sensors");
    ps_location = gensym("location");
    ps_geoLocation = gensym("geoLocation");
    
    return 0;
}

void *chain_device_new(t_symbol *s, long argc, t_atom *argv)
{
    t_chain_device *x = (t_chain_device *)object_alloc(s_chain_device_class);

    long attrstart = attr_args_offset(argc, argv);
    t_symbol *site_name = NULL;

    if (attrstart && atom_gettype(argv) == A_SYM)
        site_name = atom_getsym(argv);

    if (!x->s_site_name) {
        if (site_name)
            object_attr_setsym(x, ps_name, site_name);
    }

    attr_args_process(x, argc, argv);

    x->s_outlet2 = outlet_new(x, NULL);
    x->s_outlet = outlet_new(x, NULL);
    x->s_live_flag = 1;
    x->s_view = NULL;
    x->s_setup_cancel = false;
    return x;
}

void chain_device_set_site_name(t_chain_device *x, void *attr, long argc, t_atom *argv)
{
    t_symbol *site_name = atom_getsym(argv);
    if (!x->s_site_name || x->s_site_name!=site_name){
        x->s_site_name = site_name; 

        if (x->s_systhread_setup == NULL){
            systhread_create((method) chain_device_setup_threadproc, x,
                             0, 0, 0, &x->s_systhread_setup);
        }
    }
}

void chain_device_set_device_name(t_chain_device *x, void *attr, long argc, t_atom *argv)
{
    t_symbol *device_name = atom_getsym(argv);
    if (!x->s_device_name || x->s_device_name!=device_name){
        x->s_device_name = device_name;
    }
}

int chain_device_get_dict(t_chain_device *x)
{
    int err = 0;

    if (x->s_dictionary)
        dictobj_release(x->s_dictionary);
    x->s_dictionary = dictobj_findregistered_retain(x->s_site_name);

    if (!x->s_dictionary){
        err = 1;
    }
    dictionary_getobject(x->s_dictionary, ps_db, &x->s_db);
    // Attach object to site
    object_subscribe(ps_maxchain, x->s_site_name, NULL, x);

    return err;
}

void chain_device_free(t_chain_device *x)
{
    if (x->s_dictionary)
        dictobj_release(x->s_dictionary);

    unsigned int ret;
    if (x->s_systhread_setup){
        x->s_setup_cancel = true;
        systhread_join(x->s_systhread_setup, &ret);
        x->s_systhread_setup = NULL;
    }
}

void chain_device_send_sensor(t_chain_device *x, const char *href){
    if(!x->s_db){
        chain_error("No DB!");
        return;
    }

    t_db_result *db_result = NULL;

    query_data_by_sensor_href(x->s_db, href, &db_result);

    if (!db_result_numrecords(db_result)){
        chain_error("No sensor found");
        return;
    }

    double value = db_result_float(db_result, 0, 0);
    const char *timestamp = db_result_string(db_result, 0, 1);
    const char *metric_name = db_result_string(db_result, 0, 2);

    t_atom av[2];
    short ac = 2;
    atom_setsym(av, gensym(metric_name));
    atom_setfloat(av+1, value);

    outlet_list(x->s_outlet, 0L, ac, av);
}

void chain_device_send_metric(t_chain_device *x, t_symbol *metric){
    if(!x->s_db){
        chain_error("No DB!");
        return;
    }

    if(!x->s_device_name){
        chain_error("No device name set");
        return;
    }

    t_db_result *db_result = NULL;

    query_data_by_device_name_metric_name(x->s_db, x->s_device_name->s_name, metric->s_name, &db_result);

    if (!db_result_numrecords(db_result)){
        chain_error("No such metric for this device");
        return;
    }

    double value = db_result_float(db_result, 0, 0);
    const char *timestamp = db_result_string(db_result, 0, 1);

    t_atom av[2];
    short ac = 2;
    atom_setsym(av, metric);
    atom_setfloat(av+1, value);

    outlet_list(x->s_outlet, 0L, ac, av);
}

void chain_device_send_all(t_chain_device *x){
    if(!x->s_db){
        chain_error("No DB!");
        return;
    }

    if(!x->s_device_name){
        chain_error("No device name set");
        return;
    }

    t_db_result *db_result = NULL;

    query_data_by_device_name(x->s_db, x->s_device_name->s_name, &db_result);

    long numrecords = db_result_numrecords(db_result);

    for (int i=0; i<numrecords; i++){
        const char *timestamp, *metric_name;
        double value;
        timestamp = db_result_string(db_result, i, 1);
        value = db_result_float(db_result, i, 0);
        metric_name = db_result_string(db_result, i, 2);

        t_atom av[2];
        short ac = 2;
        atom_setsym(av, gensym(metric_name));
        atom_setfloat(av+1, value);

        outlet_list(x->s_outlet, 0L, ac, av);
    }
}

void chain_device_notify(t_chain_device *x, t_symbol *s, t_symbol *msg,
                         void *sender, void *data)
{
    if (s == x->s_site_name && x->s_live_flag && msg == x->s_device_name) {
        const char *href = (const char *)data;
        chain_device_send_sensor(x, href);
    }
}

void chain_device_int(t_chain_device *x, long n)
{
}

void chain_device_bang(t_chain_device *x)
{
    chain_device_send_all(x);
}

void chain_device_metric(t_chain_device *x, t_symbol *metric)
{
    chain_device_send_metric(x, metric);
}

void chain_device_sensors(t_chain_device *x)
{
    if(!x->s_db){
        chain_error("No DB!");
        return;
    }
    if(!x->s_device_name){
        chain_error("No device name set");
        return;
    }
    t_db_result *db_result = NULL;
    long numrecords;

    query_list_metrics_by_device_name(x->s_db, x->s_device_name->s_name, &db_result); 

    numrecords = db_result_numrecords(db_result);
    if(!numrecords){
        chain_error("No sensors were found!");
        return;
    }

    t_atom av[numrecords];
    for (int i=0; i<numrecords; i++){
        const char *name;
        name = db_result_string(db_result, i, 0);
        atom_setsym(av+i, gensym(name));
    }

    outlet_anything(x->s_outlet2, ps_sensors, numrecords, av);
}

void chain_device_geoLocation(t_chain_device *x)
{
    if(!x->s_db){
        chain_error("No DB!");
        return;
    }
    if(!x->s_device_name){
        chain_error("No device name set");
        return;
    }
    t_db_result *db_result = NULL;
    query_get_device_location(x->s_db, x->s_device_name->s_name, &db_result);

    double lat = db_result_float(db_result, 0, 0);
    double lon = db_result_float(db_result, 0, 1);
    double ele = db_result_float(db_result, 0, 2);

    t_atom av[2];
    short ac = 2;
    atom_setfloat(av, lat);
    atom_setfloat(av+1, lon);
    outlet_anything(x->s_outlet2, ps_geoLocation, ac, av);
}

void chain_device_location(t_chain_device *x)
{
    if(!x->s_db){
        chain_error("No DB!");
        return;
    }
    if(!x->s_device_name){
        chain_error("No device name set");
        return;
    }
    t_db_result *db_result = NULL;
    query_get_device_location(x->s_db, x->s_device_name->s_name, &db_result);

    double f_x = db_result_float(db_result, 0, 3);
    double f_z = db_result_float(db_result, 0, 4);

    t_atom av[2];
    short ac = 2;
    atom_setfloat(av, f_x);
    atom_setfloat(av+1, f_z);
    outlet_anything(x->s_outlet2, ps_location, ac, av);
}

void *chain_device_setup_threadproc(t_chain_device *x)
{
    int err=1;
    while (err){
        err = chain_device_get_dict(x);
        systhread_sleep(1000);

        if (x->s_setup_cancel)
            break;
    }

    systhread_exit(0);
    return NULL;
}