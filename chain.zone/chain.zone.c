/**
 @file
 chain.zone - a simple chain_zone object
 
 @ingroup    maxchain
 */

#include <math.h>

#include "ext.h"
#include "ext_common.h"
#include "ext_dictobj.h"
#include "ext_database.h"
#include "ext_symobject.h"
#include "ext_hashtab.h"

#include "messages.h"
#include "queries.h"

typedef struct chain_zone
{
    t_object s_obj;
    t_systhread s_systhread_setup;
    int s_setup_cancel;
    void *s_outlet;
    void *s_outlet2;
    float s_pos[3];
    float s_enter_threshold;
    float s_exit_threshold;
    t_hashtab *s_current_devices;
    t_symbol *s_site_name;
    t_dictionary *s_dictionary;
    t_database *s_db;
} t_chain_zone;

typedef struct t_ordered_triple {
    float s_x;
    float s_y;
    float s_z;
} t_ordered_triple;

void *chain_zone_new(t_symbol *s, long argc, t_atom *argv);
void chain_zone_free(t_chain_zone *x);
void chain_zone_int(t_chain_zone *x, long n);
void chain_zone_bang(t_chain_zone *x);
void chain_zone_set_site_name(t_chain_zone *x, void *attr, long argc, t_atom *argv);
void chain_zone_set_pos(t_chain_zone *x, void *attr, long argc, t_atom *argv);
void chain_zone_set_enter(t_chain_zone *x, void *attr, long argc, t_atom *argv);
void chain_zone_set_exit(t_chain_zone *x, void *attr, long argc, t_atom *argv);


void chain_zone_update(t_chain_zone *x);
void chain_zone_exit_handler(t_hashtab_entry *e, void *arg);
int chain_zone_get_dict(t_chain_zone *x);
void *chain_zone_setup_threadproc(t_chain_zone *x);

static t_class *s_chain_zone_class = NULL;

t_symbol *ps_url, *ps_name, *ps_db, *ps_devices, *ps_added, *ps_removed;

int C74_EXPORT main(void)
{
    t_class *c;
    
    c = class_new("chain.zone", (method)chain_zone_new, (method)chain_zone_free, sizeof(t_chain_zone), (method)0L, A_GIMME, 0);
    
    class_addmethod(c, (method)chain_zone_bang, "bang", 0);
    class_addmethod(c, (method)chain_zone_int, "int", A_LONG, 0);
    
    CLASS_ATTR_SYM(c, "name", 0, t_chain_zone, s_site_name);
    CLASS_ATTR_ACCESSORS(c, "name", NULL, chain_zone_set_site_name);

    CLASS_ATTR_FLOAT(c, "pos", 0, t_chain_zone, s_pos);
    CLASS_ATTR_ACCESSORS(c, "pos", NULL, chain_zone_set_pos);
    CLASS_ATTR_FLOAT(c, "enter", 0, t_chain_zone, s_enter_threshold);
    CLASS_ATTR_ACCESSORS(c, "enter", NULL, chain_zone_set_enter);
    CLASS_ATTR_FLOAT(c, "exit", 0, t_chain_zone, s_exit_threshold);
    CLASS_ATTR_ACCESSORS(c, "exit", NULL, chain_zone_set_exit);
    
    class_register(CLASS_BOX, c);
    
    ps_url = gensym("url");
    ps_name = gensym("name");
    ps_db = gensym("db");
    ps_devices = gensym("devices");
    ps_added = gensym("added");
    ps_removed = gensym("removed");
    
    s_chain_zone_class = c;
    
    return 0;
}

void *chain_zone_new(t_symbol *s, long argc, t_atom *argv)
{
    t_chain_zone *x = (t_chain_zone *)object_alloc(s_chain_zone_class);
    
    long attrstart = attr_args_offset(argc, argv);
    t_symbol *site_name = NULL;
    
    if (attrstart && atom_gettype(argv) == A_SYM)
        site_name = atom_getsym(argv);

    x->s_enter_threshold = 10.0;
    x->s_exit_threshold = 20.0;   

    attr_args_process(x, argc, argv);
    if (!x->s_site_name) {
        if (site_name)
            object_attr_setsym(x, ps_name, site_name);
    }

    x->s_outlet2 = outlet_new(x, NULL);
    x->s_outlet = outlet_new(x, NULL);
    x->s_setup_cancel = false;
    x->s_current_devices = hashtab_new(0);
    
    return x;
}

void chain_zone_set_site_name(t_chain_zone *x, void *attr, long argc, t_atom *argv)
{
    t_symbol *site_name = atom_getsym(argv);
    if (!x->s_site_name || x->s_site_name!=site_name){
        x->s_site_name = site_name;
        
        if (x->s_systhread_setup == NULL){
            systhread_create((method) chain_zone_setup_threadproc, x, 0, 0, 0, &x->s_systhread_setup);
        }
    }
}

