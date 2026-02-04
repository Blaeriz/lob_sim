#include "core/matching.h"
#include "core/level_ops.h"
#include "sim/stats.h"
#include <stdlib.h>


qty_t match_order(order_book_t *book, order_t *incoming, trade_t *trades, size_t max_trades) {
  size_t trade_count = 0;
  price_tree_t *tree;
  side_t side;

  // 1. Validate inputs (return 0 if book/incoming/trades is NULL or max_trades is 0)

  if (!book || !incoming || !trades || max_trades == 0) {
    return 0;
  }

  side = incoming->side;

  // 2. Pick the opposite tree based on incoming->side

  if (side == SIDE_BUY) { // BUY ORDER
    tree = &book->asks;
  } else { // SELL ORDER
    tree = &book->bids;
  }

  // 3. Main loop: 

  price_level_t *best;

  while (incoming->qty > 0 && trade_count < max_trades) {

    // a. Get best price level

    if (side == SIDE_BUY) {
      best = pt_min(tree);
    } else {
      best = pt_max(tree);
    }
    
    // b. Check crossing condition — break if:
    
    if (best == NULL || (side == SIDE_BUY && incoming->price < best->price) || (side == SIDE_SELL && incoming->price > best->price)) {
      break;
    }

    // c. Inner loop: match against orders at this price level
    while (incoming->qty > 0 && trade_count < max_trades && level_peek(best) != NULL) {
      
      // 1. Peek at front order (don't remove yet)

      order_t *resting = level_peek(best);

      // 2. Calculate fill qty: min(incoming->qty, resting->qty)

      qty_t fill = (incoming->qty > resting->qty) ? resting->qty : incoming->qty;

      // 3. Fill in trades[trade_count]:

      trades[trade_count].id = trade_count;
      trades[trade_count].price = resting->price;
      trades[trade_count].qty = fill;
      trades[trade_count].ts = incoming->ts;
      stats_on_trade(resting->price, fill);
      
      // Now figure out buy_id and sell_id:
      if (side == SIDE_BUY) {
        trades[trade_count].buy_id = incoming->id;
        trades[trade_count].sell_id = resting->id;
      } else {
        trades[trade_count].buy_id = resting->id;
        trades[trade_count].sell_id = incoming->id;
      }

      best->total_qty -= fill;

      // 4. Decrement quantities:
      incoming->qty -= fill;
      resting->qty -= fill;
      
      // 5. If resting order is fully filled, remove it from the level
      if (resting->qty == 0) {
        level_pop(best);
      }

      trade_count++;
    }

    // d. Clean up empty level
    if (level_is_empty(best)) {
      // Remove from tree and free the level
      pt_remove(tree, best->price);
      free(best);
    }
  }
  
  // 4. Return trade_count
  return trade_count;
}
