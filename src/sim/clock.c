#include "sim/clock.h"

static timestamp_t current_time = 0;

timestamp_t sim_time_now(void) { return current_time; }

void sim_time_advance(timestamp_t dt) { current_time += dt; }

void sim_time_set(timestamp_t t) { current_time = t; }
