#ifndef __LOG_H
#define __LOG_H
#include <stdio.h>
#define L(...) fprintf(stderr, "%s:%d [\e[35m%s\e[0m] ", __FILE__, __LINE__, __func__); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n");
#endif
