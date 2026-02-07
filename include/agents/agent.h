#ifndef AGENT_H
#define AGENT_H

#include "common/types.h"
#include "core/book.h"
#include "core/order.h"

typedef struct agent agent_t;

typedef void (*agent_step_fn)(agent_t* agent, order_book_t* book, timestamp_t now);

struct agent
{
  agent_id_t id;
  agent_step_fn step;
  void* state;
};

#endif // !AGENT_H
