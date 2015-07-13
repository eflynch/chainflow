//
//  chainevent.h
//
//
//  Created by Evan Lynch on 6/22/15.
//
//

#ifndef ____chainevent__
#define ____chainevent__

#include <time.h>


typedef struct chain_event
{
    time_t s_time;
    const char *s_href;
    const char *s_timestamp;
    double s_value;
} t_chain_event;


#endif /* defined(____chainevent__) */


t_chain_event *chain_new_event(time_t s_time, const char *href, const char *timestamp, double value);

void chain_free_event(t_chain_event *e);
