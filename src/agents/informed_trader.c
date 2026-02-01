#define _GNU_SOURCE
#include "agents/informed_trader.h"
#include "core/book.h"
#include "core/price_tree.h"
#include "core/order.h"
#include <stdlib.h>
#include <time.h>

typedef struct {
  order_id_t next_order_id;
  unsigned int rng_seed;

  price_t fair_value;
  price_t threshold;
  qty_t order_qty;

  price_t drift_rate;
  timestamp_t last_update_time;
} informed_trader_state_t;

static void informed_step(agent_t *agent, order_book_t *book, timestamp_t now) {
  (void)agent;
  (void)book;
  (void)now;
}

agent_t *informed_trader_create(agent_id_t id) {
  agent_t *a = malloc(sizeof(agent_t));
  if (!a) return NULL;

  informed_trader_state_t *state = malloc(sizeof(informed_trader_state_t));
  if (!state) { free(a); return NULL; }

  state->next_order_id = 1;
  state->rng_seed = (unsigned int)time(NULL) + id;
  state->fair_value = 1000;
  state->threshold = 5;
  state->order_qty = 20;
  state->drift_rate = 1;
  state->last_update_time = 0;

  a->id = id;
  a->step = informed_step;
  a->state = state;
  return a;
}

void informed_trader_destroy(agent_t *agent) {
  if (!agent) return;
  if (agent->state) free(agent->state);
  free(agent);
}
