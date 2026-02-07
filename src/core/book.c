#include <stddef.h>
#include <stdlib.h>

#include "core/book.h"
#include "core/level_ops.h"
#include "core/matching.h"
#include "core/trade.h"

#ifdef BENCHMARK
#include "bench/latency.h"
extern latency_tracker_t add_order_tracker;
extern latency_tracker_t remove_order_tracker;
#endif

void book_init(order_book_t* book)
{
  pt_init(&book->bids);
  pt_init(&book->asks);
  om_init(&book->orders, 65536);
}

static void free_level_payload(price_level_t* lvl)
{
  if (!lvl)
    return;

  // If the book owns orders, replace NULL with `free` (or your order_free function).
  level_free_queue(lvl, NULL);

  free(lvl);
}

void book_free(order_book_t* book)
{
  if (!book)
    return;

  pt_clear(&book->bids, free_level_payload);
  pt_clear(&book->asks, free_level_payload);
  om_free(&book->orders);
}

void book_add_order(order_book_t* book, order_t* order)
{
#ifdef BENCHMARK
  uint64_t start = time_now_ns();
#endif
  if (!book || !order)
    return;

  // MATCH FIRST
  trade_t trades[100];
  match_order(book, order, trades, 100);

  // IF FILLED, FREE ORDER
  if (order->qty == 0)
  {
    free(order);
    return;
  }

  price_tree_t* tree = (order->side == SIDE_BUY) ? &book->bids : &book->asks;

  price_level_t* lvl = pt_find(tree, order->price);
  if (!lvl)
  {
    lvl = (price_level_t*)malloc(sizeof *lvl);
    if (!lvl)
      return; // or handle error upstream

    level_init(lvl, order->price);

    int rc = pt_insert(tree, order->price, lvl);
    if (rc != 1)
    {
      // rc==0 duplicate shouldn't happen because pt_find failed, but handle anyway
      // rc==-1 alloc failure inside tree
      free(lvl);
      return;
    }
  }

  order_node_t* node = level_push(lvl, order);
  om_insert(&book->orders, order->id, order, order->side, order->price, node);
#ifdef BENCHMARK
  latency_record(&add_order_tracker, time_now_ns() - start);
#endif
}

void book_remove_order(order_book_t* book, order_id_t id)
{
#ifdef BENCHMARK
  uint64_t start = time_now_ns();
#endif
  if (!book)
  {
    return;
  }

  // 1. Find the order in the map
  om_entry_t* entry = om_find(&book->orders, id);
  if (!entry)
  {
    return;
  }

  // 2. Get the tree (bids or asks based on side)
  price_tree_t* tree = (entry->side == SIDE_BUY) ? &book->bids : &book->asks;

  // 3. Find the level at that price
  price_level_t* lvl = pt_find(tree, entry->price);

  // 4. Remove the order from the level's queue
  level_remove(lvl, entry->node);

  // 5. If level is empty, remove from tree and free level
  if (level_is_empty(lvl))
  {
    pt_remove(tree, entry->price);
    free(lvl);
  }

  // 6. Remove from order map
  om_remove(&book->orders, id);
#ifdef BENCHMARK
  latency_record(&remove_order_tracker, time_now_ns() - start);
#endif
}
