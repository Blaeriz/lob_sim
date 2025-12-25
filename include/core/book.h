#ifndef BOOK_H
#define BOOK_H

#include "../common/types.h"
#include "level.h"

typedef struct {
  price_level_t *bids; /* descending prices */
  price_level_t *asks; /* ascending prices */
  price_t best_bid;
  price_t best_ask;
} order_book_t;

/* lifecycle */
void book_init(order_book_t *book);
void book_free(order_book_t *book);

/* state updates */
void book_add_order(order_book_t *book, order_t *order);
void book_remove_order(order_book_t *book, order_id_t id);

#endif
