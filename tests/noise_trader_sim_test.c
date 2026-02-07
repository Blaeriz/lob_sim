#include "agents/noise_trader.h"
#include "core/book.h"
#include "sim/simulator.h"
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
  // 1. Create and initialize the order book
  order_book_t book;
  book_init(&book);

  // 2. Initialize the simulator
  simulator_init(&book);

  // 3. Create and add noise traders
  int num_agents = 3;
  for (int i = 0; i < num_agents; ++i)
  {
    agent_t* nt = noise_trader_create(i + 1);
    if (!nt)
    {
      fprintf(stderr, "Failed to create noise trader %d\n", i + 1);
      return 1;
    }
    simulator_add_agent(nt);
  }

  // 4. Run the simulation for 5000 ticks
  printf("Running simulation with %d noise traders for 5000 ticks...\n", num_agents);
  simulator_run(5000);

  // 5. Cleanup
  simulator_free();
  book_free(&book);
  printf("Simulation complete.\n");
  return 0;
}
