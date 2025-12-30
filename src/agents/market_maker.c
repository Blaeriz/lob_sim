#include "agents/market_maker.h"
#include <stdlib.h>

static void mm_step(agent_t *agent, timestamp_t now) {
  (void)agent;
  (void)now;
}

agent_t *market_maker_create(agent_id_t id) {
  agent_t *a = malloc(sizeof(agent_t));
  if (!a) return NULL;
  a->id = id;
  a->step = mm_step;
  a->state = NULL;
  return a;
}
