#ifndef MATCHING_H
#define MATCHING_H

#include "book.h"
#include "trade.h"
#include <stddef.h>

qty_t match_order(order_book_t* book, order_t* incoming, trade_t* trades, size_t max_trades);

#endif
