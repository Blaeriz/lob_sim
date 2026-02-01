#ifndef INFORMED_TRADER_H
#define INFORMED_TRADER_H

#include "agent.h"

agent_t *informed_trader_create(agent_id_t id);
void informed_trader_destroy(agent_t *agent);

#endif
