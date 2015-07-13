/**
 @file
 chain.metric - a chain_metric object
 
 @ingroup    maxchain
 */
#include "chain.metric.h"

#include "messages.h"

// Create + Destroy
void *chain_metric_new(t_symbol *s, long argc, t_atom *argv);
void chain_metric_free(t_chain_metric *x);

// Methods
void chain_metric_bang(t_chain_metric *x);
void chain_metric_int(t_chain_metric *x, long n);

// Attribute Setters
void chain_metric_set_site_name(t_chain_metric *x, void *attr, long argc, t_atom *argv);
void chain_metric_set_metric_name(t_chain_metric *x, void *attr, long argc, t_atom *argv);

// Notify
void chain_metric_notify(t_chain_metric *x, t_symbol *s, t_symbol *msg, void *sender, void *data);

static t_class *s_chain_metric_class = NULL;

t_symbol *ps_url, *ps_db, *ps_maxchain, *ps_name;

int C74_EXPORT main(void)
{
    t_class *c;

    c = class_new("chain.metric", (method)chain_metric_new, (method)chain_metric_free, sizeof(t_chain_metric), (method)0L, A_GIMME, 0);
    
    class_addmethod(c, (method)chain_metric_bang, "bang", 0);
    class_addmethod(c, (method)chain_metric_int, "int", A_LONG, 0);
    class_addmethod(c, (method)chain_metric_notify, "notify", A_CANT, 0);

    CLASS_ATTR_SYM(c, "name", ATTR_SET_OPAQUE_USER, t_chain_metric, s_site_name);
    CLASS_ATTR_SYM(c, "metric_name", 0, t_chain_metric, s_metric_name);
    CLASS_ATTR_ACCESSORS(c, "metric_name", NULL, chain_metric_set_metric_name);

    class_register(CLASS_BOX, c);

    ps_url = gensym("url");
    ps_name = gensym("name");
    ps_db = gensym("db");

    s_chain_metric_class = c;

    return 0;
}

void *chain_metric_new(t_symbol *s, long argc, t_atom *argv)
{
    t_chain_metric *x = (t_chain_metric *)object_alloc(s_chain_metric_class);

    chain_worker_new((t_chain_worker *)x, s, argc, argv);

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

void chain_metric_set_metric_name(t_chain_metric *x, void *attr, long argc, t_atom *argv)
{
    t_symbol *metric_name = atom_getsym(argv);
    if(!x->s_metric_name || x->s_metric_name!=metric_name){
        x->s_metric_name = metric_name;
    }
}

void chain_metric_int(t_chain_metric *x, long n)
{
}

void chain_metric_bang(t_chain_metric *x)
{
}
