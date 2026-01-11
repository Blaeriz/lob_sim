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