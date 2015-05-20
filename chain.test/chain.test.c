/**
    @file
    chain.test - a simple chain_test object
 
    @ingroup    maxchain
 */

#include "ext.h"
#include "ext_common.h"
#include "ext_obex.h"
#include "ext_time.h"
#include "ext_itm.h"

typedef struct chain_test
{
    t_object s_obj;
    long s_value;
} t_chain_test;

void *chain_test_new(t_symbol *s, long argc, t_atom *argv);
void chain_test_free(t_chain_test *x);
void chain_test_int(t_chain_test *x, long n);
void chain_test_bang(t_chain_test *x);

static t_class *s_chain_test_class = NULL;

int C74_EXPORT main(void)
{
    t_class *c;

    c = class_new("chain.test", (method)chain_test_new, (method)chain_test_free, sizeof(t_chain_test), (method)0L, A_GIMME, 0);
    
    class_addmethod(c, (method)chain_test_bang, "bang", 0);
    class_addmethod(c, (method)chain_test_int, "int", A_LONG, 0);

    class_register(CLASS_BOX, c);

    s_chain_test_class = c;

    return 0;
}

void *chain_test_new(t_symbol *s, long argc, t_atom *argv)
{
    t_chain_test *x = (t_chain_test *)object_alloc(s_chain_test_class);

    x->s_value = 0;
    
    return x;
}

void chain_test_free(t_chain_test *x)
{
}

void chain_test_int(t_chain_test *x, long n)
{
    x->s_value = n;
}

void chain_test_bang(t_chain_test *x)
{
    post("value is %ld", x->s_value);
}
