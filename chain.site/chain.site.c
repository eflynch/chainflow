/**
 @file
 chain.site - a simple chain_site object
 
 @ingroup    maxchain
 */

#include "ext.h"
#include "ext_common.h"
#include "ext_dictobj.h"
#include "ext_database.h"

#include "messages.h"
#include "queries.h"
#include "chainsummary.h"

#define URL_SIZE 1024


typedef struct chain_site
{
    t_object s_obj;
    void *s_outlet;
    t_symbol *s_site_name;
    t_symbol *s_url;
    t_dictionary *s_dictionary;
    t_database *s_db;
} t_chain_site;

void *chain_site_new(t_symbol *s, long argc, t_atom *argv);
void chain_site_free(t_chain_site *x);
void chain_site_load(t_chain_site *x);
void chain_site_bang(t_chain_site *x);
void chain_site_int(t_chain_site *x, long n);
void chain_site_set_site_name(t_chain_site *x, void *attr, long argc, t_atom *argv);
void chain_site_set_url(t_chain_site *x, void *attr, long argc, t_atom *argv);

static t_class *s_chain_site_class = NULL;

t_symbol *ps_url, *ps_db;

int C74_EXPORT main(void)
{
    ps_url = gensym("url");
    ps_db = gensym("db");
    
    t_class *c;

    c = class_new("chain.site", (method)chain_site_new, (method)chain_site_free, \
                  sizeof(t_chain_site), (method)NULL, A_GIMME, 0);
    
    class_addmethod(c, (method)chain_site_load, "load", 0);
    class_addmethod(c, (method)chain_site_int, "int", A_LONG, 0);
    class_addmethod(c, (method)chain_site_bang, "bang", 0);

    CLASS_ATTR_SYM(c, "name", 0, t_chain_site, s_site_name);
    CLASS_ATTR_ACCESSORS(c, "name", NULL, chain_site_set_site_name);

    CLASS_ATTR_SYM(c, "url", 0, t_chain_site, s_url);
    CLASS_ATTR_ACCESSORS(c, "url", NULL, chain_site_set_url);

    class_register(CLASS_BOX, c);
    
    s_chain_site_class = c;
    
    return 0;
}

void chain_site_int(t_chain_site *x, long n)
{
}

void chain_site_bang(t_chain_site *x)
{
}

void *chain_site_new(t_symbol *s, long argc, t_atom *argv)
{
    t_chain_site *x = (t_chain_site *)object_alloc(s_chain_site_class);

    long attrstart = attr_args_offset(argc, argv);
    t_symbol *site_name = NULL;

    if (attrstart && atom_gettype(argv) == A_SYM)
        site_name = atom_getsym(argv);

    if (!x->s_site_name) {
        if (site_name)
            object_attr_setsym(x, gensym("name"), site_name);
        else
            object_attr_setsym(x, gensym("name"), symbol_unique());
    }

    attr_args_process(x, argc, argv);

    x->s_outlet = outlet_new(x, NULL);
    return x;
}

void chain_site_set_site_name(t_chain_site *x, void *attr, long argc, t_atom *argv)
{
    t_symbol *site_name = atom_getsym(argv);
    if (!x->s_site_name || !site_name || x->s_site_name!=site_name){
        if (x->s_dictionary)
            object_free(x->s_dictionary);
        x->s_dictionary = dictionary_new();
        x->s_dictionary = dictobj_register(x->s_dictionary, &site_name);
        x->s_site_name = site_name;

        if(x->s_db)
            db_close(&x->s_db);
        db_open(site_name, NULL, &x->s_db);
        query_init_database(x->s_db);

        dictionary_deleteentry(x->s_dictionary, ps_db);
        dictionary_appendobject(x->s_dictionary, ps_db, x->s_db);
    }
}

void chain_site_set_url(t_chain_site *x, void *attr, long argc, t_atom *argv)
{
    t_symbol *url_sym = atom_getsym(argv);

    if (!x->s_url || !url_sym || x->s_url!=url_sym){
        dictionary_deleteentry(x->s_dictionary, ps_url);
        dictionary_appendsym(x->s_dictionary, ps_url, url_sym);
        x->s_url = url_sym;
        char url_str[URL_SIZE];
        strncpy_zero(url_str, url_sym->s_name, sizeof(url_str));
        chain_debug("Set url to: %s", url_str);
    }
}

void chain_site_load(t_chain_site *x)
{
    if(!x->s_db){
        chain_error("database not open");
        return;
    }
    if(!x->s_url){
        chain_error("no url set");
        return;
    }
    chain_load_summary(x->s_url->s_name, x->s_db);
}

void chain_site_free(t_chain_site *x)
{
    if (x->s_dictionary)
        object_free(x->s_dictionary);
}
