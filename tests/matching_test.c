#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "core/book.h"
#include "core/level_ops.h"
#include "core/matching.h"

// Helper to create an order
static order_t* make_order(order_id_t id, side_t side, price_t price, qty_t qty)
{
  order_t* o = (order_t*)malloc(sizeof(order_t));
  o->id = id;
  o->side = side;
  o->type = ORDER_LIMIT;
  o->price = price;
  o->qty = qty;
  o->ts = 0;
  return o;
}

// Test 1: No match — empty book
static void test_no_match_empty_book(void)
{
  printf("test_no_match_empty_book... ");

  order_book_t book;
  book_init(&book);

  order_t* buy = make_order(1, SIDE_BUY, 100, 10);
  trade_t trades[10];

  qty_t count = match_order(&book, buy, trades, 10);

  assert(count == 0);
  assert(buy->qty == 10); // unchanged

  free(buy);
  book_free(&book);
  printf("PASSED\n");
}

// Test 2: No match — prices don't cross
static void test_no_match_no_cross(void)
{
  printf("test_no_match_no_cross... ");

  order_book_t book;
  book_init(&book);

  // Resting ask at 105
  order_t* ask = make_order(1, SIDE_SELL, 105, 10);
  book_add_order(&book, ask);

  // Incoming buy at 100 — won't cross (100 < 105)
  order_t* buy = make_order(2, SIDE_BUY, 100, 10);
  trade_t trades[10];

  qty_t count = match_order(&book, buy, trades, 10);

  assert(count == 0);
  assert(buy->qty == 10);
  assert(ask->qty == 10);

  free(buy);
  free(ask);
  book_free(&book);
  printf("PASSED\n");
}

// Test 3: Full match — single order
static void test_full_match_single(void)
{
  printf("test_full_match_single... ");

  order_book_t book;
  book_init(&book);

  // Resting ask at 100
  order_t* ask = make_order(1, SIDE_SELL, 100, 10);
  book_add_order(&book, ask);

  // Incoming buy at 100 — exact match
  order_t* buy = make_order(2, SIDE_BUY, 100, 10);
  trade_t trades[10];

  qty_t count = match_order(&book, buy, trades, 10);

  assert(count == 1);
  assert(buy->qty == 0);
  assert(ask->qty == 0);
  assert(trades[0].qty == 10);
  assert(trades[0].price == 100);
  assert(trades[0].buy_id == 2);
  assert(trades[0].sell_id == 1);

  free(buy);
  free(ask);
  book_free(&book);
  printf("PASSED\n");
}

// Test 4: Partial match — incoming larger
static void test_partial_match_incoming_larger(void)
{
  printf("test_partial_match_incoming_larger... ");

  order_book_t book;
  book_init(&book);

  // Resting ask at 100 for qty 5
  order_t* ask = make_order(1, SIDE_SELL, 100, 5);
  book_add_order(&book, ask);

  // Incoming buy at 100 for qty 10
  order_t* buy = make_order(2, SIDE_BUY, 100, 10);
  trade_t trades[10];

  qty_t count = match_order(&book, buy, trades, 10);

  assert(count == 1);
  assert(buy->qty == 5); // 10 - 5 remaining
  assert(ask->qty == 0); // fully filled
  assert(trades[0].qty == 5);

  free(buy);
  free(ask);
  book_free(&book);
  printf("PASSED\n");
}

// Test 5: Partial match — resting larger
static void test_partial_match_resting_larger(void)
{
  printf("test_partial_match_resting_larger... ");

  order_book_t book;
  book_init(&book);

  // Resting ask at 100 for qty 20
  order_t* ask = make_order(1, SIDE_SELL, 100, 20);
  book_add_order(&book, ask);

  // Incoming buy at 100 for qty 10
  order_t* buy = make_order(2, SIDE_BUY, 100, 10);
  trade_t trades[10];

  qty_t count = match_order(&book, buy, trades, 10);

  assert(count == 1);
  assert(buy->qty == 0);  // fully filled
  assert(ask->qty == 10); // 20 - 10 remaining
  assert(trades[0].qty == 10);

  free(buy);
  free(ask);
  book_free(&book);
  printf("PASSED\n");
}

// Test 6: Multiple trades — FIFO at same price
static void test_multiple_trades_fifo(void)
{
  printf("test_multiple_trades_fifo... ");

  order_book_t book;
  book_init(&book);

  // Three resting asks at 100 (FIFO order: ask1, ask2, ask3)
  order_t* ask1 = make_order(1, SIDE_SELL, 100, 5);
  order_t* ask2 = make_order(2, SIDE_SELL, 100, 5);
  order_t* ask3 = make_order(3, SIDE_SELL, 100, 5);
  book_add_order(&book, ask1);
  book_add_order(&book, ask2);
  book_add_order(&book, ask3);

  // Incoming buy at 100 for qty 12
  order_t* buy = make_order(4, SIDE_BUY, 100, 12);
  trade_t trades[10];

  qty_t count = match_order(&book, buy, trades, 10);

  assert(count == 3);
  assert(buy->qty == 0);  // fully filled (12)
  assert(ask1->qty == 0); // filled 5
  assert(ask2->qty == 0); // filled 5
  assert(ask3->qty == 3); // filled 2, remaining 3

  // Check FIFO order
  assert(trades[0].sell_id == 1);
  assert(trades[1].sell_id == 2);
  assert(trades[2].sell_id == 3);

  assert(trades[0].qty == 5);
  assert(trades[1].qty == 5);
  assert(trades[2].qty == 2);

  free(buy);
  free(ask1);
  free(ask2);
  free(ask3);
  book_free(&book);
  printf("PASSED\n");
}

