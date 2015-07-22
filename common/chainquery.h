#ifndef ____chainquery__
#define ____chainquery__

#include "chainevent.h"

#endif /* defined(____chainquery__) */

void chain_load_summary(const char *url, t_database *db, long include_nonactive);
void chain_get_websocket(const char *url, const char **wstext);
void chain_get_data(const char *url, time_t start, time_t end, t_chain_event **events, long *num_events);
