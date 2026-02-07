#include "core/level_ops.h"
#include <assert.h>
#include <stdlib.h>

static void level_assert_invariants(const price_level_t *level) {
    /* Empty queue: head and tail must both be NULL */
    if (level->head == NULL) {
        assert(level->tail == NULL);
    }

    /* Non-empty queue: tail must exist and tail->next must be NULL */
    if (level->tail != NULL) {
        assert(level->tail->next == NULL);
    }
}

void level_init(price_level_t *level, price_t price) {
    level->price = price;
    level->total_qty = 0;
    level->head = NULL;
    level->tail = NULL;

    level_assert_invariants(level);
}

int level_is_empty(const price_level_t *level) {
    return level->head == NULL;
}

order_t *level_peek(const price_level_t *level) {
    if (!level->head) return NULL;
    return level->head->order;
}

order_node_t *level_push(price_level_t *level, order_t *order){
  order_node_t *new_node = malloc(sizeof *new_node);
  if (!new_node) {
      return 0; // Handle memory allocation failure
  }
  new_node->order = order;
  new_node->next = NULL;
  new_node->prev = NULL;
  if(level->tail) {
    level->tail->next = new_node;
    new_node->prev = level->tail;
    level->tail = new_node;
  }else {
    level->head = new_node;
    level->tail = new_node;
  }

  level->total_qty += order->qty;
  level_assert_invariants(level);
  return new_node; 
}

order_t *level_pop(price_level_t *level) {
  level_assert_invariants(level);
  if (!level->head) { return NULL; }
  order_node_t *temp = level->head;
  order_t *o = temp->order;
  level->head = level->head->next;
  if(!(level->head)){
      level->tail = NULL;
  }
  //level->total_qty -= o->qty;

  if (level->head) {
    level->head->prev = NULL;
  }

  free(temp);

  level_assert_invariants(level);
  return o;
}

void level_free_queue(price_level_t *level, void (*free_order)(order_t *order)) {
    if (!level) return;

    order_node_t *cur = level->head;
    while (cur) {
        order_node_t *next = cur->next;
        if (free_order && cur->order) free_order(cur->order);
        free(cur);
        cur = next;
    }

    level->head = NULL;
    level->tail = NULL;
    level->total_qty = 0;
}

int level_remove(price_level_t *level, order_node_t *node) {
  // 1. Validate inputs (return -1 if invalid)
  if(!level || !node) {
    return -1;
  }

  if (node == level->head) {
    level->head = node->next;
  }

  if (node == level->tail) {
    level->tail = node->prev;
  }

  // Stitch neighbors together
  if (node->prev) {
      node->prev->next = node->next;
  }
  if (node->next) {
      node->next->prev = node->prev;
  }

  level->total_qty -= node->order->qty;

  free(node);

  return 0;
}