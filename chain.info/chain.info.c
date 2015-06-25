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

typedef struct chain_info
{
    t_object s_obj;
    t_systhread s_systhread_setup;
    int s_setup_cancel;
    void *s_outlet;
    t_symbol *s_site_name;
    t_dictionary *s_dictionary;
    t_database *s_db;
} t_chain_info;

void *chain_info_new(t_symbol *s, long argc, t_atom *argv);
void chain_info_free(t_chain_info *x);
void chain_info_int(t_chain_info *x, long n);
void chain_info_bang(t_chain_info *x);
void chain_info_set_site_name(t_chain_info *x, void *attr, long argc, t_atom *argv);
void chain_info_metrics(t_chain_info *x);
void chain_info_devices(t_chain_info *x);
void chain_info_near(t_chain_info *x, t_symbol *s, long argc, t_atom *argv);
void chain_info_nearest(t_chain_info *x, t_symbol *s, long argc, t_atom *argv);

int chain_info_get_dict(t_chain_info *x);
void *chain_info_setup_threadproc(t_chain_info *x);

static t_class *s_chain_info_class = NULL;

t_symbol *ps_url, *ps_name, *ps_db, *ps_devices, *ps_metrics;

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

    CLASS_ATTR_SYM(c, "name", 0, t_chain_info, s_site_name);
    CLASS_ATTR_ACCESSORS(c, "name", NULL, chain_info_set_site_name);

    class_register(CLASS_BOX, c);

    ps_url = gensym("url");
    ps_name = gensym("name");
    ps_db = gensym("db");
    ps_metrics = gensym("metrics");
    ps_devices = gensym("devices");

    s_chain_info_class = c;

    return 0;
}

void *chain_info_new(t_symbol *s, long argc, t_atom *argv)
{
    t_chain_info *x = (t_chain_info *)object_alloc(s_chain_info_class);

    long attrstart = attr_args_offset(argc, argv);
    t_symbol *site_name = NULL;

    if (attrstart && atom_gettype(argv) == A_SYM)
        site_name = atom_getsym(argv);

    attr_args_process(x, argc, argv);
    if (!x->s_site_name) {
        if (site_name)
            object_attr_setsym(x, ps_name, site_name);
    }

    x->s_outlet = outlet_new(x, NULL);
    x->s_setup_cancel = false;
    
    return x;
}

void chain_info_set_site_name(t_chain_info *x, void *attr, long argc, t_atom *argv)
{
    t_symbol *site_name = atom_getsym(argv);
    if (!x->s_site_name || x->s_site_name!=site_name){
        x->s_site_name = site_name;

        if (x->s_systhread_setup == NULL){
            systhread_create((method) chain_info_setup_threadproc, x, 0, 0, 0, &x->s_systhread_setup);
        }
        

    }
}

int chain_info_get_dict(t_chain_info *x)
{
    int err = 0;
    if (x->s_dictionary)
        dictobj_release(x->s_dictionary);
    x->s_dictionary = dictobj_findregistered_retain(x->s_site_name);

    if (!x->s_dictionary)
        err = 1;

    dictionary_getobject(x->s_dictionary, ps_db, &x->s_db);

    return err;
}

void chain_info_free(t_chain_info *x)
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

    query_list_devices_near_point(x->s_db, (double)f_x, (double)f_z, (double)f_r, &db_result);

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

    query_list_nearest_devices(x->s_db, (double)f_x, (double)f_z, n, &db_result);

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

    query_list_metrics(x->s_db, &db_result);

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

    query_list_devices(x->s_db, &db_result);

    numrecords = db_result_numrecords(db_result);

    t_atom av[numrecords];

    for (int i=0; i<numrecords; i++){
        const char *name;
        name = db_result_string(db_result, i, 0);
        atom_setsym(av+i, gensym(name));
    }

    outlet_anything(x->s_outlet, ps_devices, numrecords, av);
}

void *chain_info_setup_threadproc(t_chain_info *x)
{
    int err=1;
    while(err){
        chain_info_get_dict(x);
        systhread_sleep(1000);

        if (x->s_setup_cancel)
            break;
    }

    systhread_exit(0);
    return NULL;
}
