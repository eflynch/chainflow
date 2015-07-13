/**
    @file
    chain.info - a simple chain_info object
 
    @ingroup    maxchain
 */

#include "ext.h"
#include "ext_common.h"
#include "ext_dictobj.h"
#include "ext_database.h"

#include "messages.h"
#include "queries.h"
#include "chainworker.h"

typedef struct chain_info
{
    t_chain_worker s_worker;
    void *s_outlet;
} t_chain_info;

// Create + Destroy
void *chain_info_new(t_symbol *s, long argc, t_atom *argv);
void chain_info_free(t_chain_info *x);


// Methods
void chain_info_int(t_chain_info *x, long n);
void chain_info_bang(t_chain_info *x);
void chain_info_metrics(t_chain_info *x);
void chain_info_devices(t_chain_info *x);
void chain_info_near(t_chain_info *x, t_symbol *s, long argc, t_atom *argv);
void chain_info_nearest(t_chain_info *x, t_symbol *s, long argc, t_atom *argv);

// Attribute Setters
void chain_info_set_site_name(t_chain_info *x, void *attr, long argc, t_atom *argv);

// Notify
void chain_info_notify(t_chain_info *x, t_symbol *s, t_symbol *msg, void *sender, void *data);

static t_class *s_chain_info_class = NULL;

t_symbol *ps_url, *ps_name, *ps_devices, *ps_metrics;

int C74_EXPORT main(void)
{
    t_class *c;

    c = class_new("chain.info", (method)chain_info_new, (method)chain_info_free, sizeof(t_chain_info), (method)0L, A_GIMME, 0);
    
    class_addmethod(c, (method)chain_info_bang, "bang", 0);
    class_addmethod(c, (method)chain_info_metrics, "metrics", 0);
    class_addmethod(c, (method)chain_info_devices, "devices", 0);
    class_addmethod(c, (method)chain_info_near, "near", A_GIMME, 0);
    class_addmethod(c, (method)chain_info_nearest, "nearest", A_GIMME, 0);
    class_addmethod(c, (method)chain_info_int, "int", A_LONG, 0);
    class_addmethod(c, (method)chain_info_notify, "notify", A_CANT, 0);

    CLASS_ATTR_SYM(c, "name", ATTR_SET_OPAQUE_USER, t_chain_info, s_worker.s_site_name);

    class_register(CLASS_BOX, c);

    ps_url = gensym("url");
    ps_name = gensym("name");
    ps_metrics = gensym("metrics");
    ps_devices = gensym("devices");

    s_chain_info_class = c;

    return 0;
}

void *chain_info_new(t_symbol *s, long argc, t_atom *argv)
{
    t_chain_info *x = (t_chain_info *)object_alloc(s_chain_info_class);

    chain_worker_new((t_chain_worker *)x, s, argc, argv);

    attr_args_process(x, argc, argv);

    x->s_outlet = outlet_new(x, NULL);

    return x;
}

void chain_info_free(t_chain_info *x)
{
    chain_worker_release_site((t_chain_worker *)x);
}


void chain_info_notify(t_chain_info *x, t_symbol *s, t_symbol *msg, void *sender, void *data)
{
    chain_worker_notify((t_chain_worker *)x, s, msg, sender, data);
}

void chain_info_int(t_chain_info *x, long n)
{
}

void chain_info_bang(t_chain_info *x)
{
}

void chain_info_near(t_chain_info *x, t_symbol *s, long argc, t_atom *argv)
{
    if (argc!=3){
        chain_error("near expected with 3 arguments; found %ld", argc);
        return;
    }
    float f_x = atom_getfloat(argv);
    float f_z = atom_getfloat(argv+1);
    float f_r = atom_getfloat(argv+2);

    t_db_result *db_result = NULL;

    query_list_devices_near_point(x->s_worker.s_db, (double)f_x, (double)f_z, (double)f_r, &db_result);

    long numrecords = db_result_numrecords(db_result);
    t_atom av[numrecords];
    for (int i=0; i<numrecords; i++){
        const char *name;
        name = db_result_string(db_result, i, 0);
        atom_setsym(av+i, gensym(name));
    }

    outlet_anything(x->s_outlet, ps_devices, numrecords, av);
}

void chain_info_nearest(t_chain_info *x, t_symbol *s, long argc, t_atom *argv)
{
    if (argc!=3){
        chain_error("near expected with 3 arguments; found %ld", argc);
        return;
    }
    float f_x = atom_getfloat(argv);
    float f_z = atom_getfloat(argv+1);
    long n = atom_getlong(argv+2);

    t_db_result *db_result = NULL;

    query_list_nearest_devices(x->s_worker.s_db, (double)f_x, (double)f_z, n, &db_result);

    long numrecords = db_result_numrecords(db_result);
    t_atom av[numrecords];
    for (int i=0; i<numrecords; i++){
        const char *name;
        name = db_result_string(db_result, i, 0);
        atom_setsym(av+i, gensym(name));
    }

    outlet_anything(x->s_outlet, ps_devices, numrecords, av);
}

void chain_info_metrics(t_chain_info *x)
{ 
    t_db_result *db_result = NULL;
    long numrecords;

    query_list_metrics(x->s_worker.s_db, &db_result);

    numrecords = db_result_numrecords(db_result);

    t_atom av[numrecords];

    for (int i=0; i<numrecords; i++){
        const char *name;
        name = db_result_string(db_result, i, 0);
        atom_setsym(av+i, gensym(name));
    }

    outlet_anything(x->s_outlet, ps_metrics, numrecords, av);
}

void chain_info_devices(t_chain_info *x)
{ 
    t_db_result *db_result = NULL;
    long numrecords;

    query_list_devices(x->s_worker.s_db, &db_result);

    numrecords = db_result_numrecords(db_result);

    t_atom av[numrecords];

    for (int i=0; i<numrecords; i++){
        const char *name;
        name = db_result_string(db_result, i, 0);
        atom_setsym(av+i, gensym(name));
    }

    outlet_anything(x->s_outlet, ps_devices, numrecords, av);
}


