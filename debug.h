#ifndef DEBUG_H
#define DEBUG_H

#ifdef DEBUG
#include <stdio.h>
#define debugp(format, ...) \
    fprintf(stderr, "%s:%d " format "\n", \
            __FILE__, __LINE__, \
            ##__VA_ARGS__)
#else
#define debugp(...) do {} while(0)
#endif

#endif
