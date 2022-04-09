#ifndef __STATISTICS_H_
#define __STATISTICS_H_

#include "main.h"

void init_stats(struct arguments a);

void close_stats();

void update_statistics(size_t frames_delivered, size_t frames_dropped, size_t frames_delivered_after_full_rank);

#endif