#ifndef TRADE_H
#define TRADE_H

#include "common/types.h"

typedef struct {
  trade_id_t id;
  order_id_t buy_id;
  order_id_t sell_id;
  price_t price;
  qty_t qty;
  timestamp_t ts;
} trade_t;

#endif
