#ifndef LATENCY_H
#define LATENCY_H

#include <stdint.h>
#include <stddef.h>

#define LATENCY_MAX_SAMPLES 100000

typedef struct {
    // Counters
    size_t count;           // Total operations recorded
    
    // Basic stats (updated on every record)
    uint64_t min_ns;
    uint64_t max_ns;
    uint64_t sum_ns;
    
    // Sample storage for percentiles
    uint64_t *samples;      // Array of samples
    size_t sample_count;    // How many samples stored
    size_t sample_capacity; // Max samples (LATENCY_MAX_SAMPLES)
    
} latency_tracker_t;

void latency_init(latency_tracker_t *tracker);

void latency_record(latency_tracker_t *tracker, uint64_t latency_ns);

double latency_mean(latency_tracker_t *tracker);
uint64_t latency_percentile(latency_tracker_t *tracker, double p);

void latency_print(latency_tracker_t *tracker, const char *name);

void latency_free(latency_tracker_t *tracker);

uint64_t time_now_ns(void);

#endif