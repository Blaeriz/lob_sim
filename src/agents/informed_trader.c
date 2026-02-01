#include "agents/informed_trader.h"
#include <stdlib.h>

static void informed_step(agent_t *agent, order_book_t *book, timestamp_t now) {
  (void)agent;
  (void)book;
  (void)now;
}

agent_t *informed_trader_create(agent_id_t id) {
  agent_t *a = malloc(sizeof(agent_t));
  if (!a) return NULL;
  a->id = id;
  a->step = informed_step;
  a->state = NULL;
  return a;
}
