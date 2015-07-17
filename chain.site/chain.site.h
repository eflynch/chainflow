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

#include "pri_queue.h"
#include "pseudoclock.h"
#include "chainevent.h"

typedef struct chain_site
{
    t_object s_obj;
    t_systhread s_systhread_load;
    t_systhread s_systhread_play;
    t_systhread s_systhread_historical;
    int s_play_cancel;
    int s_historical_cancel;
    int s_live;
    void *s_outlet_busy;
    void *s_outlet;
    t_symbol *s_site_name;
    t_symbol *s_url;
    t_symbol *s_wshref;
    t_dictionary *s_dictionary;
    t_database *s_db;
    void *s_reg_ptr;
    float s_historical_ts;
    long s_historical_start;
    t_pseudo_clk *s_historical_clk;
    t_linklist *s_historical_clklist;
} t_chain_site;

#endif /* defined(____chain_site__) */

int chain_site_update_sensors(t_chain_site *x, t_chain_event *e);
