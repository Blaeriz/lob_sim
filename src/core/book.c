#include <stddef.h>

#include "../../include/core/book.h"

void book_init(order_book_t *book) {
  book->bids = NULL;
  book->asks = NULL;
  book->best_bid = 0;
  book->best_ask = 0;
}

void book_free(order_book_t *book) { (void)book; }
