#define _GNU_SOURCE
#include "bench/latency.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdint.h>  

// time_now_ns():
//   - Use clock_gettime(CLOCK_MONOTONIC, &ts)
//   - Convert to nanoseconds: ts.tv_sec * 1e9 + ts.tv_nsec
//   - Return as uint64_t
uint64_t time_now_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

// latency_init():
//   - Allocate samples trackeray with malloc (LATENCY_MAX_SAMPLES * sizeof(uint64_t))
//   - Set sample_capacity to LATENCY_MAX_SAMPLES
//   - Set count, sample_count, sum_ns to 0
//   - Set min_ns to UINT64_MAX (so first sample becomes min)
//   - Set max_ns to 0
void latency_init(latency_tracker_t *tracker) {
    tracker->samples = malloc(LATENCY_MAX_SAMPLES * sizeof(uint64_t));

    if (!tracker->samples) {
        printf("Couldnt allocate memory for latency tracker");
        return;
    }

    tracker->sample_capacity = LATENCY_MAX_SAMPLES;
    tracker->count = 0; tracker->sample_count = 0; tracker->sum_ns = 0;
    tracker->min_ns = UINT64_MAX;
    tracker->max_ns = 0;
}

// latency_record():
//   - Increment count
//   - Add latency_ns to sum_ns
//   - Update min_ns if latency_ns < min_ns
//   - Update max_ns if latency_ns > max_ns
//   - If sample_count < sample_capacity, store sample and increment sample_count
//   - (What do you do when full? Wrap around or stop?)
void latency_record(latency_tracker_t *tracker, uint64_t latency_ns) {
    tracker->count += 1;
    tracker->sum_ns += latency_ns;
    if (latency_ns < tracker->min_ns) {
        tracker->min_ns = latency_ns;
    }
    if (latency_ns > tracker->max_ns) {
        tracker->max_ns = latency_ns;
    }
    tracker->samples[tracker->sample_count] = latency_ns;
    tracker->sample_count = (tracker->sample_count + 1) % tracker->sample_capacity;
}

// latency_mean():
//   - Return (double)sum_ns / count
//   - Handle count == 0 case

double latency_mean(latency_tracker_t *tracker) {
    if (tracker->count == 0) {
        return 0.0;
    }
    return (double)(tracker->sum_ns/(double)tracker->count);
}

int cmp_u64(const void *a, const void *b) {
    uint64_t x = *(const uint64_t *)a;
    uint64_t y = *(const uint64_t *)b;

    if (x < y) return -1;
    if (x > y) return 1;
    return 0;
}

// latency_percentile():
//   - Need to sort samples first (use qsort)
//   - Calculate index: (size_t)(p * sample_count)
//   - Return samples[index]
//   - Handle edge cases (empty, p=1.0)

uint64_t latency_percentile (latency_tracker_t *tracker, double p) {
    if (tracker->count < 1) {
        return 0;
    }

    size_t num_samples = (tracker->count < tracker->sample_capacity ? tracker->count : tracker->sample_capacity);

    qsort(tracker->samples, num_samples, sizeof(uint64_t), cmp_u64);

    size_t index = (size_t)(p * num_samples);

    if (index >= num_samples) {
        index = num_samples - 1;
    }

    return tracker->samples[index];
}

// latency_print():
//   - Print name, count, min, max, mean, p50, p99
//   - Format nicely with units (ns, μs, ms)

void latency_print(latency_tracker_t *tracker, const char *name) {
    if (tracker->count == 0) {
        printf("No samples recorded.");
        return;
    }

    double mean = latency_mean(tracker);
    uint64_t p50 = latency_percentile(tracker, 0.50);
    uint64_t p99 = latency_percentile(tracker, 0.99);

    printf("%s:\n", name);
    printf("Count: %zu\n", tracker->count);
    printf("Min: %luns\n", tracker->min_ns);
    printf("Max: %luns\n", tracker->max_ns);
    printf("Mean: %.2fns\n", mean);
    printf("p50: %luns\n", p50);
    printf("p99: %luns\n", p99);

}

// latency_free():
//   - Free samples trackeray
//   - Set pointer to NULL
void latency_free(latency_tracker_t *tracker) {
    if (tracker){
        free(tracker->samples);
        tracker->samples = NULL;
    }
}