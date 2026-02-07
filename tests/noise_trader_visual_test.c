#define _DEFAULT_SOURCE
#include "agents/noise_trader.h"
#include "core/book.h"
#include "core/level_ops.h"
#include "core/price_tree.h"
#include "sim/simulator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Colors (ANSI escape codes)
#define COLOR_RESET "\033[0m"
#define COLOR_GREEN "\033[32m"
#define COLOR_RED "\033[31m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_CYAN "\033[36m"
#define COLOR_BOLD "\033[1m"
#define CLEAR_SCREEN() printf("\033[2J\033[H")

#define MAX_DISPLAY_LEVELS 8
#define ANIMATION_DELAY_MS 100

// Collect prices from tree into array (for display)
typedef struct
{
  price_t price;
  qty_t total_qty;
  int order_count;
} level_info_t;

static void collect_level(price_level_t* lvl, level_info_t* info)
{
  info->price = lvl->price;
  info->total_qty = lvl->total_qty;
  info->order_count = 0;
  order_node_t* node = lvl->head;
  while (node)
  {
    info->order_count++;
    node = node->next;
  }
}

static void print_book(order_book_t* book, const char* message)
{
  CLEAR_SCREEN();
  printf("\n");
  printf(COLOR_BOLD "  ╔════════════════════════════════════════════════════════╗\n" COLOR_RESET);
  printf(COLOR_BOLD "  ║            📊 LIMIT ORDER BOOK SIMULATOR 📊            ║\n" COLOR_RESET);
  printf(COLOR_BOLD "  ╚════════════════════════════════════════════════════════╝\n" COLOR_RESET);
  printf("\n");
  level_info_t bids[MAX_DISPLAY_LEVELS] = {0};
  level_info_t asks[MAX_DISPLAY_LEVELS] = {0};
  int num_bids = 0;
  int num_asks = 0;
  price_level_t* ask = pt_min(&book->asks);
  while (ask && num_asks < MAX_DISPLAY_LEVELS)
  {
    collect_level(ask, &asks[num_asks]);
    num_asks++;
    price_level_t* next_ask = NULL;
    price_t min_price = INT64_MAX;
    for (price_t p = ask->price + 1; p < ask->price + 1000; p++)
    {
      price_level_t* found = pt_find(&book->asks, p);
      if (found && found->price < min_price)
      {
        min_price = found->price;
        next_ask = found;
        break;
      }
    }
    ask = next_ask;
  }
  price_level_t* bid = pt_max(&book->bids);
  while (bid && num_bids < MAX_DISPLAY_LEVELS)
  {
    collect_level(bid, &bids[num_bids]);
    num_bids++;
    price_level_t* next_bid = NULL;
    for (price_t p = bid->price - 1; p > bid->price - 1000; p--)
    {
      price_level_t* found = pt_find(&book->bids, p);
      if (found)
      {
        next_bid = found;
        break;
      }
    }
    bid = next_bid;
  }
  printf("       " COLOR_BOLD "%-20s      %-20s\n" COLOR_RESET, "BIDS (Buyers)", "ASKS (Sellers)");
  printf("       ─────────────────────────────────────────────────\n");
  for (int i = num_asks - 1; i >= 0; i--)
  {
    printf("       %20s      " COLOR_RED "%6lld @ %-6lld" COLOR_RESET " [%d orders]\n", "",
           (long long)asks[i].total_qty, (long long)asks[i].price, asks[i].order_count);
  }
  price_level_t* best_bid = pt_max(&book->bids);
  price_level_t* best_ask = pt_min(&book->asks);
  printf("       ─────────────────────────────────────────────────\n");
  if (best_bid && best_ask)
  {
    price_t spread = best_ask->price - best_bid->price;
    printf(COLOR_YELLOW "       %20s  ⬆ SPREAD: %lld ⬇\n" COLOR_RESET, "", (long long)spread);
  }
  else
  {
    printf(COLOR_YELLOW "       %20s  ⬆ SPREAD: --- ⬇\n" COLOR_RESET, "");
  }
  printf("       ─────────────────────────────────────────────────\n");
  for (int i = 0; i < num_bids; i++)
  {
    printf("       " COLOR_GREEN "%6lld @ %-6lld" COLOR_RESET " [%d orders]\n",
           (long long)bids[i].total_qty, (long long)bids[i].price, bids[i].order_count);
  }
  for (int i = num_bids; i < 3; i++)
  {
    printf("\n");
  }
  printf("\n       ─────────────────────────────────────────────────\n");
  printf(COLOR_CYAN "       📈 Total Orders: %zu\n" COLOR_RESET, book->orders.count);
  if (message && strlen(message) > 0)
  {
    printf("\n" COLOR_BOLD "       💬 %s\n" COLOR_RESET, message);
  }
  printf("\n");
}

int main(void)
{
  order_book_t book;
  book_init(&book);
  simulator_init(&book);
  int num_agents = 3;
  agent_t* agents[num_agents];
  for (int i = 0; i < num_agents; ++i)
  {
    agents[i] = noise_trader_create(i + 1);
    simulator_add_agent(agents[i]);
  }
  int total_ticks = 5000;
  int display_every = 100;
  char msg[128];
  for (int t = 0; t < total_ticks; t += display_every)
  {
    simulator_run(t + display_every);
    snprintf(msg, sizeof(msg), "Tick %d/%d", t + display_every, total_ticks);
    print_book(&book, msg);
    usleep(100000); // 0.1s delay for animation
  }

  for (int i = 0; i < num_agents; i++)
  {
    noise_trader_destroy(agents[i]);
  }

  simulator_free();
  book_free(&book);
  return 0;
}
