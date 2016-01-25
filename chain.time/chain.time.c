/**
 @file
 chain.time
 
 @ingroup    chainflow
 */
#include <time.h>

#include "ext.h"
#include "ext_common.h"
#include "ext_dictobj.h"

#include "messages.h"
#include "chainlib.h"
#include "chainworker.h"
#include "pseudoclock.h"

typedef struct chain_time
{
    t_chain_worker s_worker;
    void *s_outlet;
    void *s_outlet2;
    void *s_outlet3;
} t_chain_time;

// Create + Destroy
void *chain_time_new(t_symbol *s, long argc, t_atom *argv);
void chain_time_free(t_chain_time *x);

// Methods
void chain_time_int(t_chain_time *x, long n);
void chain_time_bang(t_chain_time *x);
void chain_time_parse(t_chain_time *x, t_symbol *t);
void chain_time_format(t_chain_time *x, long timestamp);
void chain_time_now(t_chain_time *x);
void chain_time_historical_now(t_chain_time *x);
void chain_time_time_of_day(t_chain_time *x, long tz_shift);
void chain_time_historical_time_of_day(t_chain_time *x, long tz_shift);

// Notify
void chain_time_notify(t_chain_time *x, t_symbol *s, t_symbol *msg, void *sender, void *data);

static t_class *s_chain_time_class = NULL;

t_symbol *ps_clk;

int C74_EXPORT main(void)
{
    t_class *c;
    
    c = class_new("chain.time", (method)chain_time_new, (method)chain_time_free,
                  sizeof(t_chain_time), (method)0L, A_GIMME, 0);
    
    class_addmethod(c, (method)chain_time_bang, "bang", 0);
    class_addmethod(c, (method)chain_time_int, "int", A_LONG, 0);
    class_addmethod(c, (method)chain_time_notify, "notify", A_CANT, 0);
    class_addmethod(c, (method)chain_time_parse, "parse", A_SYM, 0);
    class_addmethod(c, (method)chain_time_format, "format", A_LONG, 0);
    class_addmethod(c, (method)chain_time_now, "now", 0);
    class_addmethod(c, (method)chain_time_historical_now, "historical_now", 0);
    class_addmethod(c, (method)chain_time_time_of_day, "tod", A_LONG, 0);
    class_addmethod(c, (method)chain_time_historical_time_of_day, "historical_tod", A_LONG, 0);

    CLASS_ATTR_SYM(c, "name", ATTR_SET_OPAQUE_USER, t_chain_time, s_worker.s_site_name);
    
    class_register(CLASS_BOX, c);
    s_chain_time_class = c;

    ps_clk = gensym("clk");
    
    return 0;
}

void *chain_time_new(t_symbol *s, long argc, t_atom *argv)
{
    t_chain_time *x = (t_chain_time *)object_alloc(s_chain_time_class);
    
    chain_worker_new((t_chain_worker *)x, s, argc, argv);
    
    attr_args_process(x, argc, argv);
    
    x->s_outlet3 = outlet_new(x, NULL);
    x->s_outlet2 = outlet_new(x, NULL);
    x->s_outlet = outlet_new(x, "int");
    
    return x;
}

void chain_time_free(t_chain_time *x)
{
    chain_worker_free((t_chain_worker *)x);
}

void chain_time_notify(t_chain_time *x, t_symbol *s, t_symbol *msg,
                         void *sender, void *data)
{
    chain_worker_notify((t_chain_worker *)x, s, msg, sender, data);
}

void chain_time_int(t_chain_time *x, long n)
{
}

void chain_time_bang(t_chain_time *x)
{
}

void chain_time_parse(t_chain_time *x, t_symbol *t)
{
    float fracsecs;
    time_t timestamp = time_from_string(t->s_name, &fracsecs);
    outlet_int(x->s_outlet, (long) timestamp);
}

void chain_time_format(t_chain_time *x, long timestamp)
{
    char *buffer = string_from_time((time_t) timestamp);
    outlet_anything(x->s_outlet2, gensym(buffer), 0, NULL);
    free(buffer);
}

void chain_time_now(t_chain_time *x)
{
    time_t timestamp = local_now();
    char *buffer = string_from_time(timestamp);
    outlet_int(x->s_outlet, (long) timestamp);
    outlet_anything(x->s_outlet2, gensym(buffer), 0, NULL);
    free(buffer);
}

void chain_time_historical_now(t_chain_time *x)
{
    t_object *obj = NULL;
    dictionary_getobject(x->s_worker.s_dictionary, ps_clk, &obj);
    if (!obj){
        return;
    }
    t_pseudo_clk *clk = (t_pseudo_clk *)obj;

    time_t timestamp = pseudo_now(clk);
    char *buffer = string_from_time(timestamp);
    outlet_int(x->s_outlet, (long) timestamp);
    outlet_anything(x->s_outlet2, gensym(buffer), 0, NULL);
    free(buffer);
}

void chain_time_time_of_day(t_chain_time *x, long tz_shift)
{
    time_t timestamp = local_now();
    time_t seconds = tod_from_time(timestamp);

    outlet_int(x->s_outlet3, ((long)seconds + tz_shift) % 86400);
}

void chain_time_historical_time_of_day(t_chain_time *x, long tz_shift)
{
    t_object *obj = NULL;
    dictionary_getobject(x->s_worker.s_dictionary, ps_clk, &obj);
    if (!obj){
        return;
    }
    t_pseudo_clk *clk = (t_pseudo_clk *)obj;

    time_t timestamp = pseudo_now(clk);
    time_t seconds = tod_from_time(timestamp);

    outlet_int(x->s_outlet3, ((long)seconds + tz_shift) % 86400);
}
