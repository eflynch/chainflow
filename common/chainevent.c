#include "chainevent.h"

t_chain_event chain_new_event(time_t s_time, const char *href, const char *timestamp,
                               double value){
    t_chain_event e;
    e.s_time = s_time;
    e.s_value = value;
    memcpy(e.s_href, href, strlen(href) + 1);
    memcpy(e.s_timestamp, timestamp, strlen(timestamp) + 1);

    return e;
}
