#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "core/order_map.h"

// Helper to create a dummy order
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

// Test 1: Init and free empty map
static void test_init_free(void) {
  printf("test_init_free... ");

  order_map_t map;
  om_init(&map, 16);

  assert(map.buckets != NULL);
  assert(map.num_buckets == 16);
  assert(map.count == 0);

  om_free(&map);

  assert(map.buckets == NULL);
  assert(map.num_buckets == 0);
  assert(map.count == 0);

  printf("PASSED\n");
}

// Test 2: Insert and find single entry
static void test_insert_find_single(void) {
  printf("test_insert_find_single... ");

  order_map_t map;
  om_init(&map, 16);

  order_t *order = make_order(42, SIDE_BUY, 100, 10);
  int rc = om_insert(&map, 42, order, SIDE_BUY, 100);

  assert(rc == 0);
  assert(map.count == 1);

  om_entry_t *entry = om_find(&map, 42);
  assert(entry != NULL);
  assert(entry->key == 42);
  assert(entry->order == order);
  assert(entry->side == SIDE_BUY);
  assert(entry->price == 100);

  free(order);
  om_free(&map);
  printf("PASSED\n");
}

// Test 3: Find non-existent entry
static void test_find_not_found(void) {
  printf("test_find_not_found... ");

  order_map_t map;
  om_init(&map, 16);

  order_t *order = make_order(1, SIDE_BUY, 100, 10);
  om_insert(&map, 1, order, SIDE_BUY, 100);

  om_entry_t *entry = om_find(&map, 999);
  assert(entry == NULL);

  free(order);
  om_free(&map);
  printf("PASSED\n");
}

// Test 4: Insert multiple entries
static void test_insert_multiple(void) {
  printf("test_insert_multiple... ");

  order_map_t map;
  om_init(&map, 16);

  order_t *orders[5];
  for (int i = 0; i < 5; i++) {
    orders[i] = make_order(i + 1, SIDE_BUY, 100 + i, 10);
    om_insert(&map, i + 1, orders[i], SIDE_BUY, 100 + i);
  }

  assert(map.count == 5);

  // Verify all can be found
  for (int i = 0; i < 5; i++) {
    om_entry_t *entry = om_find(&map, i + 1);
    assert(entry != NULL);
    assert(entry->order == orders[i]);
    assert(entry->price == 100 + i);
  }

  for (int i = 0; i < 5; i++) {
    free(orders[i]);
  }
  om_free(&map);
  printf("PASSED\n");
}

// Test 5: Collision handling (same bucket)
static void test_collision(void) {
  printf("test_collision... ");

  order_map_t map;
  om_init(&map, 8);  // small bucket count to force collisions

  // IDs 1, 9, 17 all hash to bucket 1 (mod 8)
  order_t *o1 = make_order(1, SIDE_BUY, 100, 10);
  order_t *o9 = make_order(9, SIDE_SELL, 200, 20);
  order_t *o17 = make_order(17, SIDE_BUY, 300, 30);

  om_insert(&map, 1, o1, SIDE_BUY, 100);
  om_insert(&map, 9, o9, SIDE_SELL, 200);
  om_insert(&map, 17, o17, SIDE_BUY, 300);

  assert(map.count == 3);

  // All three should be findable
  om_entry_t *e1 = om_find(&map, 1);
  om_entry_t *e9 = om_find(&map, 9);
  om_entry_t *e17 = om_find(&map, 17);

  assert(e1 != NULL && e1->order == o1);
  assert(e9 != NULL && e9->order == o9);
  assert(e17 != NULL && e17->order == o17);

  free(o1);
  free(o9);
  free(o17);
  om_free(&map);
  printf("PASSED\n");
}

// Test 6: Remove single entry
static void test_remove_single(void) {
  printf("test_remove_single... ");

  order_map_t map;
  om_init(&map, 16);

  order_t *order = make_order(42, SIDE_BUY, 100, 10);
  om_insert(&map, 42, order, SIDE_BUY, 100);

  assert(map.count == 1);

  int rc = om_remove(&map, 42);
  assert(rc == 0);
  assert(map.count == 0);

  om_entry_t *entry = om_find(&map, 42);
  assert(entry == NULL);

  free(order);
  om_free(&map);
  printf("PASSED\n");
}

// Test 7: Remove non-existent entry
static void test_remove_not_found(void) {
  printf("test_remove_not_found... ");

  order_map_t map;
  om_init(&map, 16);

  order_t *order = make_order(1, SIDE_BUY, 100, 10);
  om_insert(&map, 1, order, SIDE_BUY, 100);

  int rc = om_remove(&map, 999);
  assert(rc == -1);
  assert(map.count == 1);

  free(order);
  om_free(&map);
  printf("PASSED\n");
}

// Test 8: Remove from empty bucket
static void test_remove_empty_bucket(void) {
  printf("test_remove_empty_bucket... ");

  order_map_t map;
  om_init(&map, 16);

  int rc = om_remove(&map, 42);
  assert(rc == -1);

  om_free(&map);
  printf("PASSED\n");
}

