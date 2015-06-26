/**
 @file
 chain.metric - a chain_metric object
 
 @ingroup    maxchain
 */

#include "chain.metric.h"

#include "messages.h"

void *chain_metric_new(t_symbol *s, long argc, t_atom *argv);
void chain_metric_free(t_chain_metric *x);
void chain_metric_bang(t_chain_metric *x);
void chain_metric_int(t_chain_metric *x, long n);
void chain_metric_set_site_name(t_chain_metric *x, void *attr, long argc, t_atom *argv);
void chain_metric_set_metric_name(t_chain_metric *x, void *attr, long argc, t_atom *argv);
void *chain_metric_setup_threadproc(t_chain_metric *x);

static t_class *s_chain_metric_class = NULL;

t_symbol *ps_url, *ps_db, *ps_maxchain, *ps_name;

int C74_EXPORT main(void)
{
    t_class *c;

    c = class_new("chain.metric", (method)chain_metric_new, (method)chain_metric_free, sizeof(t_chain_metric), (method)0L, A_GIMME, 0);
    
    class_addmethod(c, (method)chain_metric_bang, "bang", 0);
    class_addmethod(c, (method)chain_metric_int, "int", A_LONG, 0);

    CLASS_ATTR_SYM(c, "name", 0, t_chain_metric, s_site_name);
    CLASS_ATTR_ACCESSORS(c, "name", NULL, chain_metric_set_site_name);
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

void chain_metric_set_site_name(t_chain_metric *x, void *attr, long argc, t_atom *argv)
{
    t_symbol *site_name = atom_getsym(argv);
    if (!x->s_site_name || x->s_site_name!=site_name){
        x->s_site_name = site_name;

        if (x->s_systhread_setup == NULL){
            systhread_create((method) chain_metric_setup_threadproc, x, 0, 0, 0, &x->s_systhread_setup);
        }
        

    }
}

void chain_metric_set_metric_name(t_chain_metric *x, void *attr, long argc, t_atom *argv)
{
    t_symbol *metric_name = atom_getsym(argv);
    if(!x->s_metric_name || x->s_metric_name!=metric_name){
        x->s_metric_name = metric_name;
    }
}

int chain_metric_get_dict(t_chain_metric *x)
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

void chain_metric_free(t_chain_metric *x)
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

void chain_metric_int(t_chain_metric *x, long n)
{
}

void chain_metric_bang(t_chain_metric *x)
{
}

void *chain_metric_setup_threadproc(t_chain_metric *x)
{
    int err=1;
    while(err){
        err = chain_metric_get_dict(x);
        systhread_sleep(1000);

        if (x->s_setup_cancel)
            break;
    }

    systhread_exit(0);
    return NULL;
}
