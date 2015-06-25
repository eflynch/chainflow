/**
 @file
 chain.zone - a simple chain_zone object
 
 @ingroup    maxchain
 */

#include "ext.h"
#include "ext_common.h"
#include "ext_dictobj.h"
#include "ext_database.h"

#include "messages.h"
#include "queries.h"

typedef struct chain_zone
{
    t_object s_obj;
    t_systhread s_systhread_setup;
    int s_setup_cancel;
    void *s_outlet;
    float s_pos[3];
    float s_enter_threshold;
    float s_exit_threshold;
    t_symbol *s_site_name;
    t_dictionary *s_dictionary;
    t_database *s_db;
} t_chain_zone;

void *chain_zone_new(t_symbol *s, long argc, t_atom *argv);
void chain_zone_free(t_chain_zone *x);
void chain_zone_int(t_chain_zone *x, long n);
void chain_zone_bang(t_chain_zone *x);
void chain_zone_set_site_name(t_chain_zone *x, void *attr, long argc, t_atom *argv);
void chain_zone_set_pos(t_chain_zone *x, void *attr, long argc, t_atom *argv);
void chain_zone_set_enter(t_chain_zone *x, void *attr, long argc, t_atom *argv);
void chain_zone_set_exit(t_chain_zone *x, void *attr, long argc, t_atom *argv);


int chain_zone_get_dict(t_chain_zone *x);
void *chain_zone_setup_threadproc(t_chain_zone *x);

static t_class *s_chain_zone_class = NULL;

t_symbol *ps_url, *ps_name, *ps_db, *ps_devices;

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
    
    attr_args_process(x, argc, argv);
    if (!x->s_site_name) {
        if (site_name)
            object_attr_setsym(x, ps_name, site_name);
    }
    
    x->s_outlet = outlet_new(x, NULL);
    x->s_setup_cancel = false;
    
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
    float f_z = atom_getfloat(argv+1);
    x->s_pos[0] = f_x;
    x->s_pos[1] = f_y;
    x->s_pos[2] = f_z;
}

void chain_zone_set_enter(t_chain_zone *x, void *attr, long argc, t_atom *argv){
    float enter_threshold = atom_getfloat(argv);
    x->s_enter_threshold = enter_threshold;
}

void chain_zone_set_exit(t_chain_zone *x, void *attr, long argc, t_atom *argv){
    float exit_threshold = atom_getfloat(argv);
    x->s_exit_threshold = exit_threshold;
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
    
    
    unsigned int ret;
    if (x->s_systhread_setup){
        x->s_setup_cancel = true;
        systhread_join(x->s_systhread_setup, &ret);
        x->s_systhread_setup = NULL;
    }
}

void chain_zone_int(t_chain_zone *x, long n)
{
}

void chain_zone_bang(t_chain_zone *x)
{
}

void *chain_zone_setup_threadproc(t_chain_zone *x)
{
    int err=1;
    while(err){
        chain_zone_get_dict(x);
        systhread_sleep(1000);
        
        if (x->s_setup_cancel)
            break;
    }
    
    systhread_exit(0);
    return NULL;
}
