//
//  chain.metric.h
//  
//
//  Created by Evan Lynch on 6/22/15.
//
//

#ifndef ____chain_metric__
#define ____chain_metric__

#include <stdio.h>
#include "ext.h"
#include "ext_dictobj.h"
#include "ext_database.h"

#endif /* defined(____chain_metric__) */

typedef struct chain_metric
{
    t_object s_obj;
    t_systhread s_systhread_setup;
    int s_setup_cancel;
    void *s_outlet;
    t_symbol *s_site_name;
    t_dictionary *s_dictionary;
    t_database *s_db;
} t_chain_metric;
