#include "chainevent.h"

t_chain_event *chain_new_event(time_t s_time, const char *href, const char *timestamp, double value){
    t_chain_event *e = malloc(sizeof(t_chain_event));
    if (!e)
        return NULL;

    e->s_href = malloc((strlen(href) + 1) * sizeof(char));
    strcpy(e->s_href, href);

    e->s_time = s_time;

    e->s_timestamp = malloc((strlen(timestamp) + 1) * sizeof(char));
    strcpy(e->s_timestamp, timestamp);

    e->s_value = value;

    return e;
}

void chain_free_event(t_chain_event *e){
    free(e->s_href);
    free(e->s_timestamp);
    free(e);
}
