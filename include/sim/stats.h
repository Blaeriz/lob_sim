#ifndef STATS_H
#define STATS_H

#include "common/types.h"
#include <stddef.h>

typedef struct
{
  double mid_price;
  double spread;
  double volatility;
  qty_t volume;
  size_t trade_count;
} market_stats_t;

void stats_init(void);
void stats_on_trade(price_t price, qty_t qty);
market_stats_t stats_snapshot(void);

#endif
