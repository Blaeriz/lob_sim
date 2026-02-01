#ifndef NOISE_TRADER_H
#define NOISE_TRADER_H

#include "agent.h"

agent_t *noise_trader_create(agent_id_t id);
void noise_trader_destroy(agent_t *agent);

#endif
