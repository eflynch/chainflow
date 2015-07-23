#include <math.h>

double chain_mean(double *argv, long argc)
{
    if (argc <= 0){
        return 0.0;
    }
    double acc = 0.0;
    for (long i=0; i<argc; i++){
        acc += *(argv+i);
    }
    return acc / (double) argc;
}

double chain_std(double *argv, long argc)
{
    if (argc <= 0){
        return 0.0;
    }
    double mean = chain_mean(argv, argc);

    double acc = 0.0;
    for (long i=0; i<argc; i++){
        acc += (*(argv+i) - mean) * (*(argv+i) - mean);
    }
    return sqrt(acc / (double) argc);
}

double chain_max(double *argv, long argc)
{
    if (argc <= 0){
        return 0.0;
    }
    double max = *argv;
    for (long i=1; i<argc; i++){
        if (*(argv+i) > max){
            max = *(argv+i);
        }
    }
    return max;
}

double chain_min(double *argv, long argc)
{
    if (argc <= 0){
        return 0.0;
    }
    double min = *argv;
    for (long i=1; i<argc; i++){
        if (*(argv+i) < min){
            min = *(argv+i);
        }
    }
    return min;
}
