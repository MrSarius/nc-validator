#ifndef __UTIL_H_
#define __UTIL_H_

#include <stdbool.h>

void setVerbose(bool v);

void logger(const char* format, ...);

#endif