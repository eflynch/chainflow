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
#include "ext_common.h"
#include "ext_dictobj.h"
#include "ext_database.h"

#include "chainworker.h"

#endif /* defined(____chain_metric__) */

typedef struct chain_metric
{
    t_chain_worker s_worker;
    t_systhread s_systhread_setup;
    int s_setup_cancel;
    void *s_outlet;
    t_symbol *s_metric_name;
    t_symbol *s_measure;
    t_symbol *s_interp;
    float s_pos_x;
    float s_pos_y;
    float s_pos_z;
    float s_radius;
} t_chain_metric;
