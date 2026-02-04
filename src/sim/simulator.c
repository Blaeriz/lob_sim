#include "sim/simulator.h"
#include <stdlib.h>
#include <stdio.h>

static simulator_t g_sim;

#define SIMULATOR_INITIAL_CAPACITY 16 

void simulator_init(order_book_t *book) {
    g_sim.book = book;
    g_sim.current_time = 0;
    g_sim.dt = 1;
    g_sim.agent_capacity = SIMULATOR_INITIAL_CAPACITY;
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

void simulator_add_agent(agent_t *agent) {
    if(!agent) {
        fprintf(stderr, "Cannot add NULL agent\n");
        return;
    }

    if (g_sim.agent_count >= g_sim.agent_capacity) {
        size_t new_size = g_sim.agent_capacity * 2;
        agent_t **temp = realloc(g_sim.agents, new_size * sizeof(agent_t *));
        if(!temp) {
            fprintf(stderr, "Failed to allocate memory for agents array\n");
            exit(EXIT_FAILURE);
        }
        g_sim.agents = temp;

        for(size_t i = g_sim.agent_capacity; i<new_size; i++) {
            g_sim.agents[i] = NULL;
        }
        g_sim.agent_capacity = new_size;
    }

    g_sim.agents[g_sim.agent_count] = agent;

    g_sim.agent_count++;
}

void simulator_run(timestamp_t end_time) {
    while (g_sim.current_time < end_time) {
        for (size_t i = 0; i < g_sim.agent_count; i++) {
            agent_t *agent = g_sim.agents[i];
            if (agent != NULL && agent->step != NULL) {
                agent->step(agent, g_sim.book,  g_sim.current_time);
            }
        }

        // if (g_sim.current_time % 1000 == 0) {
        //     printf("Time: %lu\n", g_sim.current_time);
        // }

        g_sim.current_time += g_sim.dt;
    }

    //printf("Simulation complete at time %lu\n", g_sim.current_time);
}

void simulator_free() {

    if (g_sim.agents) {
        free(g_sim.agents);
    }

    g_sim.agents = NULL;
    g_sim.book = NULL;
    
    g_sim.agent_count = 0;
    g_sim.agent_capacity = 0;
}
