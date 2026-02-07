#include "sim/stats.h"

/* Very small stats implementation to collect basic metrics. */
static price_t last_price = 0;
static qty_t total_volume = 0;
static size_t trade_count = 0;

void stats_init(void)
{
  last_price = 0;
  total_volume = 0;
  trade_count = 0;
}

void stats_on_trade(price_t price, qty_t qty)
{
  last_price = price;
  total_volume += qty;
  trade_count++;
}

market_stats_t stats_snapshot(void)
{
  market_stats_t s = {0};
  s.mid_price = (double)last_price;
  s.spread = 0.0;
  s.volatility = 0.0;
  s.volume = total_volume;
  s.trade_count = trade_count;
  return s;
}