// Test 9: Remove head of chain
static void test_remove_chain_head(void) {
  printf("test_remove_chain_head... ");

  order_map_t map;
  om_init(&map, 8);

  // IDs 1, 9, 17 all hash to bucket 1
  order_t *o1 = make_order(1, SIDE_BUY, 100, 10);
  order_t *o9 = make_order(9, SIDE_SELL, 200, 20);
  order_t *o17 = make_order(17, SIDE_BUY, 300, 30);

  om_insert(&map, 1, o1, SIDE_BUY, 100);
  om_insert(&map, 9, o9, SIDE_SELL, 200);
  om_insert(&map, 17, o17, SIDE_BUY, 300);  // head of chain

  // Remove head (17 was inserted last, so it's the head)
  int rc = om_remove(&map, 17);
  assert(rc == 0);
  assert(map.count == 2);

  // 17 should be gone, 1 and 9 should remain
  assert(om_find(&map, 17) == NULL);
  assert(om_find(&map, 1) != NULL);
  assert(om_find(&map, 9) != NULL);

  free(o1);
  free(o9);
  free(o17);
  om_free(&map);
  printf("PASSED\n");
}

// Test 10: Remove middle of chain
static void test_remove_chain_middle(void) {
  printf("test_remove_chain_middle... ");

  order_map_t map;
  om_init(&map, 8);

  // IDs 1, 9, 17 all hash to bucket 1
  // Insert order: 1, 9, 17 → chain is: 17 -> 9 -> 1 -> NULL
  order_t *o1 = make_order(1, SIDE_BUY, 100, 10);
  order_t *o9 = make_order(9, SIDE_SELL, 200, 20);
  order_t *o17 = make_order(17, SIDE_BUY, 300, 30);

  om_insert(&map, 1, o1, SIDE_BUY, 100);
  om_insert(&map, 9, o9, SIDE_SELL, 200);
  om_insert(&map, 17, o17, SIDE_BUY, 300);

  // Remove middle (9)
  int rc = om_remove(&map, 9);
  assert(rc == 0);
  assert(map.count == 2);

  // 9 should be gone, 1 and 17 should remain
  assert(om_find(&map, 9) == NULL);
  assert(om_find(&map, 1) != NULL);
  assert(om_find(&map, 17) != NULL);

  free(o1);
  free(o9);
  free(o17);
  om_free(&map);
  printf("PASSED\n");
}

// Test 11: Remove tail of chain
static void test_remove_chain_tail(void) {
  printf("test_remove_chain_tail... ");

  order_map_t map;
  om_init(&map, 8);

  // IDs 1, 9, 17 all hash to bucket 1
  // Chain: 17 -> 9 -> 1 -> NULL (1 is tail)
  order_t *o1 = make_order(1, SIDE_BUY, 100, 10);
  order_t *o9 = make_order(9, SIDE_SELL, 200, 20);
  order_t *o17 = make_order(17, SIDE_BUY, 300, 30);

  om_insert(&map, 1, o1, SIDE_BUY, 100);
  om_insert(&map, 9, o9, SIDE_SELL, 200);
  om_insert(&map, 17, o17, SIDE_BUY, 300);

  // Remove tail (1)
  int rc = om_remove(&map, 1);
  assert(rc == 0);
  assert(map.count == 2);

  // 1 should be gone, 9 and 17 should remain
  assert(om_find(&map, 1) == NULL);
  assert(om_find(&map, 9) != NULL);
  assert(om_find(&map, 17) != NULL);

  free(o1);
  free(o9);
  free(o17);
  om_free(&map);
  printf("PASSED\n");
}

// Test 12: Large number of entries
static void test_many_entries(void) {
  printf("test_many_entries... ");

  order_map_t map;
  om_init(&map, 64);

  const int N = 1000;
  order_t **orders = malloc(N * sizeof(order_t *));

  // Insert N orders
  for (int i = 0; i < N; i++) {
    orders[i] = make_order(i + 1, (i % 2) ? SIDE_BUY : SIDE_SELL, 100 + i, 10);
    om_insert(&map, i + 1, orders[i], orders[i]->side, orders[i]->price);
  }

  assert(map.count == N);

  // Verify all findable
  for (int i = 0; i < N; i++) {
    om_entry_t *entry = om_find(&map, i + 1);
    assert(entry != NULL);
    assert(entry->order == orders[i]);
  }

  // Remove every other one
  for (int i = 0; i < N; i += 2) {
    int rc = om_remove(&map, i + 1);
    assert(rc == 0);
  }

  assert(map.count == N / 2);

  // Verify removals
  for (int i = 0; i < N; i++) {
    om_entry_t *entry = om_find(&map, i + 1);
    if (i % 2 == 0) {
      assert(entry == NULL);  // removed
    } else {
      assert(entry != NULL);  // still there
    }
  }

  for (int i = 0; i < N; i++) {
    free(orders[i]);
  }
  free(orders);
  om_free(&map);
  printf("PASSED\n");
}

// Test 13: NULL inputs
static void test_null_inputs(void) {
  printf("test_null_inputs... ");

  order_map_t map;
  om_init(&map, 16);

  // om_insert with NULL map
  assert(om_insert(NULL, 1, NULL, SIDE_BUY, 100) == -1);

  // om_find with NULL map
  assert(om_find(NULL, 1) == NULL);

  // om_remove with NULL map
  assert(om_remove(NULL, 1) == -1);

  // om_init with NULL map (should not crash)
  om_init(NULL, 16);

  // om_free with NULL map (should not crash)
  om_free(NULL);

  om_free(&map);
  printf("PASSED\n");
}

int main(void) {
  printf("\n=== Running order_map tests ===\n\n");

  test_init_free();
  test_insert_find_single();
  test_find_not_found();
  test_insert_multiple();
  test_collision();
  test_remove_single();
  test_remove_not_found();
  test_remove_empty_bucket();
  test_remove_chain_head();
  test_remove_chain_middle();
  test_remove_chain_tail();
  test_many_entries();
  test_null_inputs();

  printf("\n=== All tests PASSED ===\n\n");
  return 0;
}