#include "agents/noise_trader.h"
#include "core/book.h"
#include "sim/simulator.h"

int main(void) {
  order_book_t book;
  book_init(&book);

  simulator_init(&book);

  agent_t *a = noise_trader_create(1);
  simulator_add_agent(a);

  simulator_run(1000);

  book_free(&book);
  return 0;
}
