#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "../agents/agent.h"
#include "../core/book.h"
#include "../sim/event.h"

void simulator_init(order_book_t *book);
void simulator_add_agent(agent_t *agent);
void simulator_run(timestamp_t end_time);

#endif
