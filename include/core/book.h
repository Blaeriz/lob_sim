#ifndef BOOK_H
#define BOOK_H

#include "common/types.h"
#include "core/level.h"
#include "core/price_tree.h"
#include "core/order_map.h"

typedef struct {
  price_tree_t bids; /* descending prices */
  price_tree_t asks; /* ascending prices */
  order_map_t orders;
} order_book_t;

/* lifecycle */
void book_init(order_book_t *book);
void book_free(order_book_t *book);

/* state updates */
void book_add_order(order_book_t *book, order_t *order);
void book_remove_order(order_book_t *book, order_id_t id);

#endif
