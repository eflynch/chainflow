#include "pseudoclock.h"

time_t local_now(void){
    time_t rawtime;
    time(&rawtime);
    return rawtime;
}

/*
    Accept strings of the following formats:

    YYYY-MM-DDTHH:MM:SS{.n+}*[+-](hhmm|hh:mm)
*/
time_t time_from_string(const char *string){
    char s[40];
    char *idx = s;

    if (strlen(string) < 19){
        chain_error("Bad timestamp");
        return -1;
    }

    // copy over date + time
    memcpy(idx, string, 19);
    idx += 19;

    // deal with fractional seconds
    if (*(string + 19) == '.'){
        long frac;
        char *frac_end;
        frac = strtol(string + 20, &frac_end, 10);
    }

    // Deal with timezone
    // Deal with format edgecase (timezone +hh:mm rather than +hhmm)
    if (*(string + strlen(string) - 3) == ':'){
        memcpy(idx, string + strlen(string) - 6, 3);
        idx += 3;
        memcpy(idx, string + strlen(string) - 2, 3);
        idx += 3;
    } else {
        memcpy(idx, string + strlen(string) - 5, 6);
        idx += 6;
    }

    struct tm tm;
    char *fmt = strptime(s, "%FT%T%z", &tm);
    if(fmt == NULL){
        chain_error("Timestamp %s may not have been parsed correctly", string);
    }
    return mktime(&tm);
}

char *string_from_time(time_t rawtime){
    char *buffer = malloc(sizeof(char) * 41);
    strftime(buffer, 40, "%FT%T+00:00", gmtime(&rawtime));
    return buffer;
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
