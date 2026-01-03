#include <assert.h>
#include <stdio.h>

#include "core/price_tree.h"
#include "core/level_ops.h"

int main(void) {
  price_tree_t t;
  pt_init(&t);

  // Empty tree: min/max/find should be NULL
  assert(pt_min(&t) == NULL);
  assert(pt_max(&t) == NULL);
  assert(pt_find(&t, 100) == NULL);

  // Create a few dummy price levels. Stack allocation is fine for tests.
  price_level_t l99, l100, l101;
  level_init(&l99, 99);
  level_init(&l100, 100);
  level_init(&l101, 101);

  // Insert
  assert(pt_insert(&t, 100, &l100) == 1);
  assert(pt_insert(&t, 99, &l99) == 1);
  assert(pt_insert(&t, 101, &l101) == 1);

  // Find
  assert(pt_find(&t, 99) == &l99);
  assert(pt_find(&t, 100) == &l100);
  assert(pt_find(&t, 101) == &l101);
  assert(pt_find(&t, 102) == NULL);

  // Min/Max
  assert(pt_min(&t) == &l99);
  assert(pt_max(&t) == &l101);

  // Duplicate key should be rejected (your chosen convention: 0)
  assert(pt_insert(&t, 100, &l100) == 0);

  printf("ptree_test: OK\n");
  return 0;
}