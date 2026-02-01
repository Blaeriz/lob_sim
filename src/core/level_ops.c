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

int level_push(price_level_t *level, order_t *order){
    order_node_t *new_node = malloc(sizeof *new_node);
    if (!new_node) {
        return 0; // Handle memory allocation failure
    }
    new_node->order = order;
    new_node->next = NULL;
    if(level->tail) {
        level->tail->next = new_node;
        level->tail = new_node;
        level->total_qty += order->qty;
        level_assert_invariants(level);
        return 1;
    } else {
        level->head = new_node;
        level->tail = new_node;
        level->total_qty += order->qty;
        level_assert_invariants(level);
        return 1;
    }
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

int level_remove(price_level_t *level, order_t *order) {
  // 1. Validate inputs (return -1 if invalid)
  if(!level || !order) {
    return -1;
  }
  // 2. Handle empty list
  if (level->head ==  NULL) {
    return -1;
  }
  // 3. Special case: head matches
  //    - Update level->head
  //    - If head was also tail, update level->tail to NULL
  //    - Decrement level->total_qty by order->qty
  //    - Free the node (NOT the order — caller owns it)
  //    - Return 0
  if (level->head->order == order) {
    order_node_t *t = level->head;
    if (level->head == level->tail) {
      level->head = level->tail = NULL;
    } else {
        level->head = level->head->next;
    }
    level->total_qty -= order->qty;
    free(t);

    return 0;
  }
  
  // 4. Walk list with prev/curr pointers
  //    - If curr->order == order:
  //      - prev->next = curr->next
  //      - If curr was tail, update level->tail = prev
  //      - Decrement level->total_qty
  //      - Free the node
  //      - Return 0
  order_node_t *prev = level->head;
  order_node_t *curr = level->head->next;

  while (curr != NULL) {
    if (curr->order == order) {
      order_node_t *temp = curr;
      prev->next = curr->next;
      if (curr == level->tail) {
        level->tail = prev;
      }
      level->total_qty -= order->qty;
      free(temp);

      return 0;
    } else {
      prev = curr;
      curr = curr->next;
    }
  }
  
  // 5. Return -1 if not found

  return -1;
}