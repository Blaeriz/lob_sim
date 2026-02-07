#define _DEFAULT_SOURCE
#include "agents/informed_trader.h"
#include "agents/market_maker.h"
#include "agents/noise_trader.h"
#include "core/book.h"
#include "core/level_ops.h"
#include "core/price_tree.h"
#include "sim/simulator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Colors
#define COLOR_RESET "\033[0m"
#define COLOR_GREEN "\033[32m"
#define COLOR_RED "\033[31m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_CYAN "\033[36m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_BLUE "\033[34m"
#define COLOR_BOLD "\033[1m"
#define CLEAR_SCREEN() printf("\033[2J\033[H")

#define MAX_DISPLAY_LEVELS 8

static size_t count_orders(price_level_t* lvl)
{
  size_t count = 0;
  order_node_t* node = lvl->head;
  while (node)
  {
    count++;
    node = node->next;
  }
  return count;
}

static size_t count_total_active_orders(order_book_t* book)
{
  size_t total = 0;

  // Count bid orders
  price_level_t* lvl = pt_max(&book->bids);
  while (lvl)
  {
    total += count_orders(lvl);
    price_level_t* next = NULL;
    price_t search = lvl->price - 1;
    while (search > 0 && search > lvl->price - 1000 && !next)
    {
      next = pt_find(&book->bids, search--);
    }
    lvl = next;
  }

  // Count ask orders
  lvl = pt_min(&book->asks);
  while (lvl)
  {
    total += count_orders(lvl);
    price_level_t* next = NULL;
    price_t search = lvl->price + 1;
    while (search < lvl->price + 1000 && !next)
    {
      next = pt_find(&book->asks, search++);
    }
    lvl = next;
  }

  return total;
}

static void print_book(order_book_t* book, const char* msg, int num_noise, int num_mm,
                       int num_informed)
{
  CLEAR_SCREEN();

  printf(COLOR_CYAN COLOR_BOLD);
  printf("  ╔════════════════════════════════════════════════════════╗\n");
  printf("  ║       📊 LOB SIM: FULL AGENT SIMULATION 📊            ║\n");
  printf("  ╚════════════════════════════════════════════════════════╝\n\n" COLOR_RESET);

  printf(COLOR_BOLD "       BIDS (Buyers)             ASKS (Sellers)\n" COLOR_RESET);
  printf("       ─────────────────────────────────────────────────\n");

  // Collect ask prices (lowest first)
  price_t ask_prices[MAX_DISPLAY_LEVELS];
  int num_asks = 0;
  price_level_t* lvl = pt_min(&book->asks);
  while (lvl && num_asks < MAX_DISPLAY_LEVELS)
  {
    ask_prices[num_asks++] = lvl->price;
    price_level_t* next = NULL;
    price_t search = lvl->price + 1;
    while (search < lvl->price + 1000 && !next)
    {
      next = pt_find(&book->asks, search++);
    }
    lvl = next;
  }

  // Print asks (high to low for display)
  for (int i = num_asks - 1; i >= 0; i--)
  {
    lvl = pt_find(&book->asks, ask_prices[i]);
    if (lvl)
    {
      printf(COLOR_RED
             "                                    %4ld @ %-6ld [%zu orders]\n" COLOR_RESET,
             lvl->total_qty, lvl->price, count_orders(lvl));
    }
  }

  // Collect bid prices (highest first)
  price_t bid_prices[MAX_DISPLAY_LEVELS];
  int num_bids = 0;
  lvl = pt_max(&book->bids);
  while (lvl && num_bids < MAX_DISPLAY_LEVELS)
  {
    bid_prices[num_bids++] = lvl->price;
    price_level_t* next = NULL;
    price_t search = lvl->price - 1;
    while (search > lvl->price - 1000 && search > 0 && !next)
    {
      next = pt_find(&book->bids, search--);
    }
    lvl = next;
  }

  // Calculate spread
  price_t best_bid = num_bids > 0 ? bid_prices[0] : 0;
  price_t best_ask = num_asks > 0 ? ask_prices[0] : 0;
  price_t spread = (best_bid && best_ask) ? best_ask - best_bid : 0;

  printf("       ─────────────────────────────────────────────────\n");
  printf(COLOR_YELLOW "                             ⬆ SPREAD: %ld ⬇\n" COLOR_RESET, spread);
  printf("       ─────────────────────────────────────────────────\n");

  // Print bids (high to low)
  for (int i = 0; i < num_bids; i++)
  {
    lvl = pt_find(&book->bids, bid_prices[i]);
    if (lvl)
    {
      printf(COLOR_GREEN "          %4ld @ %-6ld [%zu orders]\n" COLOR_RESET, lvl->total_qty,
             lvl->price, count_orders(lvl));
    }
  }

  printf("\n       ─────────────────────────────────────────────────\n");

  // Stats
  size_t total_orders = count_total_active_orders(book);
  printf(COLOR_MAGENTA "       🎲 Noise Traders: %d\n" COLOR_RESET, num_noise);
  printf(COLOR_BLUE "       🏦 Market Makers: %d\n" COLOR_RESET, num_mm);
  printf(COLOR_YELLOW "       🧠 Informed Traders: %d\n" COLOR_RESET, num_informed);
  printf(COLOR_CYAN "       📈 Total Orders: %zu\n" COLOR_RESET, total_orders);
  printf(COLOR_CYAN "\n       💬 %s\n\n" COLOR_RESET, msg);
}

int main(void)
{
  order_book_t book;
  book_init(&book);
  simulator_init(&book);

  // Create agents
  int num_noise = 5;
  int num_mm = 2;
  int num_informed = 2;

  agent_t* noise_agents[5];
  agent_t* mm_agents[2];
  agent_t* informed_agents[2];

  // Add noise traders (IDs 1-5)
  for (int i = 0; i < num_noise; ++i)
  {
    noise_agents[i] = noise_trader_create(i + 1);
    simulator_add_agent(noise_agents[i]);
  }

  // Add market makers (IDs 100-101)
  for (int i = 0; i < num_mm; ++i)
  {
    mm_agents[i] = market_maker_create(100 + i);
    simulator_add_agent(mm_agents[i]);
  }

  // Add informed traders (IDs 200-201)
  for (int i = 0; i < num_informed; ++i)
  {
    informed_agents[i] = informed_trader_create(200 + i);
    simulator_add_agent(informed_agents[i]);
  }

  int total_ticks = 5000;
  int display_every = 50;
  char msg[128];

  for (int t = 0; t < total_ticks; t += display_every)
  {
    simulator_run(t + display_every);
    snprintf(msg, sizeof(msg), "Tick %d/%d", t + display_every, total_ticks);
    print_book(&book, msg, num_noise, num_mm, num_informed);
    usleep(80000);
  }

  // Cleanup
  for (int i = 0; i < num_noise; i++)
  {
    noise_trader_destroy(noise_agents[i]);
  }
  for (int i = 0; i < num_mm; i++)
  {
    market_maker_destroy(mm_agents[i]);
  }
  for (int i = 0; i < num_informed; i++)
  {
    informed_trader_destroy(informed_agents[i]);
  }

  simulator_free();
  book_free(&book);

  printf(COLOR_GREEN "\n  Simulation complete!\n" COLOR_RESET);
  return 0;
}