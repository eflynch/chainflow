/**
    @file
    test - a test object
    jeremy bernstein - jeremy@bootsquad.com 

    @ingroup    examples    
*/

#include "ext.h"                            // standard Max include, always required
#include "ext_obex.h"                       // required for new style Max object

////////////////////////// object struct
typedef struct _test 
{
    t_object    ob;
    t_atom      val;
    t_symbol    *name;
    void        *out;
} t_test;

///////////////////////// function prototypes
//// standard set
void *test_new(t_symbol *s, long argc, t_atom *argv);
void test_free(t_test *x);
void test_assist(t_test *x, void *b, long m, long a, char *s);

void test_int(t_test *x, long n);
void test_float(t_test *x, double f);
void test_anything(t_test *x, t_symbol *s, long ac, t_atom *av);
void test_bang(t_test *x);
void test_identify(t_test *x);
void test_dblclick(t_test *x);
void test_acant(t_test *x);

//////////////////////// global class pointer variable
void *test_class;


int C74_EXPORT main(void)
{   
    t_class *c;
    
    c = class_new("test", (method)test_new, (method)test_free, (long)sizeof(t_test), 
                  0L /* leave NULL!! */, A_GIMME, 0);
    
    class_addmethod(c, (method)test_bang,          "bang", 0);
    class_addmethod(c, (method)test_int,           "int",      A_LONG, 0);  
    class_addmethod(c, (method)test_float,         "float",    A_FLOAT, 0);  
    class_addmethod(c, (method)test_anything,      "anything", A_GIMME, 0);  
    class_addmethod(c, (method)test_identify,      "identify", 0);
    CLASS_METHOD_ATTR_PARSE(c, "identify", "undocumented", gensym("long"), 0, "1");

    // we want to 'reveal' the otherwise hidden 'xyzzy' method
    class_addmethod(c, (method)test_anything,      "xyzzy", A_GIMME, 0);
    // here's an otherwise undocumented method, which does something that the user can't actually 
    // do from the patcher however, we want them to know about it for some weird documentation reason. 
    // so let's make it documentable. it won't appear in the quickref, because we can't send it from a message.
    class_addmethod(c, (method)test_acant,         "blooop", A_CANT, 0);   
    CLASS_METHOD_ATTR_PARSE(c, "blooop", "documentable", gensym("long"), 0, "1");

    /* you CAN'T call this from the patcher */
    class_addmethod(c, (method)test_assist,            "assist",       A_CANT, 0);  
    class_addmethod(c, (method)test_dblclick,          "dblclick",     A_CANT, 0);
    
    CLASS_ATTR_SYM(c, "name", 0, t_test, name);
    
    class_register(CLASS_BOX, c);
    test_class = c;

    return 0;
}

void test_acant(t_test *x)
{
    object_post((t_object *)x, "can't touch this!");
}

void test_assist(t_test *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) { //inlet
        sprintf(s, "I am inlet %ld", a);
    } 
    else {  // outlet
        sprintf(s, "I am outlet %ld", a);           
    }
}

void test_free(t_test *x)
{
    ;
}

void test_dblclick(t_test *x)
{
    object_post((t_object *)x, "I got a double-click");
}

void test_int(t_test *x, long n)
{
    atom_setlong(&x->val, n);
    test_bang(x);
}

void test_float(t_test *x, double f)
{
    atom_setfloat(&x->val, f);
    test_bang(x);
}

void test_anything(t_test *x, t_symbol *s, long ac, t_atom *av)
{
    if (s == gensym("xyzzy")) {
        object_post((t_object *)x, "A hollow voice says 'Plugh'");
    } else {
        atom_setsym(&x->val, s);
        test_bang(x);
    }
}

void test_bang(t_test *x)
{
    switch (x->val.a_type) {
        case A_LONG: outlet_int(x->out, atom_getlong(&x->val)); break;
        case A_FLOAT: outlet_float(x->out, atom_getfloat(&x->val)); break;
        case A_SYM: outlet_anything(x->out, atom_getsym(&x->val), 0, NULL); break;
        default: break;
    }
}

void test_identify(t_test *x)
{
    object_post((t_object *)x, "my name is %s", x->name->s_name);
}

void *test_new(t_symbol *s, long argc, t_atom *argv)
{
    t_test *x = NULL;
    
    if (x = (t_test *)object_alloc(test_class)) {
        x->name = gensym("");
        if (argc && argv) {
            x->name = atom_getsym(argv);
        }
        if (!x->name || x->name == gensym(""))
            x->name = symbol_unique();
        
        atom_setlong(&x->val, 0);
        x->out = outlet_new(x, NULL);
    }
    return (x);
}