void chain_zone_set_pos(t_chain_zone *x, void *attr, long argc, t_atom *argv){
    if (argc != 3){
        chain_error("Zone position requires 3 arguments not %ld", argc);
        return;
    }
    float f_x = atom_getfloat(argv);
    float f_y = atom_getfloat(argv+1);
    float f_z = atom_getfloat(argv+2);
    x->s_pos[0] = f_x;
    x->s_pos[1] = f_y;
    x->s_pos[2] = f_z;

    chain_zone_update(x);
}

void chain_zone_set_enter(t_chain_zone *x, void *attr, long argc, t_atom *argv){
    float enter_threshold = atom_getfloat(argv);
    if (x->s_exit_threshold < enter_threshold){
        x->s_exit_threshold = enter_threshold;
    }
    x->s_enter_threshold = enter_threshold;

    chain_zone_update(x);
}

void chain_zone_set_exit(t_chain_zone *x, void *attr, long argc, t_atom *argv){
    float exit_threshold = atom_getfloat(argv);
    if (exit_threshold >= x->s_enter_threshold){
        x->s_exit_threshold = exit_threshold;
    } else {
        x->s_exit_threshold = x->s_enter_threshold;
    }

    chain_zone_update(x);
}

int chain_zone_get_dict(t_chain_zone *x)
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

void chain_zone_free(t_chain_zone *x)
{
    if (x->s_dictionary)
        dictobj_release(x->s_dictionary);

    hashtab_chuck(x->s_current_devices);
    
    
    unsigned int ret;
    if (x->s_systhread_setup){
        x->s_setup_cancel = true;
        systhread_join(x->s_systhread_setup, &ret);
        x->s_systhread_setup = NULL;
    }
}

void chain_zone_update(t_chain_zone *x)
{
    hashtab_funall(x->s_current_devices, (method)chain_zone_exit_handler, x);

    t_db_result *db_result = NULL;

    query_list_devices_near_point(x->s_db, x->s_pos[0], x->s_pos[2], x->s_enter_threshold, &db_result);

    long numrecords = db_result_numrecords(db_result);
    for (int i=0; i<numrecords; i++){
        const char *name = db_result_string(db_result, i, 0);
        float f_x = db_result_float(db_result, i, 1);
        float f_z = db_result_float(db_result, i, 2);
        float f_d = db_result_float(db_result, i, 3);

        t_symbol *device_name = gensym(name);
        t_object *val = NULL;

        hashtab_lookup(x->s_current_devices, device_name, &val);
        if (!val){
            t_symobject *value = symobject_new(device_name);
            t_ordered_triple *value_point = malloc(sizeof(t_ordered_triple));
            value_point->s_x = f_x;
            value_point->s_z = f_z;
            value->thing = value_point;

            hashtab_store(x->s_current_devices, device_name, (t_object *)value);

            t_atom av[1];
            atom_setsym(av, gensym(name));
            outlet_anything(x->s_outlet, ps_added, 1, av);
        }
    }
}

void chain_zone_exit_handler(t_hashtab_entry *e, void *arg)
{
    t_chain_zone *x = (t_chain_zone *)arg;
    t_symbol *device_name = e->key;// chuck if outside of exit threshold and output
    t_symobject *value = (t_symobject *)e->value;
    t_ordered_triple *value_point = value->thing;
    float f_d = (value_point->s_x - x->s_pos[0]) * (value_point->s_x - x->s_pos[0]);
    f_d += (value_point->s_z - x->s_pos[2]) * (value_point->s_z - x->s_pos[2]);
    if (f_d > x->s_exit_threshold * x->s_exit_threshold)
    {
        hashtab_chuckkey(x->s_current_devices, device_name);
        free(value_point);
        t_atom av[1];
        atom_setsym(av, device_name);
        outlet_anything(x->s_outlet2, ps_removed, 1, av);
    }
}

void chain_zone_int(t_chain_zone *x, long n)
{
}

void chain_zone_bang(t_chain_zone *x)
{
    long ac = 0;
    t_symbol **keys = NULL;
    hashtab_getkeys(x->s_current_devices, &ac, &keys);

    t_atom av[ac];

    for (int i=0; i<ac; i++){
        atom_setsym(av+i, keys[i]);
    }

    outlet_anything(x->s_outlet, ps_devices, ac, av);
    if (keys)
        sysmem_freeptr(keys);
}

void *chain_zone_setup_threadproc(t_chain_zone *x)
{
    int err=1;
    while(err){
        err = chain_zone_get_dict(x);
        systhread_sleep(1000);
        
        if (x->s_setup_cancel)
            break;
    }
    
    systhread_exit(0);
    return NULL;
}
