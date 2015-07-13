//
//  chain.site.h
//
//
//  Created by Evan Lynch on 6/22/15.
//
//

#ifndef ____chain_site__
#define ____chain_site__

#include <time.h>
#include <stdio.h>
#include "ext.h"
#include "ext_dictobj.h"
#include "ext_database.h"

#endif /* defined(____chain_site__) */

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
    void *s_reg_ptr
} t_chain_site;

typedef struct chain_event
{
    time_t s_time;
    const char *s_href;
    const char *s_timestamp;
    double s_value;
} t_chain_event;
 
int chain_site_update_sensors(t_chain_site *x, const char *href,
                              const char *timestamp, double value);
