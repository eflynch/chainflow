/**
 @file
 chain.device - 
 
 @ingroup    maxchain
 */

#include "ext.h"
#include "ext_common.h"
#include "ext_obex.h"
#include "ext_time.h"
#include "ext_itm.h"

typedef struct chain_device
{
    t_object s_obj;
} t_chain_device;

void *chain_device_new(t_symbol *s, long argc, t_atom *argv);
void chain_device_free(t_chain_device *x);
void chain_device_int(t_chain_device *x, long n);
void chain_device_bang(t_chain_device *x);

static t_class *s_chain_device_class = NULL;

int C74_EXPORT main(void)
{
    t_class *c;
    
    c = class_new("chain.device", (method)chain_device_new, (method)chain_device_free, sizeof(t_chain_device), (method)0L, A_GIMME, 0);
    
    class_addmethod(c, (method)chain_device_bang, "bang", 0);
    class_addmethod(c, (method)chain_device_int, "int", A_LONG, 0);
    
    class_register(CLASS_BOX, c);
    s_chain_device_class = c;
    
    return 0;
}

void *chain_device_new(t_symbol *s, long argc, t_atom *argv)
{
    t_chain_device *x = (t_chain_device *)object_alloc(s_chain_device_class);
    return x;
}

void chain_device_free(t_chain_device *x)
{
}

void chain_device_int(t_chain_device *x, long n)
{
}

void chain_device_bang(t_chain_device *x)
{
}
