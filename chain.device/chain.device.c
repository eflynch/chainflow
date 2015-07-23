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
#include "chainevent.h"
#include "chainworker.h"
#include "chainmath.h"
#include "chainquery.h"

typedef struct chain_device
{
    t_chain_worker s_worker;
    t_symbol *s_device_name;
    long s_autoupdate;
    long s_deviation;
    void *s_outlet;
    void *s_outlet2;
} t_chain_device;

// Create + Destroy
void *chain_device_new(t_symbol *s, long argc, t_atom *argv);
void chain_device_free(t_chain_device *x);

// Attribute Setters
void chain_device_set_device_name(t_chain_device *x, void *attr, long argc, t_atom *argv);

// Methods
void chain_device_int(t_chain_device *x, long n);
void chain_device_bang(t_chain_device *x);
void chain_device_metric(t_chain_device *x, t_symbol *metric);
void chain_device_metrics(t_chain_device *x);
void chain_device_geoLocation(t_chain_device *x);
void chain_device_location(t_chain_device *x);
void chain_device_data(t_chain_device *x, t_symbol *metric, long start, long end);

// Helpers
void chain_device_send_all(t_chain_device *x);
void chain_device_send_sensor(t_chain_device *x, const char *href);

// Notify
void chain_device_notify(t_chain_device *x, t_symbol *s, t_symbol *msg, void *sender, void *data);


static t_class *s_chain_device_class = NULL;

t_symbol *ps_name, *ps_location, *ps_geoLocation, *ps_metrics;


int C74_EXPORT main(void)
{
    t_class *c;
    
    c = class_new("chain.device", (method)chain_device_new, (method)chain_device_free,
                  sizeof(t_chain_device), (method)0L, A_GIMME, 0);
    
    class_addmethod(c, (method)chain_device_bang, "bang", 0);
    class_addmethod(c, (method)chain_device_int, "int", A_LONG, 0);
    class_addmethod(c, (method)chain_device_metric, "metric", A_SYM, 0);
    class_addmethod(c, (method)chain_device_notify, "notify", A_CANT, 0);
    class_addmethod(c, (method)chain_device_metrics, "metrics", 0);
    class_addmethod(c, (method)chain_device_geoLocation, "geoLocation", 0);
    class_addmethod(c, (method)chain_device_location, "location", 0);
    class_addmethod(c, (method)chain_device_data, "data", A_SYM, A_LONG, A_LONG);

    CLASS_ATTR_SYM(c, "name", ATTR_SET_OPAQUE_USER, t_chain_device, s_worker.s_site_name);

    CLASS_ATTR_SYM(c, "device_name", 0, t_chain_device, s_device_name);
    CLASS_ATTR_ACCESSORS(c, "device_name", NULL, (method)chain_device_set_device_name);

    CLASS_ATTR_LONG(c, "autoupdate", 0, t_chain_device, s_autoupdate);
    CLASS_ATTR_LONG(c, "deviation", 0, t_chain_device, s_deviation);
    
    class_register(CLASS_BOX, c);
    s_chain_device_class = c;

    ps_name = gensym("name");
    ps_metrics = gensym("metrics");
    ps_location = gensym("location");
    ps_geoLocation = gensym("geoLocation");
    
    return 0;
}

void *chain_device_new(t_symbol *s, long argc, t_atom *argv)
{
    t_chain_device *x = (t_chain_device *)object_alloc(s_chain_device_class);

    chain_worker_new((t_chain_worker *)x, s, argc, argv);

    x->s_outlet2 = outlet_new(x, NULL);
    x->s_outlet = outlet_new(x, NULL);
    x->s_autoupdate = 1;
    x->s_deviation = 0;

    attr_args_process(x, argc, argv);

    return x;
}

void chain_device_free(t_chain_device *x)
{
    chain_worker_free((t_chain_worker *)x);
}

void chain_device_notify(t_chain_device *x, t_symbol *s, t_symbol *msg,
                         void *sender, void *data)
{
    if (s == x->s_worker.s_site_name && x->s_autoupdate && msg == x->s_device_name) {
        const char *href = (const char *)data;
        chain_device_send_sensor(x, href);
    }

    chain_worker_notify((t_chain_worker *)x, s, msg, sender, data);
}

void chain_device_set_device_name(t_chain_device *x, void *attr, long argc, t_atom *argv)
{
    t_symbol *device_name = atom_getsym(argv);
    if (!x->s_device_name || x->s_device_name!=device_name){
        x->s_device_name = device_name;
    }
}

