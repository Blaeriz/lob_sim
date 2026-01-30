#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "agents/agent.h"
#include "core/book.h"
#include "sim/event.h"

typedef struct simulator_t {
    order_book_t *book;
    //event_t *event_queue;
    agent_t **agents;
    size_t agent_count;
    size_t agent_capacity;
    timestamp_t current_time;
    timestamp_t dt;
} simulator_t;

void simulator_init(order_book_t *book);
void simulator_add_agent(agent_t *agent);
void simulator_run(timestamp_t end_time);
void simulator_free(void);

#endif
