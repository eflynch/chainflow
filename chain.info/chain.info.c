/**
    @file
    chain.test - a simple chain_info object
 
    @ingroup    maxchain
 */

#include "ext.h"
#include "ext_common.h"
#include "ext_obex.h"
#include "ext_time.h"
#include "ext_itm.h"
#include "ext_dictobj.h"

typedef struct chain_info
{
    t_object s_obj;
    long s_value;
    t_symbol *s_dict_name;
    t_dictionary *s_dictionary;
} t_chain_info;

void *chain_info_new(t_symbol *s, long argc, t_atom *argv);
void chain_info_free(t_chain_info *x);
void chain_info_int(t_chain_info *x, long n);
void chain_info_bang(t_chain_info *x);
void chain_info_set_dict_name(t_chain_info *x, void *attr, long argc, t_atom *argv);

static t_class *s_chain_info_class = NULL;

int C74_EXPORT main(void)
{
    t_class *c;

    c = class_new("chain.info", (method)chain_info_new, (method)chain_info_free, sizeof(t_chain_info), (method)0L, A_GIMME, 0);
    
    class_addmethod(c, (method)chain_info_bang, "bang", 0);
    class_addmethod(c, (method)chain_info_int, "int", A_LONG, 0);

    CLASS_ATTR_SYM(c, "name", 0, t_chain_info, s_dict_name);
    CLASS_ATTR_ACCESSORS(c, "name", NULL, chain_info_set_dict_name);

    class_register(CLASS_BOX, c);

    s_chain_info_class = c;

    return 0;
}

void *chain_info_new(t_symbol *s, long argc, t_atom *argv)
{
    t_chain_info *x = (t_chain_info *)object_alloc(s_chain_info_class);

    long attrstart = attr_args_offset(argc, argv);
    t_symbol *dict_name = NULL;

    if (attrstart && atom_gettype(argv) == A_SYM)
        dict_name = atom_getsym(argv);

    x->s_dictionary = NULL;

    attr_args_process(x, argc, argv);
    if (!x->s_dict_name) {
        if (dict_name)
            object_attr_setsym(x, gensym("name"), dict_name);
    }

    x->s_value = 0;
    
    return x;
}

void chain_info_set_dict_name(t_chain_info *x, void *attr, long argc, t_atom *argv)
{
    t_symbol *dict_name = atom_getsym(argv);
    if (!x->s_dict_name || !dict_name || x->s_dict_name!=dict_name){
        dictobj_release(x->s_dictionary);
        x->s_dictionary = dictobj_findregistered_retain(dict_name);
        x->s_dict_name = dict_name;
    }
}

void chain_info_free(t_chain_info *x)
{
    dictobj_release(x->s_dictionary);
}

void chain_info_int(t_chain_info *x, long n)
{
    x->s_value = n;
}

void chain_info_bang(t_chain_info *x)
{
    t_symbol *url;
    dictionary_getsym(x->s_dictionary, gensym("url"), &url);
    post("url is %s", url);
}
