/**
    @file
    chain.info - a simple chain_info object
 
    @ingroup    maxchain
 */

#include "ext.h"
#include "ext_common.h"
#include "ext_dictobj.h"
#include "ext_database.h"

#include "messages.h"
#include "queries.h"

typedef struct chain_info
{
    t_object s_obj;
    void *s_outlet;
    t_symbol *s_site_name;
    t_dictionary *s_dictionary;
    t_database *s_db;
} t_chain_info;

void *chain_info_new(t_symbol *s, long argc, t_atom *argv);
void chain_info_free(t_chain_info *x);
void chain_info_int(t_chain_info *x, long n);
void chain_info_bang(t_chain_info *x);
void chain_info_set_site_name(t_chain_info *x, void *attr, long argc, t_atom *argv);
void chain_info_metrics(t_chain_info *x);

static t_class *s_chain_info_class = NULL;

t_symbol *ps_clear, *ps_append, *ps_url, *ps_name, *ps_db;

int C74_EXPORT main(void)
{
    t_class *c;

    c = class_new("chain.info", (method)chain_info_new, (method)chain_info_free, sizeof(t_chain_info), (method)0L, A_GIMME, 0);
    
    class_addmethod(c, (method)chain_info_bang, "bang", 0);
    class_addmethod(c, (method)chain_info_metrics, "metrics", 0);
    class_addmethod(c, (method)chain_info_int, "int", A_LONG, 0);

    CLASS_ATTR_SYM(c, "name", 0, t_chain_info, s_site_name);
    CLASS_ATTR_ACCESSORS(c, "name", NULL, chain_info_set_site_name);

    class_register(CLASS_BOX, c);

    ps_clear = gensym("clear");
    ps_append = gensym("append");
    ps_url = gensym("url");
    ps_name = gensym("name");
    ps_db = gensym("db");

    s_chain_info_class = c;

    return 0;
}

void *chain_info_new(t_symbol *s, long argc, t_atom *argv)
{
    t_chain_info *x = (t_chain_info *)object_alloc(s_chain_info_class);

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
    
    return x;
}

void chain_info_set_site_name(t_chain_info *x, void *attr, long argc, t_atom *argv)
{
    t_symbol *site_name = atom_getsym(argv);
    if (!x->s_site_name || !site_name || x->s_site_name!=site_name){
        if (x->s_dictionary)
            dictobj_release(x->s_dictionary);
        x->s_dictionary = dictobj_findregistered_retain(site_name);
        if (!x->s_dictionary)
            object_error((t_object*)x, "Chain site not found!");
        x->s_site_name = site_name;

        dictionary_getobject(x->s_dictionary, ps_db, &x->s_db);

    }
}

void chain_info_free(t_chain_info *x)
{
    if (x->s_dictionary)
        dictobj_release(x->s_dictionary);
}

void chain_info_int(t_chain_info *x, long n)
{
}

void chain_info_bang(t_chain_info *x)
{
}

void chain_info_metrics(t_chain_info *x)
{ 
    t_db_result *db_result = NULL;
    t_atom *av = NULL;
    long ac = 0;
    int numrecords;

    char parsestr[1024];
    strcpy(parsestr, "");

    query_list_metrics(x->s_db, &db_result);

    numrecords = db_result_numrecords(db_result);

    for (int i=0; i<numrecords; i++){
        const char *name;
        name = db_result_string(db_result, i, 0);
        strcat(parsestr, " ");
        strcat(parsestr, name);
    }

    atom_setparse(&ac, &av, parsestr);
    outlet_anything(x->s_outlet, gensym("metrics"), ac, av);
    sysmem_freeptr(av);
}
