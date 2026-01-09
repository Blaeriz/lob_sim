#ifndef ORDER_H
#define ORDER_H

#include "common/types.h"

typedef enum { SIDE_BUY = 1, SIDE_SELL = -1 } side_t;

typedef enum { ORDER_LIMIT, ORDER_MARKET } order_type_t;

typedef struct {
  order_id_t id;
  side_t side;
  order_type_t type;
  price_t price;
  qty_t qty;
  timestamp_t ts;
} order_t;

#endif
