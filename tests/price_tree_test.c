#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "core/price_tree.h"
#include "core/level_ops.h"

/*
  This is a smoke/regression test for price_tree.c:
  - NIL sentinel initialization
  - insert/find/min/max
  - duplicate insert
  - remove leaf / one-child / two-children
  - remove all and verify empty invariants
*/

static void expect_empty(const price_tree_t *t) {
  assert(t->root == &t->nil);
  assert(t->size == 0);
  assert(pt_min(t) == NULL);
  assert(pt_max(t) == NULL);
}

static void expect_found(const price_tree_t *t, price_t p, price_level_t *lvl) {
  price_level_t *got = pt_find(t, p);
  assert(got == lvl);
}

static void expect_not_found(const price_tree_t *t, price_t p) {
  assert(pt_find(t, p) == NULL);
}

static void test_basic_insert_find_min_max(void) {
  price_tree_t t;
  pt_init(&t);
  expect_empty(&t);

  price_level_t l99, l100, l101;
  level_init(&l99, 99);
  level_init(&l100, 100);
  level_init(&l101, 101);

  assert(pt_insert(&t, 100, &l100) == 1);
  assert(pt_insert(&t, 99, &l99) == 1);
  assert(pt_insert(&t, 101, &l101) == 1);

  expect_found(&t, 99, &l99);
  expect_found(&t, 100, &l100);
  expect_found(&t, 101, &l101);
  expect_not_found(&t, 102);

  assert(pt_min(&t) == &l99);
  assert(pt_max(&t) == &l101);

  // duplicate
  assert(pt_insert(&t, 100, &l100) == 0);
}

static void test_remove_leaf_one_child_two_children(void) {
  price_tree_t t;
  pt_init(&t);

  // Allocate levels so their addresses remain stable regardless of scope.
  // Tree does NOT own levels; it only stores pointers.
  enum { N = 7 };
  price_level_t *lvl[N];
  price_t keys[N] = { 10, 5, 15, 12, 18, 11, 13 };

  for (int i = 0; i < N; i++) {
    lvl[i] = (price_level_t *)malloc(sizeof *lvl[i]);
    assert(lvl[i] != NULL);
    level_init(lvl[i], keys[i]);
    assert(pt_insert(&t, keys[i], lvl[i]) == 1);
  }

  // Sanity: min/max
  assert(pt_min(&t)->price == 5);
  assert(pt_max(&t)->price == 18);

  // Remove a leaf (13 is typically a leaf in this shape)
  assert(pt_remove(&t, 13) == 1);
  expect_not_found(&t, 13);

  // Remove a node with one child (11 after removing 13? often one-child/leaf depending on rotations)
  assert(pt_remove(&t, 11) == 1);
  expect_not_found(&t, 11);

  // Remove a node with two children (10 is root-ish; likely has 2 children)
  assert(pt_remove(&t, 10) == 1);
  expect_not_found(&t, 10);

  // Remaining keys should still be findable
  // (some removed already: 10, 11, 13)
  for (int i = 0; i < N; i++) {
    if (keys[i] == 10 || keys[i] == 11 || keys[i] == 13) continue;
    assert(pt_find(&t, keys[i]) == lvl[i]);
  }

  // Cleanup: remove remaining from tree, then free levels
  for (int i = 0; i < N; i++) {
    if (pt_find(&t, keys[i]) != NULL) {
      assert(pt_remove(&t, keys[i]) == 1);
    }
    free(lvl[i]);
  }

  expect_empty(&t);
}

static void test_bulk_insert_remove(void) {
  price_tree_t t;
  pt_init(&t);

  enum { M = 1000 };
  price_level_t *levels[M];

  // Insert 1..M
  for (int i = 0; i < M; i++) {
    levels[i] = (price_level_t *)malloc(sizeof *levels[i]);
    assert(levels[i] != NULL);
    level_init(levels[i], (price_t)(i + 1));
    int rc = pt_insert(&t, (price_t)(i + 1), levels[i]);
    assert(rc == 1);
  }

  assert(t.size == (size_t)M);
  assert(pt_min(&t)->price == 1);
  assert(pt_max(&t)->price == M);

  // Remove evens
  for (int i = 2; i <= M; i += 2) {
    assert(pt_remove(&t, (price_t)i) == 1);
  }

  // Check: evens missing, odds present
  for (int i = 1; i <= M; i++) {
    if (i % 2 == 0) {
      expect_not_found(&t, (price_t)i);
    } else {
      assert(pt_find(&t, (price_t)i) != NULL);
      assert(pt_find(&t, (price_t)i)->price == (price_t)i);
    }
  }

  // Remove odds
  for (int i = 1; i <= M; i += 2) {
    assert(pt_remove(&t, (price_t)i) == 1);
  }

  // Now empty
  expect_empty(&t);

  // Free all levels (tree doesn't own them)
  for (int i = 0; i < M; i++) {
    free(levels[i]);
  }
}

int main(void) {
  test_basic_insert_find_min_max();
  test_remove_leaf_one_child_two_children();
  test_bulk_insert_remove();

  printf("price_tree_test: OK\n");
  return 0;
}