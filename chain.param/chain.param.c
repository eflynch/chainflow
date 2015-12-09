/**
 @file
 chain.param - a simple chain_param object
 
 @ingroup    maxchain
 */

#include "ext.h"
#include "ext_common.h"
#include "ext_dictobj.h"
#include "ext_database.h"
#include "ext_path.h"

#include "messages.h"
#include "queries.h"
#include "chainworker.h"

typedef struct chain_param
{
    t_chain_worker s_worker;
    t_symbol *s_filename;
    void *s_outlet;
} t_chain_param;

// Create + Destroy
void *chain_param_new(t_symbol *s, long argc, t_atom *argv);
void chain_param_free(t_chain_param *x);


// Methods
void chain_param_int(t_chain_param *x, long n);
void chain_param_bang(t_chain_param *x);

// Attribute Setters
void chain_param_set_site_name(t_chain_param *x, void *attr, long argc, t_atom *argv);

// Notify
void chain_param_notify(t_chain_param *x, t_symbol *s, t_symbol *msg, void *sender, void *data);

static t_class *s_chain_param_class = NULL;

t_symbol *ps_url, *ps_name, *ps_devices, *ps_metrics;

int C74_EXPORT main(void)
{
    t_class *c;
    
    c = class_new("chain.param", (method)chain_param_new, (method)chain_param_free, sizeof(t_chain_param), (method)0L, A_GIMME, 0);
    
    class_addmethod(c, (method)chain_param_bang, "bang", 0);
    class_addmethod(c, (method)chain_param_int, "int", A_LONG, 0);
    class_addmethod(c, (method)chain_param_notify, "notify", A_CANT, 0);
    
    CLASS_ATTR_SYM(c, "name", ATTR_SET_OPAQUE_USER, t_chain_param, s_worker.s_site_name);
    
    class_register(CLASS_BOX, c);
    
    ps_url = gensym("url");
    ps_name = gensym("name");
    ps_metrics = gensym("metrics");
    ps_devices = gensym("devices");
    
    s_chain_param_class = c;
    
    return 0;
}

void *chain_param_new(t_symbol *s, long argc, t_atom *argv)
{
    t_chain_param *x = (t_chain_param *)object_alloc(s_chain_param_class);
    
    chain_worker_new((t_chain_worker *)x, s, argc, argv);
    
    attr_args_process(x, argc, argv);
    
    x->s_outlet = outlet_new(x, NULL);
    
    return x;
}

void chain_param_free(t_chain_param *x)
{
    chain_worker_release_site((t_chain_worker *)x);
}


void chain_param_notify(t_chain_param *x, t_symbol *s, t_symbol *msg, void *sender, void *data)
{
    chain_worker_notify((t_chain_worker *)x, s, msg, sender, data);
}

void chain_param_int(t_chain_param *x, long n)
{
}

void chain_param_bang(t_chain_param *x)
{
}