void chain_device_send_sensor(t_chain_device *x, const char *href){
    if(!x->s_worker.s_db){
        chain_error("No DB!");
        return;
    }

    t_db_result *db_result = NULL;

    query_data_by_sensor_href(x->s_worker.s_db, href, &db_result);

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

double chain_device_compute_deviation(t_chain_device *x, t_symbol *metric,
                                      double value)
{
    t_db_result *db_result = NULL;
    query_data_by_metric_name(x->s_worker.s_db, metric->s_name, &db_result);

    long numrecords = db_result_numrecords(db_result);
    double values[numrecords];
    for (long i=0; i<numrecords; i++){
        values[i] = db_result_float(db_result, i, 0);
    }
    double mean = chain_mean(values, numrecords);
    double std = chain_std(values, numrecords);

    return (value - mean) / std;
}

void chain_device_send_metric(t_chain_device *x, t_symbol *metric){
    if(!x->s_worker.s_db){
        chain_error("No DB!");
        return;
    }

    if(!x->s_device_name){
        chain_error("No device name set");
        return;
    }

    t_db_result *db_result = NULL;

    query_data_by_device_name_metric_name(x->s_worker.s_db, x->s_device_name->s_name, metric->s_name, &db_result);

    if (!db_result_numrecords(db_result)){
        chain_error("No such metric for this device");
        return;
    }

    double value = db_result_float(db_result, 0, 0);
    const char *timestamp = db_result_string(db_result, 0, 1);

    if (x->s_deviation){
       value = chain_device_compute_deviation(x, metric, value); 
    }

    t_atom av[2];
    short ac = 2;
    atom_setsym(av, metric);
    atom_setfloat(av+1, value);

    outlet_list(x->s_outlet, 0L, ac, av);
}

void chain_device_send_all(t_chain_device *x){
    if(!x->s_worker.s_db){
        chain_error("No DB!");
        return;
    }

    if(!x->s_device_name){
        chain_error("No device name set");
        return;
    }

    t_db_result *db_result = NULL;

    query_data_by_device_name(x->s_worker.s_db, x->s_device_name->s_name, &db_result);

    long numrecords = db_result_numrecords(db_result);

    for (int i=0; i<numrecords; i++){
        const char *timestamp, *metric_name;
        double value;
        timestamp = db_result_string(db_result, i, 1);
        value = db_result_float(db_result, i, 0);
        metric_name = db_result_string(db_result, i, 2);
        t_symbol *metric = gensym(metric_name);

        if (x->s_deviation){
            value = chain_device_compute_deviation(x, metric, value);
        }

        t_atom av[2];
        short ac = 2;
        atom_setsym(av, metric);
        atom_setfloat(av+1, value);

        outlet_list(x->s_outlet, 0L, ac, av);
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

void chain_device_metrics(t_chain_device *x)
{
    if(!x->s_worker.s_db){
        chain_error("No DB!");
        return;
    }
    if(!x->s_device_name){
        chain_error("No device name set");
        return;
    }
    t_db_result *db_result = NULL;
    long numrecords;

    query_list_metrics_by_device_name(x->s_worker.s_db, x->s_device_name->s_name, &db_result); 

    numrecords = db_result_numrecords(db_result);
    if(!numrecords){
        chain_error("No metrics were found!");
        return;
    }

    t_atom av[numrecords];
    for (int i=0; i<numrecords; i++){
        const char *name;
        name = db_result_string(db_result, i, 0);
        atom_setsym(av+i, gensym(name));
    }

    outlet_anything(x->s_outlet2, ps_metrics, numrecords, av);
}

void chain_device_geoLocation(t_chain_device *x)
{
    if(!x->s_worker.s_db){
        chain_error("No DB!");
        return;
    }
    if(!x->s_device_name){
        chain_error("No device name set");
        return;
    }
    t_db_result *db_result = NULL;
    query_get_device_location(x->s_worker.s_db, x->s_device_name->s_name, &db_result);

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
    if(!x->s_worker.s_db){
        chain_error("No DB!");
        return;
    }
    if(!x->s_device_name){
        chain_error("No device name set");
        return;
    }
    t_db_result *db_result = NULL;
    query_get_device_location(x->s_worker.s_db, x->s_device_name->s_name, &db_result);

    double f_x = db_result_float(db_result, 0, 3);
    double f_z = db_result_float(db_result, 0, 4);

    t_atom av[2];
    short ac = 2;
    atom_setfloat(av, f_x);
    atom_setfloat(av+1, f_z);
    outlet_anything(x->s_outlet2, ps_location, ac, av);
}


void chain_device_data(t_chain_device *x, t_symbol *metric, long start, long end)
{
    t_db_result *db_result = NULL;
    query_sensor_href_by_device_name_metric_name(x->s_worker.s_db, x->s_device_name->s_name,
                                                 metric->s_name, &db_result);
    if(!db_result_numrecords(db_result)){
        chain_error("Metric not valid for this device");
        return;
    }
    const char *sensor_href = db_result_string(db_result, 0, 0);
    double *data;
    long num_events;
    t_chain_event *events;
    chain_get_data(sensor_href, start, end, &events, &num_events);

    t_atom av[num_events];
    for (int i=0; i<num_events; i++){
        double value = (events +i)->s_value;
        if (x->s_deviation){
            value = chain_device_compute_deviation(x, metric, value);
        }
        atom_setfloat(av+i, value);
    }
    outlet_anything(x->s_outlet, metric, num_events, av);
    free(events);
}


