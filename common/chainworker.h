#ifndef ____chainworker__
#define ____chainworker__

#include "ext.h"
#include "ext_common.h"
#include "ext_dictobj.h"
#include "ext_database.h"

#endif /* defined(____chainworker__) */

typedef struct
{
    t_object s_obj;
    t_systhread s_systhread_find_site;
    int s_find_site_cancel;
    t_symbol *s_site_name;
    void *s_site_ptr;
    t_dictionary *s_dictionary;
    t_database *s_db;
} t_chain_worker;

void chain_worker_new(t_chain_worker *x, t_symbol *s, long argc, t_atom *argv);
void chain_worker_free(t_chain_worker *x);
void chain_worker_release_site(t_chain_worker *x);
void chain_worker_find_site(t_chain_worker *x);
void chain_worker_notify(t_chain_worker *x, t_symbol *s, t_symbol *msg, void *sender, void *data);
void *chain_worker_find_site_threadproc(t_chain_worker *x);
