#define _DEFAULT_SOURCE
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "agents/informed_trader.h"
#include "agents/market_maker.h"
#include "agents/noise_trader.h"
#include "core/book.h"
#include "core/level_ops.h"
#include "core/price_tree.h"
#include "sim/simulator.h"
#include "sim/stats.h"

#ifdef BENCHMARK
#include "bench/latency.h"
latency_tracker_t add_order_tracker;
latency_tracker_t remove_order_tracker;
latency_tracker_t match_order_tracker;
#endif

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
#define MAX_AGENTS 100

// Configuration
typedef struct
{
  int num_noise;
  int num_mm;
  int num_informed;
  int total_ticks;
  int visual_mode;
} config_t;

// Helper to count orders in a level
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

static void print_usage(const char* program)
{
  printf("\n");
  printf(COLOR_CYAN COLOR_BOLD "LOB Simulator - Limit Order Book Simulation\n" COLOR_RESET);
  printf("\n");
  printf("Usage: %s [OPTIONS]\n", program);
  printf("\n");
  printf("Options:\n");
  printf("  -n, --noise NUM       Number of noise traders (default: 5)\n");
  printf("  -m, --mm NUM          Number of market makers (default: 2)\n");
  printf("  -i, --informed NUM    Number of informed traders (default: 2)\n");
  printf("  -t, --ticks NUM       Total simulation ticks (default: 5000)\n");
  printf("  -q, --quiet           Quiet mode (no progress bar)\n");
  printf("  -h, --help            Show this help message\n");
  printf("\n");
  printf("Examples:\n");
  printf("  %s                    Run with defaults\n", program);
  printf("  %s -n 10 -m 3 -i 1    10 noise, 3 MM, 1 informed\n", program);
  printf("  %s -t 100000 -q       Fast benchmark (100k ticks)\n", program);
  printf("\n");
}

static void print_book(order_book_t* book, config_t* cfg, int tick)
{
  CLEAR_SCREEN();

  printf(COLOR_CYAN COLOR_BOLD);
  printf("  ╔════════════════════════════════════════════════════════╗\n");
  printf("  ║            📊 LOB SIMULATOR 📊                        ║\n");
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
  price_t mid_price = (best_bid && best_ask) ? (best_bid + best_ask) / 2 : 0;

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
  printf(COLOR_MAGENTA "       🎲 Noise Traders: %d\n" COLOR_RESET, cfg->num_noise);
  printf(COLOR_BLUE "       🏦 Market Makers: %d\n" COLOR_RESET, cfg->num_mm);
  printf(COLOR_YELLOW "       🧠 Informed Traders: %d\n" COLOR_RESET, cfg->num_informed);
  printf("\n");
  printf(COLOR_CYAN "       📈 Mid Price: %ld\n" COLOR_RESET, mid_price);
  printf(COLOR_CYAN "       📊 Best Bid: %ld | Best Ask: %ld\n" COLOR_RESET, best_bid, best_ask);
  printf(COLOR_CYAN "\n       ⏱  Tick %d/%d\n\n" COLOR_RESET, tick, cfg->total_ticks);
}

