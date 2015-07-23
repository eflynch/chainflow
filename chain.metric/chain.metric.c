/**
 @file
 chain.metric - a chain_metric object
 
 @ingroup    maxchain
 */
#include "chain.metric.h"

#include "messages.h"
#include "chainmath.h"
#include "queries.h"

// Create + Destroy
void *chain_metric_new(t_symbol *s, long argc, t_atom *argv);
void chain_metric_free(t_chain_metric *x);

// Methods
void chain_metric_bang(t_chain_metric *x);
void chain_metric_int(t_chain_metric *x, long n);
void chain_metric_pos(t_chain_metric *x, t_symbol *s, long argc, t_atom *argv);

// Notify
void chain_metric_notify(t_chain_metric *x, t_symbol *s, t_symbol *msg, void *sender, void *data);

void chain_metric_send_mean(t_chain_metric *x);
void chain_metric_send_std(t_chain_metric *x);
void chain_metric_send_deviation(t_chain_metric *x);
void chain_metric_send_interpolation(t_chain_metric *x);
void chain_metric_send_max(t_chain_metric *x);
void chain_metric_send_min(t_chain_metric *x);
void chain_metric_send_median(t_chain_metric *x);

static t_class *s_chain_metric_class = NULL;

t_symbol *ps_url, *ps_db;
t_symbol *ps_mean, *ps_std, *ps_deviation, *ps_interpolation, *ps_max, *ps_min, *ps_median;
t_symbol *ps_bilinear;

int C74_EXPORT main(void)
{
    t_class *c;

    c = class_new("chain.metric", (method)chain_metric_new, (method)chain_metric_free, sizeof(t_chain_metric), (method)0L, A_GIMME, 0);
    
    class_addmethod(c, (method)chain_metric_bang, "bang", 0);
    class_addmethod(c, (method)chain_metric_int, "int", A_LONG, 0);
    class_addmethod(c, (method)chain_metric_notify, "notify", A_CANT, 0);
    class_addmethod(c, (method)chain_metric_pos, "pos", A_GIMME, 0);

    CLASS_ATTR_SYM(c, "name", ATTR_SET_OPAQUE_USER, t_chain_metric, s_worker.s_site_name);

    CLASS_ATTR_SYM(c, "metric_name", 0, t_chain_metric, s_metric_name);

    CLASS_ATTR_SYM(c, "measure", 0, t_chain_metric, s_measure);
    CLASS_ATTR_SYM(c, "interp", 0, t_chain_metric, s_interp);

    CLASS_ATTR_FLOAT(c, "pos_x", 0, t_chain_metric, s_pos_x);
    CLASS_ATTR_FLOAT(c, "pos_y", 0, t_chain_metric, s_pos_y);
    CLASS_ATTR_FLOAT(c, "pos_z", 0, t_chain_metric, s_pos_z);
    CLASS_ATTR_FLOAT(c, "radius", 0, t_chain_metric, s_radius);

    class_register(CLASS_BOX, c);

    ps_url = gensym("url");
    ps_db = gensym("db");

    ps_mean = gensym("mean");
    ps_std = gensym("std");
    ps_deviation = gensym("deviation");
    ps_interpolation = gensym("interpolation");
    ps_max = gensym("max");
    ps_min = gensym("min");
    ps_median = gensym("median");

    ps_bilinear = gensym("bilinear");

    s_chain_metric_class = c;

    return 0;
}

void *chain_metric_new(t_symbol *s, long argc, t_atom *argv)
{
    t_chain_metric *x = (t_chain_metric *)object_alloc(s_chain_metric_class);

    chain_worker_new((t_chain_worker *)x, s, argc, argv);

    x->s_radius = 0.0;
    x->s_pos_x = 0.0;
    x->s_pos_y = 0.0;
    x->s_pos_z = 0.0;

    x->s_measure = ps_mean;
    x->s_interp = ps_bilinear;

    attr_args_process(x, argc, argv);

    x->s_outlet = outlet_new(x, NULL);
    
    return x;
}

void chain_metric_free(t_chain_metric *x)
{
    chain_worker_release_site((t_chain_worker *)x);
}

void chain_metric_notify(t_chain_metric *x, t_symbol *s, t_symbol *msg, void *sender, void *data)
{
    chain_worker_notify((t_chain_worker *)x, s, msg, sender, data);
}

void chain_metric_int(t_chain_metric *x, long n)
{
}

