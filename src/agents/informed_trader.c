#define _GNU_SOURCE
#include "agents/informed_trader.h"
#include "core/book.h"
#include "core/order.h"
#include "core/price_tree.h"
#include <stdlib.h>
#include <time.h>

typedef struct
{
  order_id_t next_order_id;
  unsigned int rng_seed;

  price_t fair_value;
  price_t threshold;
  qty_t order_qty;

  price_t drift_rate;
  timestamp_t last_update_time;
} informed_trader_state_t;

static void informed_step(agent_t* agent, order_book_t* book, timestamp_t now)
{
  informed_trader_state_t* state = agent->state;

  // DRIFT VALUE TO SIMULATE CHANGING INFO

  price_t drift = (rand_r(&state->rng_seed) % 3) - 1;
  state->fair_value += drift * state->drift_rate;

  // FAIR VALUE STAYS POSITIVE

  if (state->fair_value < 100)
  {
    state->fair_value = 100;
  }

  // TRADE PROBABILITY
  if ((rand_r(&state->rng_seed) % 100) >= 1)
  {
    return; // Skip this tick 95% of the time
  }

  // BEST BID AND ASK

  price_level_t* bid_level = pt_max(&book->bids);
  price_level_t* ask_level = pt_min(&book->asks);

  if (!bid_level && !ask_level)
  {
    return;
  }

  price_t best_bid = bid_level ? bid_level->price : 0;
  price_t best_ask = ask_level ? ask_level->price : 0;

  // CHECK BUY OPPORTUNITY

  if (best_ask > 0 && best_ask < (state->fair_value - state->threshold))
  {
    order_t* order = malloc(sizeof(order_t));
    order->id = state->next_order_id++;
    order->side = SIDE_BUY;
    order->type = ORDER_LIMIT;
    order->price = best_ask;
    order->qty = state->order_qty;
    order->ts = now;
    book_add_order(book, order);
    return;
  }

  // CHECK SELL OPPORTuNITY

  if (best_bid > 0 && best_bid > (state->fair_value + state->threshold))
  {
    order_t* order = malloc(sizeof(order_t));
    order->id = state->next_order_id++;
    order->side = SIDE_SELL;
    order->type = ORDER_LIMIT;
    order->price = best_bid;
    order->qty = state->order_qty;
    order->ts = now;
    book_add_order(book, order);
  }
}

agent_t* informed_trader_create(agent_id_t id)
{
  agent_t* a = malloc(sizeof(agent_t));
  if (!a)
    return NULL;

  informed_trader_state_t* state = malloc(sizeof(informed_trader_state_t));
  if (!state)
  {
    free(a);
    return NULL;
  }

  state->next_order_id = 1;
  state->rng_seed = (unsigned int)time(NULL) + id;
  state->fair_value = 1000;
  state->threshold = 10;
  state->order_qty = 10;
  state->drift_rate = 1;
  state->last_update_time = 0;

  a->id = id;
  a->step = informed_step;
  a->state = state;
  return a;
}

void informed_trader_destroy(agent_t* agent)
{
  if (!agent)
    return;
  if (agent->state)
    free(agent->state);
  free(agent);
}
