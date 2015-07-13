#ifndef ____pseudoclock__
#define ____pseudoclock__

#include <time.h>

typedef struct pseudo_clk {
    time_t local_start;
    time_t pseudo_start;
    float scale;
} t_pseudo_clk;

#endif /* defined(____pseudoclock__) */


time_t local_now(void);

time_t time_from_string(const char *string);

time_t pseudo_now(t_pseudo_clk *clk);

t_pseudo_clk *new_clk(time_t start, float scale);

void free_clk(t_pseudo_clk *clk);
