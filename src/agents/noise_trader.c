#include "../../include/agents/noise_trader.h"
#include <stdlib.h>

static void noise_step(agent_t *agent, timestamp_t now) {
  (void)agent;
  (void)now;
}

agent_t *noise_trader_create(agent_id_t id) {
  agent_t *a = malloc(sizeof(agent_t));
  a->id = id;
  a->step = noise_step;
  a->state = NULL;
  return a;
}
