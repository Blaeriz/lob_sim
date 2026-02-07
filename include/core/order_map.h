#ifndef ORDER_MAP_H
#define ORDER_MAP_H

#include "core/level.h"
#include "core/order.h"
#include <stddef.h>

// What structures do you need?

// Node in the linked list (one per order in a bucket)
typedef struct om_entry
{
  order_id_t key; // the order ID
  order_t* order; // pointer to the order
  side_t side;    // which side of the book
  price_t price;  // price level (for fast tree lookup)
  struct order_node* node;
  struct om_entry* next; // next node in chain
} om_entry_t;

// The hash map itself
typedef struct
{
  om_entry_t** buckets; // array of linked list heads
  size_t num_buckets;   // size of the array
  size_t count;         // number of entries (optional, for stats)
} order_map_t;

// What functions do you need?
void om_init(order_map_t* map, size_t num_buckets);
int om_insert(order_map_t* map, order_id_t id, order_t* order, side_t side, price_t price,
              order_node_t* node);
om_entry_t* om_find(order_map_t* map, order_id_t id);
int om_remove(order_map_t* map, order_id_t id);
void om_free(order_map_t* map);

#endif