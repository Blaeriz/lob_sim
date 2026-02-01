#define _GNU_SOURCE
#include "agents/noise_trader.h"
#include "core/book.h"
#include "core/price_tree.h"
#include "core/order.h"
#include <unistd.h>
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
  noise_trader_state_t *state = (noise_trader_state_t *) agent->state;

  double roll = (double) rand_r(&state->rng_seed) / (double) RAND_MAX;
  if (roll >= state->act_probability){
    return;
  }

  side_t side = (rand_r(&state->rng_seed) % 2 == 0) ? SIDE_BUY : SIDE_SELL;

  // MID PRICE
  price_level_t *bid_level = pt_max(&book->bids);
  price_level_t *ask_level = pt_min(&book->asks);

  price_t mid_price = 0;

  if (bid_level && ask_level) {
    mid_price = (bid_level->price + ask_level->price) / 2;
  } else if (bid_level) {
    mid_price = bid_level->price;
  } else if (ask_level) {
    mid_price = ask_level->price;
  } else {
    mid_price = 1000;
  }

  // PRICE WITH OFFSET
  unsigned int offset = rand_r(&state->rng_seed) % (state->price_range + 1); 

  price_t price = 0;

  if (side == SIDE_BUY) {
    price = mid_price - offset;
  } else {
    price = mid_price + offset;
  }

  if (price < 1) {
    price = 1;
  }

  qty_t qty_range = state->max_qty - state->min_qty + 1;
  qty_t qty = state->min_qty + ((rand_r(&state->rng_seed)) % qty_range);

  // BUILD ORDER
  order_t order;
  order.id = state->next_order_id;
  order.side = side;
  order.type = ORDER_LIMIT;
  order.price = price;
  order.qty = qty;
  order.ts = now;

  // SUBMIT
  book_add_order(book, &order);

  // INCREMENT ID
  state->next_order_id++;

  printf("NoiseTrader %lu: %s %ld @ %ld\n", agent->id,
       side == SIDE_BUY ? "BUY" : "SELL", qty, price);
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

void noise_trader_destroy(agent_t *agent) {
  if (!agent) {
    return;
  }

  if (agent->state) {
    free(agent->state);
  }

  free(agent);
}