void chain_metric_bang(t_chain_metric *x)
{
    if (!x->s_worker.s_db){
        chain_error("No database found");
        return;
    }

    if (!x->s_metric_name){
        chain_error("No metric name set");
        return;
    }

    if (!x->s_measure){
        chain_error("No measure set");
        return;
    }

    if (x->s_measure == ps_mean){
        chain_metric_send_mean(x);
    } else if (x->s_measure == ps_std){
        chain_metric_send_std(x);
    } else if (x->s_measure == ps_deviation){
        chain_metric_send_deviation(x);
    } else if (x->s_measure == ps_interpolation){
        chain_metric_send_interpolation(x);
    } else if (x->s_measure == ps_max){
        chain_metric_send_max(x);
    } else if (x->s_measure == ps_min){
        chain_metric_send_min(x);
    } else if (x->s_measure == ps_median){
        chain_metric_send_median(x);
    }
}

void chain_metric_unpack_values(t_db_result *db_result, double **argv, long *argc){
    *argc = db_result_numrecords(db_result);
    *argv = malloc(sizeof(**argv) * *argc);
    for (long i=0; i<*argc; i++){
        *(*argv + i) = db_result_float(db_result, i, 0);
    }
}

void chain_metric_send_mean(t_chain_metric *x)
{
    t_db_result *db_result = NULL;
    if (x->s_radius > 0.0){
        query_near_data_by_metric_name(x->s_worker.s_db, x->s_pos_x,
                                      x->s_pos_z, x->s_radius, x->s_metric_name->s_name, &db_result);
    } else {
        query_data_by_metric_name(x->s_worker.s_db, x->s_metric_name->s_name, &db_result);
    }

    double *argv;
    long argc;
    chain_metric_unpack_values(db_result, &argv, &argc);
    outlet_float(x->s_outlet, chain_mean(argv, argc));
    free(argv);
}

void chain_metric_send_std(t_chain_metric *x)
{
    t_db_result *db_result = NULL;
    if (x->s_radius > 0.0){
        query_near_data_by_metric_name(x->s_worker.s_db, x->s_pos_x,
                                      x->s_pos_z, x->s_radius, x->s_metric_name->s_name, &db_result);
    } else {
        query_data_by_metric_name(x->s_worker.s_db, x->s_metric_name->s_name, &db_result);
    }

    double *argv;
    long argc;
    chain_metric_unpack_values(db_result, &argv, &argc);
    outlet_float(x->s_outlet, chain_std(argv, argc));
    free(argv);
}

void chain_metric_send_deviation(t_chain_metric *x){
    chain_error("Unimplemented");
}

void chain_metric_send_interpolation(t_chain_metric *x){
    chain_error("Unimplemented");
}

void chain_metric_send_max(t_chain_metric *x){
    t_db_result *db_result = NULL;
    if (x->s_radius > 0.0){
        query_near_data_by_metric_name(x->s_worker.s_db, x->s_pos_x,
                                      x->s_pos_z, x->s_radius, x->s_metric_name->s_name, &db_result);
    } else {
        query_data_by_metric_name(x->s_worker.s_db, x->s_metric_name->s_name, &db_result);
    }

    double *argv;
    long argc;
    chain_metric_unpack_values(db_result, &argv, &argc);
    outlet_float(x->s_outlet, chain_max(argv, argc));
    free(argv);
}

void chain_metric_send_min(t_chain_metric *x){
    t_db_result *db_result = NULL;
    if (x->s_radius > 0.0){
        query_near_data_by_metric_name(x->s_worker.s_db, x->s_pos_x,
                                      x->s_pos_z, x->s_radius, x->s_metric_name->s_name, &db_result);
    } else {
        query_data_by_metric_name(x->s_worker.s_db, x->s_metric_name->s_name, &db_result);
    }

    double *argv;
    long argc;
    chain_metric_unpack_values(db_result, &argv, &argc);
    outlet_float(x->s_outlet, chain_min(argv, argc));
    free(argv);
}

void chain_metric_send_median(t_chain_metric *x){
    chain_error("Unimplemented");
}


void chain_metric_pos(t_chain_metric *x, t_symbol *s, long argc, t_atom *argv){
    if (argc != 3){
        chain_error("chain.metric: pos got %ld args, expected 3.", argc);
        return;
    }
    float f_x = atom_getfloat(argv);
    float f_y = atom_getfloat(argv+1);
    float f_z = atom_getfloat(argv+2);
    x->s_pos_x = f_x;
    x->s_pos_y = f_y;
    x->s_pos_z = f_z;
}
