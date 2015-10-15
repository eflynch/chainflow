/**
 @file
 chain.data - a simple chain_data object
 
 @ingroup    maxchain
 */

#include "ext.h"
#include "ext_common.h"
#include "ext_dictobj.h"
#include "ext_database.h"

#include "messages.h"
#include "chainworker.h"

typedef struct chain_data
{
    t_chain_worker s_worker;
    void *s_outlet;
} t_chain_data;

// Create + Destroy
void *chain_data_new(t_symbol *s, long argc, t_atom *argv);
void chain_data_free(t_chain_data *x);


// Methods
void chain_data_int(t_chain_data *x, long n);
void chain_data_bang(t_chain_data *x);

// Attribute Setters
void chain_data_set_site_name(t_chain_data *x, void *attr, long argc, t_atom *argv);

// Notify
void chain_data_notify(t_chain_data *x, t_symbol *s, t_symbol *msg, void *sender, void *data);

static t_class *s_chain_data_class = NULL;

t_symbol *ps_url, *ps_name;

int C74_EXPORT main(void)
{
    t_class *c;
    
    c = class_new("chain.data", (method)chain_data_new, (method)chain_data_free, sizeof(t_chain_data), (method)0L, A_GIMME, 0);
    
    class_addmethod(c, (method)chain_data_bang, "bang", 0);
    class_addmethod(c, (method)chain_data_int, "int", A_LONG, 0);
    class_addmethod(c, (method)chain_data_notify, "notify", A_CANT, 0);
    
    CLASS_ATTR_SYM(c, "name", ATTR_SET_OPAQUE_USER, t_chain_data, s_worker.s_site_name);
    
    class_register(CLASS_BOX, c);
    
    ps_url = gensym("url");
    ps_name = gensym("name");
    
    s_chain_data_class = c;
    
    return 0;
}

void *chain_data_new(t_symbol *s, long argc, t_atom *argv)
{
    t_chain_data *x = (t_chain_data *)object_alloc(s_chain_data_class);
    
    chain_worker_new((t_chain_worker *)x, s, argc, argv);
    
    attr_args_process(x, argc, argv);
    
    x->s_outlet = outlet_new(x, NULL);
    
    return x;
}

void chain_data_free(t_chain_data *x)
{
    chain_worker_release_site((t_chain_worker *)x);
}


void chain_data_notify(t_chain_data *x, t_symbol *s, t_symbol *msg, void *sender, void *data)
{
    chain_worker_notify((t_chain_worker *)x, s, msg, sender, data);
}

void chain_data_int(t_chain_data *x, long n)
{
}

void chain_data_bang(t_chain_data *x)
{
}
