#include "agents/noise_trader.h"
#include "core/book.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct {
  order_id_t next_order_id;
  double act_probability;
  price_t price_range;
  qty_t min_qty;
  qty_t max_qty;
  unsigned int rng_seed;
} noise_trader_state_t;

static void noise_step(agent_t *agent, order_book_t *book, timestamp_t now) {
  (void)agent;
  (void)now;
}

agent_t *noise_trader_create(agent_id_t id) {
  agent_t *a = malloc(sizeof(agent_t));
  if (!a) {
    fprintf(stderr, "could not allocate memory for agent.\n");
    return NULL;
  }
  a->id = id;
  a->step = noise_step;
  noise_trader_state_t *agent_state = malloc(sizeof(noise_trader_state_t));
  if(!agent_state) {
    fprintf(stderr, "could not allocate memory for agent state.\n");
    free(a);
    return NULL;
  }
  agent_state->next_order_id = 1;
  agent_state->act_probability = 0.1;
  agent_state->price_range = 10;
  agent_state->min_qty = 1;
  agent_state->max_qty = 10;
  agent_state->rng_seed = id;
  a->state = agent_state;
  return a;
}
