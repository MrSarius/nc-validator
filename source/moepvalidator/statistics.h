#ifndef __STATISTICS_H_
#define __STATISTICS_H_

#include "main.h"

void init_stats(struct arguments);

void close_stats();

void update_statistics(size_t, size_t);

#endif