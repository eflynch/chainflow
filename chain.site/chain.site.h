#ifndef __CHAIN_SITE_H__
#define __CHAIN_SITE_H__

#include "ext.h"
#include "ext_dictobj.h"
#include "ext_database.h"

#endif

typedef struct chain_site
{
    t_object s_obj;
    t_systhread s_systhread_load;
    t_systhread s_systhread_play;
    int s_play_cancel;
    void *s_outlet_busy;
    void *s_outlet;
    t_symbol *s_site_name;
    t_symbol *s_url;
    t_symbol *s_wshref;
    t_dictionary *s_dictionary;
    t_database *s_db;
} t_chain_site;
