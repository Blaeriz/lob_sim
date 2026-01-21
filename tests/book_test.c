#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "core/book.h"
#include "core/level_ops.h"

// Helper to create an order
static order_t *make_order(order_id_t id, side_t side, price_t price, qty_t qty) {
  order_t *o = (order_t *)malloc(sizeof(order_t));
  o->id = id;
  o->side = side;
  o->type = ORDER_LIMIT;
  o->price = price;
  o->qty = qty;
  o->ts = 0;
  return o;
}

// Test 1: Init and free empty book
static void test_init_free(void) {
  printf("test_init_free... ");

  order_book_t book;
  book_init(&book);

  assert(book.orders.buckets != NULL);
  assert(book.orders.count == 0);

  book_free(&book);

  printf("PASSED\n");
}

// Test 2: Add single order
static void test_add_single_order(void) {
  printf("test_add_single_order... ");

  order_book_t book;
  book_init(&book);

  order_t *buy = make_order(1, SIDE_BUY, 100, 10);
  book_add_order(&book, buy);

  // Order should be in map
  assert(book.orders.count == 1);
  om_entry_t *entry = om_find(&book.orders, 1);
  assert(entry != NULL);
  assert(entry->order == buy);

  // Order should be in bids tree
  price_level_t *lvl = pt_find(&book.bids, 100);
  assert(lvl != NULL);
  assert(level_peek(lvl) == buy);

  free(buy);
  book_free(&book);
  printf("PASSED\n");
}

// Test 3: Add orders to both sides
static void test_add_both_sides(void) {
  printf("test_add_both_sides... ");

  order_book_t book;
  book_init(&book);

  order_t *buy = make_order(1, SIDE_BUY, 100, 10);
  order_t *sell = make_order(2, SIDE_SELL, 105, 5);

  book_add_order(&book, buy);
  book_add_order(&book, sell);

  assert(book.orders.count == 2);

  // Buy in bids
  price_level_t *bid_lvl = pt_find(&book.bids, 100);
  assert(bid_lvl != NULL);
  assert(level_peek(bid_lvl) == buy);

  // Sell in asks
  price_level_t *ask_lvl = pt_find(&book.asks, 105);
  assert(ask_lvl != NULL);
  assert(level_peek(ask_lvl) == sell);

  free(buy);
  free(sell);
  book_free(&book);
  printf("PASSED\n");
}

// Test 4: Remove single order
static void test_remove_single_order(void) {
  printf("test_remove_single_order... ");

  order_book_t book;
  book_init(&book);

  order_t *buy = make_order(1, SIDE_BUY, 100, 10);
  book_add_order(&book, buy);

  assert(book.orders.count == 1);

  book_remove_order(&book, 1);

  // Order should be gone from map
  assert(book.orders.count == 0);
  assert(om_find(&book.orders, 1) == NULL);

  // Level should be removed (was empty)
  assert(pt_find(&book.bids, 100) == NULL);

  free(buy);
  book_free(&book);
  printf("PASSED\n");
}

// Test 5: Remove order - level not empty
static void test_remove_order_level_remains(void) {
  printf("test_remove_order_level_remains... ");

  order_book_t book;
  book_init(&book);

  order_t *buy1 = make_order(1, SIDE_BUY, 100, 10);
  order_t *buy2 = make_order(2, SIDE_BUY, 100, 20);

  book_add_order(&book, buy1);
  book_add_order(&book, buy2);

  assert(book.orders.count == 2);

  // Remove first order
  book_remove_order(&book, 1);

  assert(book.orders.count == 1);
  assert(om_find(&book.orders, 1) == NULL);
  assert(om_find(&book.orders, 2) != NULL);

  // Level should still exist with buy2
  price_level_t *lvl = pt_find(&book.bids, 100);
  assert(lvl != NULL);
  assert(level_peek(lvl) == buy2);
  assert(lvl->total_qty == 20);

  free(buy1);
  free(buy2);
  book_free(&book);
  printf("PASSED\n");
}

