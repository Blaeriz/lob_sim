#ifndef LEVEL_OPS_H
#define LEVEL_OPS_H

#include "level.h"

void level_init(price_level_t *level, price_t price);

int level_is_empty(const price_level_t *level);

order_t *level_peek(const price_level_t *level);

order_node_t *level_push(price_level_t *level, order_t *order);

order_t *level_pop(price_level_t *level);

void level_free_queue(price_level_t *level, void (*free_order)(order_t *order));

int level_remove(price_level_t *level, order_node_t *node);

#endif