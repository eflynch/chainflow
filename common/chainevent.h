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
#define URL_SIZE 1024


typedef struct chain_event
{
    time_t s_time;
    char s_href[URL_SIZE];
    char s_timestamp[40];
    double s_value;
} t_chain_event;


#endif /* defined(____chainevent__) */


t_chain_event chain_new_event(time_t s_time, const char *href, const char *timestamp, double value);
