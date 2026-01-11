#include "core/order_map.h"
#include <stdlib.h>

void om_init(order_map_t *map, size_t num_buckets) {
  // 1. Validate input
  if (!map || num_buckets == 0) {
    return;
  }
  
  // 2. Allocate buckets array (use calloc)
  map->buckets = calloc(num_buckets, sizeof(om_entry_t *));
  
  // 3. Set num_buckets and count
  map->count = 0;
  map->num_buckets = num_buckets;
}

int om_insert(order_map_t *map, order_id_t id, order_t *order, side_t side, price_t price) {
  // 1. Validate inputs (return -1 if invalid)
  if (!map) {
    return -1;
  }
  
  // 2. Calculate bucket index
  uint64_t idx = id % map->num_buckets;

  // 3. Allocate new om_entry_t node
  om_entry_t *z = malloc(sizeof(om_entry_t));
  // 4. Fill in the node fields (key, order, side, price)
  z->key = id;
  z->order = order;
  z->price = price;
  z->side = side;
  // 5. Prepend to bucket's linked list (new node becomes head)
  z->next = map->buckets[idx];
  map->buckets[idx] = z;
  // 6. Increment map->count
  map->count++;
  // 7. Return 0 for success
  return 0;
}

om_entry_t *om_find(order_map_t *map, order_id_t id) {
  // 1. Validate input (return NULL if invalid)
  if (!map) {
    return NULL;
  }
  // 2. Calculate bucket index
  uint64_t idx = id % map->num_buckets;
  // 3. Walk the linked list in that bucket
  //    - If node->key == id, return that node
  om_entry_t *curr = map->buckets[idx];
  while (curr != NULL) {
    if (curr->key == id) {
        return curr;
    } else {
        curr = curr->next;
    }
  }
  
  // 4. Return NULL if not found
  return NULL;
}

int om_remove(order_map_t *map, order_id_t id) {
  // 1. Validate input (return -1 if invalid)
  if (!map) return -1;
  // 2. Calculate bucket index
  uint64_t idx = id % map->num_buckets;
  // 3. Handle special case: if head node matches, update bucket head
  om_entry_t *curr = map->buckets[idx];

  if (!curr) {
    return -1;
  }

  if (curr->key == id) {
    map->buckets[idx] = map->buckets[idx]->next;
    free(curr);
    map->count--;
    return 0;
  }
  // 4. Otherwise, walk list keeping track of previous node
  //    - When found: prev->next = curr->next (unlink curr)
  //    - free(curr)
  om_entry_t *prev = curr;
  curr = curr->next;
  while (curr != NULL) {
    if (curr->key == id) {
      prev->next = curr->next;
      free(curr);
      map->count--;
      return 0;
    } else {
      prev = curr;
      curr = curr->next;
    }
  }
  
  // 5. Decrement map->count
  
  // 6. Return 0 on success, -1 if not found
  return -1;
}

void om_free(order_map_t *map) {
  // 1. Validate input
  if (!map) {
    return;
  }
  
  // 2. For each bucket:
  //    - Walk the linked list and free each node
  size_t size = map->num_buckets;

  for (size_t i = 0; i < size; i++) {
    om_entry_t *curr = map->buckets[i];
    while (curr != NULL) {
      om_entry_t *next = curr->next;
      free(curr);
      curr = next;
    }
  }
  
  // 3. Free the buckets array itself
  free(map->buckets);
  // 4. Reset map fields (optional but good practice)

  map->buckets = NULL;
  map->count = 0;
  map->num_buckets = 0;
}
