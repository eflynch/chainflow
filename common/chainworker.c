#include "chainworker.h"
#include "messages.h"
#include "ext_dictobj.h"

t_symbol *cws_maxchain, *cws_db, *cws_free, *cws_willfree, *cws_deprecated, *cws_name;

static void chain_worker_prenew(t_chain_worker *x){
    cws_maxchain = gensym("maxchain");
    cws_db = gensym("db");
    cws_free = gensym("free");
    cws_willfree = gensym("willfree");
    cws_deprecated = gensym("deprecated");
    cws_name = gensym("name");
}

static void chain_worker_postnew(t_chain_worker *x, t_symbol *site_name){
    x->s_find_site_cancel = false;
    x->s_site_name = site_name;
    x->s_site_ptr = NULL;
    chain_worker_find_site(x); 
}

void chain_worker_new_dict(t_chain_worker *x, t_symbol *s, long argc, t_atom *argv){
    chain_worker_prenew(x);
    t_dictionary *d = NULL;

    if (!(d = object_dictionaryarg(argc, argv)))
        return;

    if(!dictionary_hasentry(d, cws_name))
        return;

    t_symbol *site_name = NULL;

    dictionary_getsym(d, cws_name, &site_name);
    chain_worker_postnew(x, site_name);
}

void chain_worker_new(t_chain_worker *x, t_symbol *s, long argc, t_atom *argv){
    chain_worker_prenew(x);
    long attrstart = attr_args_offset(argc, argv);
    t_symbol *site_name = NULL;

    if (attrstart && atom_gettype(argv) == A_SYM){
        site_name = atom_getsym(argv);
    }

    if (!site_name){
        chain_info("No sitename set");
        site_name = gensym("default_site");
    }

    chain_worker_postnew(x, site_name);
}

void chain_worker_free(t_chain_worker *x){
    chain_worker_release_site(x);
    object_detach(cws_maxchain, x->s_site_name, x);
}

void chain_worker_release_site(t_chain_worker *x){
    if (x->s_systhread_find_site){
        x->s_find_site_cancel = true;
        unsigned int ret;
        systhread_join(x->s_systhread_find_site, &ret);
        x->s_systhread_find_site = NULL;
    }

    if (x->s_dictionary){
        dictobj_release(x->s_dictionary);
        x->s_dictionary = NULL;
    }
}

void chain_worker_find_site(t_chain_worker *x){
    // Make sure it is released
    chain_worker_release_site(x);

    // Find a site
    systhread_create((method) chain_worker_find_site_threadproc, x,
                     0, -32, 0, &x->s_systhread_find_site);
}

void *chain_worker_find_site_threadproc(t_chain_worker *x)
{
    while(1){
        void *site_ptr = object_findregistered(cws_maxchain, x->s_site_name);
        if (site_ptr != x->s_site_ptr){
            object_detach(cws_maxchain, x->s_site_name, x);
            break;
        }
        if (x->s_find_site_cancel){
            break;
        }
    }
    while(1){
        if (x->s_find_site_cancel){
            x->s_find_site_cancel = false;
            break;
        }
        
        void *site_ptr = object_findregistered(cws_maxchain, x->s_site_name);
        if (site_ptr){
            x->s_dictionary = dictobj_findregistered_retain(x->s_site_name);
            if (x->s_dictionary){
                // Subscribe to object
                x->s_site_ptr = object_attach(cws_maxchain, x->s_site_name, x);

                // Cache pointer to database
                dictionary_getobject(x->s_dictionary, cws_db, &x->s_db);
                break;
            }
        }

        systhread_sleep(100);
    }

    x->s_systhread_find_site = NULL;
    systhread_exit(0);
    return NULL;
}

void chain_worker_notify(t_chain_worker *x, t_symbol *s, t_symbol *msg, void *sender, void *data)
{
    // This doesn't get called when you would hope (After the site is freed...)
    if (s == x->s_site_name && msg == cws_free){
        chain_worker_find_site(x);
    }

    // Prepare for site to free
    if (s == x->s_site_name && msg == cws_willfree){
        chain_worker_release_site(x);
    }
}
