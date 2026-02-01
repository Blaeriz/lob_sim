#ifndef MARKET_MAKER_H
#define MARKET_MAKER_H

#include "agent.h"

agent_t *market_maker_create(agent_id_t id);
void market_maker_destroy(agent_t *agent);

#endif
