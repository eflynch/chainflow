#include <time.h>
#include "pseudoclock.h"
#include "messages.h"

time_t local_now(void){
    time_t rawtime;
    time(&rawtime);
    return rawtime;
}

time_t time_from_string(const char *string){
    struct tm tm;
    char *err = strptime(string, "%FT%T%z", &tm);
    if(err){
        chain_error("Not sure why but that date string didn't work");
        return 0;
    }
    return mktime(&tm);
}

time_t pseudo_now(t_pseudo_clk *clk){
    int pseudo_elapsed_time = (int)((float)(local_now() - clk->local_start) * clk->scale);
    return clk->pseudo_start + pseudo_elapsed_time;
}

t_pseudo_clk *new_clk(time_t start, float scale){
    t_pseudo_clk *clk = malloc(sizeof(t_pseudo_clk));
    clk->local_start = local_now();
    clk->pseudo_start = start;
    clk->scale = scale;
    return clk;
}

void free_clk(t_pseudo_clk *clk){
    free(clk);
};
