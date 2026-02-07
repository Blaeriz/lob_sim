#define _GNU_SOURCE
#include "agents/market_maker.h"
#include "core/book.h"
#include "core/order.h"
#include "core/price_tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct
{
  order_id_t next_order_id;
  unsigned int rng_seed;
  price_t half_spread;
  qty_t order_qty;
  qty_t inventory;
  qty_t max_inventory;
  order_id_t active_bid_id;
  order_id_t active_ask_id;
} market_maker_state_t;

static void mm_step(agent_t* agent, order_book_t* book, timestamp_t now)
{
  market_maker_state_t* state = agent->state;

  if (state->active_bid_id != 0)
  {
    book_remove_order(book, state->active_bid_id);
    state->active_bid_id = 0;
  }

  if (state->active_ask_id != 0)
  {
    book_remove_order(book, state->active_ask_id);
    state->active_ask_id = 0;
  }

  // MID PRICE

  price_level_t* bid_level = pt_max(&book->bids);
  price_level_t* ask_level = pt_min(&book->asks);

  price_t mid_price;

  if (bid_level && ask_level)
  {
    mid_price = (bid_level->price + ask_level->price) / 2;
  }
  else if (bid_level)
  {
    mid_price = bid_level->price;
  }
  else if (ask_level)
  {
    mid_price = ask_level->price;
  }
  else
  {
    mid_price = 1000; // DEF
  }

  // PRICES

  price_t bid_price = mid_price - state->half_spread;
  price_t ask_price = mid_price + state->half_spread;

  // INVENTORY SKEW

  qty_t skew = state->inventory / 10;
  bid_price -= skew;
  ask_price += skew;

  // POSITIVE PRICING

  if (bid_price < 1)
  {
    bid_price = 1;
  }
  if (ask_price < 1)
  {
    ask_price = 1;
  }

  // POST BID

  if (state->inventory < state->max_inventory)
  {
    order_t* bid = malloc(sizeof(order_t));
    if (bid)
    {
      bid->id = state->next_order_id++;
      bid->side = SIDE_BUY;
      bid->type = ORDER_LIMIT;
      bid->price = bid_price;
      bid->qty = state->order_qty;
      bid->ts = now;

      book_add_order(book, bid);
      state->active_bid_id = bid->id;
    }
  }

  // POST ASK
  if (state->inventory > -state->max_inventory)
  {
    order_t* ask = malloc(sizeof(order_t));
    if (ask)
    {
      ask->id = state->next_order_id++;
      ask->side = SIDE_SELL;
      ask->type = ORDER_LIMIT;
      ask->price = ask_price;
      ask->qty = state->order_qty;
      ask->ts = now;

      book_add_order(book, ask);
      state->active_ask_id = ask->id;
    }
  }
}

agent_t* market_maker_create(agent_id_t id)
{
  agent_t* a = malloc(sizeof(agent_t));
  if (!a)
    return NULL;
  market_maker_state_t* state = malloc(sizeof(market_maker_state_t));
  if (!state)
  {
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

void market_maker_destroy(agent_t* agent)
{
  if (!agent)
  {
    return;
  }

  if (agent->state)
  {
    free(agent->state);
  }

  free(agent);
}