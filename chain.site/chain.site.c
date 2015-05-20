/**
 @file
 chain.test - a simple chain_site object
 
 @ingroup    maxchain
 */

#include "ext.h"
#include "ext_common.h"
#include "ext_obex.h"
#include "ext_time.h"
#include "ext_itm.h"

typedef struct chain_site
{
    t_object s_obj;
    long s_value;
} t_chain_site;

void *chain_site_new(t_symbol *s, long argc, t_atom *argv);
void chain_site_free(t_chain_site *x);
void chain_site_int(t_chain_site *x, long n);
void chain_site_bang(t_chain_site *x);

static t_class *s_chain_site_class = NULL;

int C74_EXPORT main(void)
{
    t_class *c;
    
    c = class_new("chain.site", (method)chain_site_new, (method)chain_site_free, sizeof(t_chain_site), (method)0L, A_GIMME, 0);
    
    class_addmethod(c, (method)chain_site_bang, "bang", 0);
    class_addmethod(c, (method)chain_site_int, "int", A_LONG, 0);
    
    class_register(CLASS_BOX, c);
    
    s_chain_site_class = c;
    
    return 0;
}

void *chain_site_new(t_symbol *s, long argc, t_atom *argv)
{
    t_chain_site *x = (t_chain_site *)object_alloc(s_chain_site_class);
    
    x->s_value = 0;
    
    return x;
}

void chain_site_free(t_chain_site *x)
{
}

void chain_site_int(t_chain_site *x, long n)
{
    x->s_value = n;
}

void chain_site_bang(t_chain_site *x)
{
    post("value is %ld", x->s_value);
}
