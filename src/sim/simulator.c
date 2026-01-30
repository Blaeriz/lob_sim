#include "sim/simulator.h"
#include <stdlib.h>
#include <stdio.h>

static simulator_t g_sim;

void simulator_init(order_book_t *book) {
    g_sim.book = book;
    g_sim.current_time = 0;
    g_sim.dt = 1;
    g_sim.agent_capacity = 10;
    g_sim.agent_count = 0;
    g_sim.agents = malloc(g_sim.agent_capacity * sizeof(agent_t *));

    if(!g_sim.agents) {
        fprintf(stderr, "Failed to allocate memory for agents array\n");
        exit(EXIT_FAILURE);
    }

    for(size_t i = 0; i < g_sim.agent_capacity; i++) {
        g_sim.agents[i] = NULL;
    }
}

void simulator_add_agent(agent_t *agent) { (void)agent; }

void simulator_run(timestamp_t end_time) { (void)end_time; }
