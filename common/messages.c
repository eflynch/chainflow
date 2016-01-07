#include "ext.h"
#include "ext_common.h"

void chain_error(const char *fmt, ...)
{
    char buffer[4096];
    va_list args;
    va_start(args, fmt);
    vsprintf(buffer, fmt, args);
    error(buffer);
    va_end(args);
}

void chain_info(const char *fmt, ...)
{
    char buffer[4096];
    va_list args;
    va_start(args, fmt);
    vsprintf(buffer, fmt, args);
    post(buffer);
    va_end(args);
}

void chain_debug(const char *fmt, ...)
{
    char buffer[4096];
    va_list args;
    va_start(args, fmt);
    vsprintf(buffer, fmt, args);
    post(buffer);
    va_end(args);
}