static void print_final_stats(order_book_t* book, config_t* cfg, double elapsed_sec)
{
  price_level_t* bid_lvl = pt_max(&book->bids);
  price_level_t* ask_lvl = pt_min(&book->asks);

  price_t best_bid = bid_lvl ? bid_lvl->price : 0;
  price_t best_ask = ask_lvl ? ask_lvl->price : 0;
  price_t spread = (best_bid && best_ask) ? best_ask - best_bid : 0;
  price_t mid_price = (best_bid && best_ask) ? (best_bid + best_ask) / 2 : 0;

  market_stats_t stats = stats_snapshot();

  printf("\n");
  printf(COLOR_CYAN COLOR_BOLD);
  printf("  ╔════════════════════════════════════════════════════════╗\n");
  printf("  ║              📊 SIMULATION COMPLETE 📊                 ║\n");
  printf("  ╚════════════════════════════════════════════════════════╝\n\n" COLOR_RESET);

  // Print side-by-side table with values on same line
  printf("%-22s | %-22s | %-22s | %-22s\n", "Configuration", "Final Market State", "Trading Activity", "Performance");
  printf("-------------------------------------------------------------------------------------------------------------\n");
  printf("%-22s %-6d | %-22s %-6ld | %-22s %-8zu | %-22s %-8.2f\n",
         "🎲 Noise Traders:", cfg->num_noise,
         "📈 Mid Price:", mid_price,
         "🔄 Total Trades:", stats.trade_count,
         "⏱  Elapsed Time:", elapsed_sec);
  printf("%-22s %-6d | %-22s %-6ld | %-22s %-8ld | %-22s %-8.0f\n",
         "🏦 Market Makers:", cfg->num_mm,
         "📊 Best Bid:", best_bid,
         "📦 Total Volume:", stats.volume,
         "🚀 Ticks/Second:", cfg->total_ticks / elapsed_sec);
  printf("%-22s %-6d | %-22s %-6ld | %-22s %-8s | %-22s %-8s\n",
         "🧠 Informed Traders:", cfg->num_informed,
         "📊 Best Ask:", best_ask,
         "", "",
         "", "");
  printf("%-22s %-8d | %-22s %-6ld | %-22s %-8s | %-22s %-8s\n",
         "⏱  Total Ticks:", cfg->total_ticks,
         "📏 Spread:", spread,
         "", "",
         "", "");

  printf("\n");
  printf(COLOR_GREEN "  ✓ Simulation completed successfully!\n\n" COLOR_RESET);
}

