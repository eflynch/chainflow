// bang.c -- A very simple Max external object.
// Outputs a bang when a bang is received.

#include "ext.h" // Required for all Max external objects
#include "ext_obex.h"

static t_class *this_class; // Required. Global pointing to this class

typedef struct _bang // Data structure for this object
{
    t_object b_ob; // Must always be the first field; used by Max
    void *b_out; // Pointer to outlet, need one for each outlet
} t_bang;

// Prototypes for methods: need a method for each incoming message
void *bang_new(); // object creation method
void bang_bang(t_bang *bang); // method for bang message

int C74_EXPORT main()
{
    t_class *c;

    c = class_new("bang", (method)bang_new, (method)NULL, sizeof(t_bang), 0L, 0);
    class_addmethod(c, (method)bang_bang, "bang", 0);

    class_register(CLASS_BOX, c);

    this_class = c;

    return(0);
}

void *bang_new()
{
    t_bang *bang;

    // create the new instance and return a pointer to it
    bang = (t_bang *)object_alloc(this_class);

    bang->b_out = bangout(bang); // create a bang outlet

    return(bang); // must return a pointer to the new instance
}

void bang_bang(t_bang *bang)
{
    outlet_bang(bang->b_out); // send a bang to the outlet bang->b_out
}