// Test 7: Multiple price levels — price priority
static void test_price_priority(void)
{
  printf("test_price_priority... ");

  order_book_t book;
  book_init(&book);

  // Resting asks at different prices (best ask = 98)
  order_t* ask1 = make_order(1, SIDE_SELL, 100, 5);
  order_t* ask2 = make_order(2, SIDE_SELL, 98, 5); // best ask
  order_t* ask3 = make_order(3, SIDE_SELL, 102, 5);
  book_add_order(&book, ask1);
  book_add_order(&book, ask2);
  book_add_order(&book, ask3);

  // Incoming buy at 100 for qty 8
  order_t* buy = make_order(4, SIDE_BUY, 100, 8);
  trade_t trades[10];

  qty_t count = match_order(&book, buy, trades, 10);

  assert(count == 2);
  assert(buy->qty == 0);

  // Should match 98 first (best), then 100
  assert(trades[0].price == 98);
  assert(trades[0].sell_id == 2);
  assert(trades[0].qty == 5);

  assert(trades[1].price == 100);
  assert(trades[1].sell_id == 1);
  assert(trades[1].qty == 3);

  // ask3 at 102 should NOT be touched (102 > 100)
  assert(ask3->qty == 5);

  free(buy);
  free(ask1);
  free(ask2);
  free(ask3);
  book_free(&book);
  printf("PASSED\n");
}

// Test 8: Sell order matching
static void test_sell_order_matching(void)
{
  printf("test_sell_order_matching... ");

  order_book_t book;
  book_init(&book);

  // Resting bids at different prices (best bid = 102)
  order_t* bid1 = make_order(1, SIDE_BUY, 100, 5);
  order_t* bid2 = make_order(2, SIDE_BUY, 102, 5); // best bid
  book_add_order(&book, bid1);
  book_add_order(&book, bid2);

  // Incoming sell at 100 for qty 8
  order_t* sell = make_order(3, SIDE_SELL, 100, 8);
  trade_t trades[10];

  qty_t count = match_order(&book, sell, trades, 10);

  assert(count == 2);
  assert(sell->qty == 0);

  // Should match 102 first (best bid), then 100
  assert(trades[0].price == 102);
  assert(trades[0].buy_id == 2);
  assert(trades[1].price == 100);
  assert(trades[1].buy_id == 1);

  free(sell);
  free(bid1);
  free(bid2);
  book_free(&book);
  printf("PASSED\n");
}

// Test 9: max_trades limit
static void test_max_trades_limit(void)
{
  printf("test_max_trades_limit... ");

  order_book_t book;
  book_init(&book);

  // 5 resting asks
  order_t* asks[5];
  for (int i = 0; i < 5; i++)
  {
    asks[i] = make_order(i + 1, SIDE_SELL, 100, 2);
    book_add_order(&book, asks[i]);
  }

  // Incoming buy that could match all 5
  order_t* buy = make_order(10, SIDE_BUY, 100, 10);
  trade_t trades[3]; // only allow 3 trades

  qty_t count = match_order(&book, buy, trades, 3);

  assert(count == 3);    // capped at max_trades
  assert(buy->qty == 4); // 10 - (3 * 2) = 4 remaining

  for (int i = 0; i < 5; i++)
  {
    free(asks[i]);
  }
  free(buy);
  book_free(&book);
  printf("PASSED\n");
}

// Test 10: NULL inputs
static void test_null_inputs(void)
{
  printf("test_null_inputs... ");

  order_book_t book;
  book_init(&book);

  order_t* buy = make_order(1, SIDE_BUY, 100, 10);
  trade_t trades[10];

  assert(match_order(NULL, buy, trades, 10) == 0);
  assert(match_order(&book, NULL, trades, 10) == 0);
  assert(match_order(&book, buy, NULL, 10) == 0);
  assert(match_order(&book, buy, trades, 0) == 0);

  free(buy);
  book_free(&book);
  printf("PASSED\n");
}

int main(void)
{
  printf("\n=== Running match_order tests ===\n\n");

  test_no_match_empty_book();
  test_no_match_no_cross();
  test_full_match_single();
  test_partial_match_incoming_larger();
  test_partial_match_resting_larger();
  test_multiple_trades_fifo();
  test_price_priority();
  test_sell_order_matching();
  test_max_trades_limit();
  test_null_inputs();

  printf("\n=== All tests PASSED ===\n\n");
  return 0;
}