// Test 6: Remove middle order from level
static void test_remove_middle_order(void) {
  printf("test_remove_middle_order... ");

  order_book_t book;
  book_init(&book);

  order_t *o1 = make_order(1, SIDE_BUY, 100, 10);
  order_t *o2 = make_order(2, SIDE_BUY, 100, 20);
  order_t *o3 = make_order(3, SIDE_BUY, 100, 30);

  book_add_order(&book, o1);
  book_add_order(&book, o2);
  book_add_order(&book, o3);

  // Remove middle order
  book_remove_order(&book, 2);

  assert(book.orders.count == 2);

  price_level_t *lvl = pt_find(&book.bids, 100);
  assert(lvl != NULL);
  assert(lvl->total_qty == 40);  // 10 + 30

  // FIFO: o1 should still be first
  assert(level_peek(lvl) == o1);

  free(o1);
  free(o2);
  free(o3);
  book_free(&book);
  printf("PASSED\n");
}

// Test 7: Remove non-existent order
static void test_remove_nonexistent(void) {
  printf("test_remove_nonexistent... ");

  order_book_t book;
  book_init(&book);

  order_t *buy = make_order(1, SIDE_BUY, 100, 10);
  book_add_order(&book, buy);

  // Remove order that doesn't exist
  book_remove_order(&book, 999);

  // Original order should still be there
  assert(book.orders.count == 1);
  assert(om_find(&book.orders, 1) != NULL);

  free(buy);
  book_free(&book);
  printf("PASSED\n");
}

// Test 8: Remove from empty book
static void test_remove_from_empty(void) {
  printf("test_remove_from_empty... ");

  order_book_t book;
  book_init(&book);

  // Should not crash
  book_remove_order(&book, 1);

  assert(book.orders.count == 0);

  book_free(&book);
  printf("PASSED\n");
}

// Test 9: Remove sell order
static void test_remove_sell_order(void) {
  printf("test_remove_sell_order... ");

  order_book_t book;
  book_init(&book);

  order_t *sell = make_order(1, SIDE_SELL, 105, 10);
  book_add_order(&book, sell);

  book_remove_order(&book, 1);

  assert(book.orders.count == 0);
  assert(pt_find(&book.asks, 105) == NULL);

  free(sell);
  book_free(&book);
  printf("PASSED\n");
}

// Test 10: Multiple price levels
static void test_multiple_price_levels(void) {
  printf("test_multiple_price_levels... ");

  order_book_t book;
  book_init(&book);

  order_t *b1 = make_order(1, SIDE_BUY, 100, 10);
  order_t *b2 = make_order(2, SIDE_BUY, 101, 20);
  order_t *b3 = make_order(3, SIDE_BUY, 102, 30);

  book_add_order(&book, b1);
  book_add_order(&book, b2);
  book_add_order(&book, b3);

  // Remove middle price level
  book_remove_order(&book, 2);

  assert(book.orders.count == 2);
  assert(pt_find(&book.bids, 100) != NULL);
  assert(pt_find(&book.bids, 101) == NULL);  // removed
  assert(pt_find(&book.bids, 102) != NULL);

  free(b1);
  free(b2);
  free(b3);
  book_free(&book);
  printf("PASSED\n");
}

// Test 11: Remove all orders
static void test_remove_all_orders(void) {
  printf("test_remove_all_orders... ");

  order_book_t book;
  book_init(&book);

  order_t *orders[5];
  for (int i = 0; i < 5; i++) {
    orders[i] = make_order(i + 1, SIDE_BUY, 100 + i, 10);
    book_add_order(&book, orders[i]);
  }

  assert(book.orders.count == 5);

  // Remove all
  for (int i = 0; i < 5; i++) {
    book_remove_order(&book, i + 1);
  }

  assert(book.orders.count == 0);

  // All levels should be gone
  for (int i = 0; i < 5; i++) {
    assert(pt_find(&book.bids, 100 + i) == NULL);
  }

  for (int i = 0; i < 5; i++) {
    free(orders[i]);
  }
  book_free(&book);
  printf("PASSED\n");
}

// Test 12: NULL inputs
static void test_null_inputs(void) {
  printf("test_null_inputs... ");

  order_book_t book;
  book_init(&book);

  // Should not crash
  book_add_order(NULL, NULL);
  book_add_order(&book, NULL);
  book_remove_order(NULL, 1);

  book_free(&book);
  printf("PASSED\n");
}

int main(void) {
  printf("\n=== Running book tests ===\n\n");

  test_init_free();
  test_add_single_order();
  test_add_both_sides();
  test_remove_single_order();
  test_remove_order_level_remains();
  test_remove_middle_order();
  test_remove_nonexistent();
  test_remove_from_empty();
  test_remove_sell_order();
  test_multiple_price_levels();
  test_remove_all_orders();
  test_null_inputs();

  printf("\n=== All tests PASSED ===\n\n");
  return 0;
}
