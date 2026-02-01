#define _GNU_SOURCE
#include "agents/market_maker.h"
#include "core/book.h"
#include "core/price_tree.h"
#include "core/order.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
  order_id_t next_order_id;
  unsigned int rng_seed;
  price_t half_spread;
  qty_t order_qty;
  qty_t inventory;
  qty_t max_inventory;
  order_id_t active_bid_id;
  order_id_t active_ask_id;
} market_maker_state_t;


static void mm_step(agent_t *agent, order_book_t *book, timestamp_t now) {
  (void)agent;
  (void)now;
}

agent_t *market_maker_create(agent_id_t id) {
  agent_t *a = malloc(sizeof(agent_t));
  if (!a) return NULL;
  market_maker_state_t *state = malloc(sizeof(market_maker_state_t));
  if (!state) {
    free(a);
    return NULL;
  }

  // STATE
  state->next_order_id = 1;
  state->rng_seed = time(NULL) + id;
  state->half_spread = 5;
  state->order_qty = 10;
  state->inventory = 0;
  state->max_inventory = 100;
  state->active_ask_id = 0;
  state->active_bid_id = 0;

  // AGENT

  a->id = id;
  a->step = mm_step;
  a->state = state;

  return a;
}

void market_maker_destroy(agent_t *agent) {
  if (!agent) {
    return;
  }

  if (agent->state) {
    free(agent->state);
  }

  free(agent);
}