int main(int argc, char* argv[])
{
#ifdef BENCHMARK
  latency_init(&add_order_tracker);
  latency_init(&remove_order_tracker);
  latency_init(&match_order_tracker);
#endif
  // Default configuration
  config_t cfg = {
      .num_noise = 5, .num_mm = 2, .num_informed = 2, .total_ticks = 5000, .visual_mode = 1};

  // Parse command-line options
  static struct option long_options[] = {{"noise", required_argument, 0, 'n'},
                                         {"mm", required_argument, 0, 'm'},
                                         {"informed", required_argument, 0, 'i'},
                                         {"ticks", required_argument, 0, 't'},
                                         {"quiet", no_argument, 0, 'q'},
                                         {"help", no_argument, 0, 'h'},
                                         {0, 0, 0, 0}};

  int opt;
  while ((opt = getopt_long(argc, argv, "n:m:i:t:qh", long_options, NULL)) != -1)
  {
    switch (opt)
    {
    case 'n':
      cfg.num_noise = atoi(optarg);
      break;
    case 'm':
      cfg.num_mm = atoi(optarg);
      break;
    case 'i':
      cfg.num_informed = atoi(optarg);
      break;
    case 't':
      cfg.total_ticks = atoi(optarg);
      break;
    case 'q':
      cfg.visual_mode = 0;
      break;
    case 'h':
      print_usage(argv[0]);
      return 0;
    default:
      print_usage(argv[0]);
      return 1;
    }
  }

  // Validate configuration
  if (cfg.num_noise < 0 || cfg.num_mm < 0 || cfg.num_informed < 0)
  {
    fprintf(stderr, "Error: Agent counts must be non-negative\n");
    return 1;
  }
  if (cfg.num_noise + cfg.num_mm + cfg.num_informed > MAX_AGENTS)
  {
    fprintf(stderr, "Error: Total agents cannot exceed %d\n", MAX_AGENTS);
    return 1;
  }
  if (cfg.total_ticks <= 0)
  {
    fprintf(stderr, "Error: Total ticks must be positive\n");
    return 1;
  }

  // Initialize book and simulator
  order_book_t book;
  book_init(&book);
  simulator_init(&book);

  // Create agent arrays
  agent_t** noise_agents = malloc(cfg.num_noise * sizeof(agent_t*));
  agent_t** mm_agents = malloc(cfg.num_mm * sizeof(agent_t*));
  agent_t** informed_agents = malloc(cfg.num_informed * sizeof(agent_t*));

  // Add noise traders (IDs 1-N)
  for (int i = 0; i < cfg.num_noise; ++i)
  {
    noise_agents[i] = noise_trader_create(i + 1);
    simulator_add_agent(noise_agents[i]);
  }

  // Add market makers (IDs 100+)
  for (int i = 0; i < cfg.num_mm; ++i)
  {
    mm_agents[i] = market_maker_create(100 + i);
    simulator_add_agent(mm_agents[i]);
  }

  // Add informed traders (IDs 200+)
  for (int i = 0; i < cfg.num_informed; ++i)
  {
    informed_agents[i] = informed_trader_create(200 + i);
    simulator_add_agent(informed_agents[i]);
  }

  // Start timer
  clock_t start_time = clock();

  // Run simulation with progress bar
  if (cfg.visual_mode)
  {
    printf(COLOR_CYAN "\n  Running simulation...\n\n" COLOR_RESET);
    int bar_width = 40;
    int step = cfg.total_ticks / 100; // Update progress every 1%
    if (step < 1)
      step = 1;

    for (int t = 0; t < cfg.total_ticks; t += step)
    {
      int end = t + step;
      if (end > cfg.total_ticks)
        end = cfg.total_ticks;
      simulator_run(end);

      // Progress bar
      int progress = (int)((double)end / cfg.total_ticks * bar_width);
      printf("\r  [");
      for (int j = 0; j < bar_width; j++)
      {
        if (j < progress)
          printf("█");
        else
          printf("░");
      }
      printf("] %d%%", (int)((double)end / cfg.total_ticks * 100));
      fflush(stdout);
    }
    printf("\n");
  }
  else
  {
    // Quiet mode - just run
    printf("Running simulation...\n");
    simulator_run(cfg.total_ticks);
  }

  // Stop timer
  clock_t end_time = clock();
  double elapsed_sec = (double)(end_time - start_time) / CLOCKS_PER_SEC;

  // Print final book state
  print_book(&book, &cfg, cfg.total_ticks);

  // Print final stats
  print_final_stats(&book, &cfg, elapsed_sec);

#ifdef BENCHMARK
  printf("\n%-14s | %-14s | %-16s | %-12s\n", "Metric", "book_add_order", "book_remove_order", "match_order");
  printf("---------------------------------------------------------------\n");
  printf("%-14s | %-14zu | %-16zu | %-12zu\n", "Count", add_order_tracker.count, remove_order_tracker.count, match_order_tracker.count);
  printf("%-14s | %-14lu | %-16lu | %-12lu\n", "Min (ns)", add_order_tracker.min_ns, remove_order_tracker.min_ns, match_order_tracker.min_ns);
  printf("%-14s | %-14lu | %-16lu | %-12lu\n", "Max (ns)", add_order_tracker.max_ns, remove_order_tracker.max_ns, match_order_tracker.max_ns);
  printf("%-14s | %-14.2f | %-16.2f | %-12.2f\n", "Mean (ns)",
         latency_mean(&add_order_tracker), latency_mean(&remove_order_tracker), latency_mean(&match_order_tracker));
  printf("%-14s | %-14lu | %-16lu | %-12lu\n", "p50 (ns)",
         latency_percentile(&add_order_tracker, 0.50),
         latency_percentile(&remove_order_tracker, 0.50),
         latency_percentile(&match_order_tracker, 0.50));
  printf("%-14s | %-14lu | %-16lu | %-12lu\n", "p99 (ns)",
         latency_percentile(&add_order_tracker, 0.99),
         latency_percentile(&remove_order_tracker, 0.99),
         latency_percentile(&match_order_tracker, 0.99));

  // Optionally keep the detailed prints below the table:
  //   latency_print(&add_order_tracker, "book_add_order");
  //   latency_print(&remove_order_tracker, "book_remove_order");
  //   latency_print(&match_order_tracker, "match_order");

  latency_free(&add_order_tracker);
  latency_free(&remove_order_tracker);
  latency_free(&match_order_tracker);
#endif

  // Cleanup
  for (int i = 0; i < cfg.num_noise; i++)
  {
    noise_trader_destroy(noise_agents[i]);
  }
  for (int i = 0; i < cfg.num_mm; i++)
  {
    market_maker_destroy(mm_agents[i]);
  }
  for (int i = 0; i < cfg.num_informed; i++)
  {
    informed_trader_destroy(informed_agents[i]);
  }

  free(noise_agents);
  free(mm_agents);
  free(informed_agents);

  simulator_free();
  book_free(&book);

  return 0;
}