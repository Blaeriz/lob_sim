#include <stddef.h>

#include "core/book.h"

void book_init(order_book_t *book) {
  book->bids = NULL;
  book->asks = NULL;
  book->best_bid = 0;
  book->best_ask = 0;
}

void book_free(order_book_t *book) { (void)book; }

/* Minimal stubs for book operations so the project links while full
   implementation is developed. These manipulate nothing for now. */
void book_add_order(order_book_t *book, order_t *order) {
  (void)book;
  (void)order;
}

void book_remove_order(order_book_t *book, order_id_t id) {
  (void)book;
  (void)id;
}
