#include <stddef.h>
#include <stdlib.h>

#include "core/book.h"
#include "core/level_ops.h"

void book_init(order_book_t *book) {
  pt_init(&book->bids);
  pt_init(&book->asks);
}

static void free_level_payload(price_level_t *lvl) {
  if (!lvl) return;

  // If the book owns orders, replace NULL with `free` (or your order_free function).
  level_free_queue(lvl, NULL);

  free(lvl);
}

void book_free(order_book_t *book) {
  if (!book) return;

  pt_clear(&book->bids, free_level_payload);
  pt_clear(&book->asks, free_level_payload);
}

void book_add_order(order_book_t *book, order_t *order) {
  if (!book || !order) return;

  price_tree_t *tree = (order->side == SIDE_BUY) ? &book->bids : &book->asks;

  price_level_t *lvl = pt_find(tree, order->price);
  if (!lvl) {
    lvl = (price_level_t *)malloc(sizeof *lvl);
    if (!lvl) return; // or handle error upstream

    level_init(lvl, order->price);

    int rc = pt_insert(tree, order->price, lvl);
    if (rc != 1) {
      // rc==0 duplicate shouldn't happen because pt_find failed, but handle anyway
      // rc==-1 alloc failure inside tree
      free(lvl);
      return;
    }
  }

  level_push(lvl, order);
}

void book_remove_order(order_book_t *book, order_id_t id) {
  (void)book;
  (void)id;
}
