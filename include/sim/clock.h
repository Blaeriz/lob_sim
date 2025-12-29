#ifndef CLOCK_H
#define CLOCK_H

#include "../common/types.h"

timestamp_t sim_time_now(void);
void sim_time_advance(timestamp_t dt);
void sim_time_set(timestamp_t t);

#endif
