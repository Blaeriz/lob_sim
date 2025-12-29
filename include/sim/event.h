#ifndef EVENT_H
#define EVENT_H

#include "../common/types.h"
#include "../core/order.h"
#include "../core/trade.h"

typedef enum { EVENT_ORDER, EVENT_CANCEL, EVENT_TRADE } event_type_t;

typedef struct {
  timestamp_t ts;
  event_type_t type;
  void *payload;
} event_t;

#endif
