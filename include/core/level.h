#ifndef LEVEL_H
#define LEVEL_H

#include "../common/types.h"
#include "order.h"

typedef struct order_node {
  order_t *order;
  struct order_node *next;
} order_node_t;

typedef struct {
  price_t price;
  qty_t total_qty;
  order_node_t *head;
  order_node_t *tail;
} price_level_t;

#endif
