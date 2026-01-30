#define _DEFAULT_SOURCE  // for usleep on Linux

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#define CLEAR_SCREEN() system("cls")
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>
#define CLEAR_SCREEN() printf("\033[2J\033[H")
#define SLEEP_MS(ms) usleep((ms) * 1000)
#endif

#include "core/book.h"
#include "core/matching.h"
#include "core/level_ops.h"

// Colors (ANSI escape codes)
#define COLOR_RESET   "\033[0m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_RED     "\033[31m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_BOLD    "\033[1m"

// Config
#define MAX_DISPLAY_LEVELS 8
#define ANIMATION_DELAY_MS 800

// Global order ID counter
static order_id_t next_order_id = 1;

// Helper to create an order
static order_t *make_order(side_t side, price_t price, qty_t qty) {
  order_t *o = (order_t *)malloc(sizeof(order_t));
  o->id = next_order_id++;
  o->side = side;
  o->type = ORDER_LIMIT;
  o->price = price;
  o->qty = qty;
  o->ts = 0;
  return o;
}

// Collect prices from tree into array (for display)
typedef struct {
  price_t price;
  qty_t total_qty;
  int order_count;
} level_info_t;

static void collect_level(price_level_t *lvl, level_info_t *info) {
  info->price = lvl->price;
  info->total_qty = lvl->total_qty;
  
  // Count orders in level
  info->order_count = 0;
  order_node_t *node = lvl->head;
  while (node) {
    info->order_count++;
    node = node->next;
  }
}

// Print the order book
static void print_book(order_book_t *book, const char *message) {
  CLEAR_SCREEN();
  
  printf("\n");
  printf(COLOR_BOLD "  ╔════════════════════════════════════════════════════════╗\n" COLOR_RESET);
  printf(COLOR_BOLD "  ║            📊 LIMIT ORDER BOOK SIMULATOR 📊            ║\n" COLOR_RESET);
  printf(COLOR_BOLD "  ╚════════════════════════════════════════════════════════╝\n" COLOR_RESET);
  printf("\n");
  
  // Collect bid levels (we need to traverse to get them)
  level_info_t bids[MAX_DISPLAY_LEVELS] = {0};
  level_info_t asks[MAX_DISPLAY_LEVELS] = {0};
  int num_bids = 0;
  int num_asks = 0;
  
  // Get asks (ascending from min)
  price_level_t *ask = pt_min(&book->asks);
  while (ask && num_asks < MAX_DISPLAY_LEVELS) {
    collect_level(ask, &asks[num_asks]);
    num_asks++;
    // Find next ask (simple linear scan - not efficient but ok for demo)
    price_level_t *next_ask = NULL;
    price_t min_price = INT64_MAX;
    // We need to iterate - using a simple approach
    // For demo purposes, we'll just use pt_find with incrementing prices
    for (price_t p = ask->price + 1; p < ask->price + 1000; p++) {
      price_level_t *found = pt_find(&book->asks, p);
      if (found && found->price < min_price) {
        min_price = found->price;
        next_ask = found;
        break;
      }
    }
    ask = next_ask;
  }
  
  // Get bids (descending from max)
  price_level_t *bid = pt_max(&book->bids);
  while (bid && num_bids < MAX_DISPLAY_LEVELS) {
    collect_level(bid, &bids[num_bids]);
    num_bids++;
    price_level_t *next_bid = NULL;
    for (price_t p = bid->price - 1; p > bid->price - 1000; p--) {
      price_level_t *found = pt_find(&book->bids, p);
      if (found) {
        next_bid = found;
        break;
      }
    }
    bid = next_bid;
  }
  
  // Print header
  printf("       " COLOR_BOLD "%-20s      %-20s\n" COLOR_RESET, "BIDS (Buyers)", "ASKS (Sellers)");
  printf("       ─────────────────────────────────────────────────\n");
  
  // Print levels (asks first, reversed, then bids)
  // Print asks from highest to lowest (so lowest is closest to spread)
  for (int i = num_asks - 1; i >= 0; i--) {
    printf("       %20s      " COLOR_RED "%6lld @ %-6lld" COLOR_RESET " [%d orders]\n",
           "", (long long)asks[i].total_qty, (long long)asks[i].price, asks[i].order_count);
  }
  
  // Print spread
  price_level_t *best_bid = pt_max(&book->bids);
  price_level_t *best_ask = pt_min(&book->asks);
  
  printf("       ─────────────────────────────────────────────────\n");
  if (best_bid && best_ask) {
    price_t spread = best_ask->price - best_bid->price;
    printf(COLOR_YELLOW "       %20s  ⬆ SPREAD: %lld ⬇\n" COLOR_RESET, "", (long long)spread);
  } else {
    printf(COLOR_YELLOW "       %20s  ⬆ SPREAD: --- ⬇\n" COLOR_RESET, "");
  }
  printf("       ─────────────────────────────────────────────────\n");
  
  // Print bids
  for (int i = 0; i < num_bids; i++) {
    printf("       " COLOR_GREEN "%6lld @ %-6lld" COLOR_RESET " [%d orders]\n",
           (long long)bids[i].total_qty, (long long)bids[i].price, bids[i].order_count);
  }
  
  // Pad if not enough levels
  for (int i = num_bids; i < 3; i++) {
    printf("\n");
  }
  
  printf("\n       ─────────────────────────────────────────────────\n");
  
  // Print stats
  printf(COLOR_CYAN "       📈 Total Orders: %zu\n" COLOR_RESET, book->orders.count);
  
  // Print message
  if (message && strlen(message) > 0) {
    printf("\n" COLOR_BOLD "       💬 %s\n" COLOR_RESET, message);
  }
  
  printf("\n");
}

// Print trade execution
static void print_trade(trade_t *trade) {
  printf(COLOR_YELLOW "\n       ⚡ TRADE EXECUTED: %lld @ %lld (buy #%llu ↔ sell #%llu)\n" COLOR_RESET,
         (long long)trade->qty, (long long)trade->price,
         (unsigned long long)trade->buy_id, (unsigned long long)trade->sell_id);
}

int main(void) {
  order_book_t book;
  book_init(&book);
  
  trade_t trades[10];
  char msg[256];
  
  // Introduction
  print_book(&book, "Welcome! Starting order book simulation...");
  SLEEP_MS(ANIMATION_DELAY_MS * 2);
  
  // === PHASE 1: Build initial book ===
  
  // Add some sell orders (asks)
  order_t *s1 = make_order(SIDE_SELL, 105, 20);
  book_add_order(&book, s1);
  snprintf(msg, sizeof(msg), "SELL order: %lld @ %lld", (long long)s1->qty, (long long)s1->price);
  print_book(&book, msg);
  SLEEP_MS(ANIMATION_DELAY_MS);
  
  order_t *s2 = make_order(SIDE_SELL, 104, 15);
  book_add_order(&book, s2);
  snprintf(msg, sizeof(msg), "SELL order: %lld @ %lld", (long long)s2->qty, (long long)s2->price);
  print_book(&book, msg);
  SLEEP_MS(ANIMATION_DELAY_MS);
  
  order_t *s3 = make_order(SIDE_SELL, 103, 10);
  book_add_order(&book, s3);
  snprintf(msg, sizeof(msg), "SELL order: %lld @ %lld", (long long)s3->qty, (long long)s3->price);
  print_book(&book, msg);
  SLEEP_MS(ANIMATION_DELAY_MS);
  
  // Add some buy orders (bids)
  order_t *b1 = make_order(SIDE_BUY, 100, 25);
  book_add_order(&book, b1);
  snprintf(msg, sizeof(msg), "BUY order: %lld @ %lld", (long long)b1->qty, (long long)b1->price);
  print_book(&book, msg);
  SLEEP_MS(ANIMATION_DELAY_MS);
  
  order_t *b2 = make_order(SIDE_BUY, 99, 30);
  book_add_order(&book, b2);
  snprintf(msg, sizeof(msg), "BUY order: %lld @ %lld", (long long)b2->qty, (long long)b2->price);
  print_book(&book, msg);
  SLEEP_MS(ANIMATION_DELAY_MS);
  
  order_t *b3 = make_order(SIDE_BUY, 98, 20);
  book_add_order(&book, b3);
  snprintf(msg, sizeof(msg), "BUY order: %lld @ %lld", (long long)b3->qty, (long long)b3->price);
  print_book(&book, msg);
  SLEEP_MS(ANIMATION_DELAY_MS);
  
  // Add another order at existing level
  order_t *b4 = make_order(SIDE_BUY, 100, 15);
  book_add_order(&book, b4);
  snprintf(msg, sizeof(msg), "BUY order: %lld @ %lld (adding to existing level)", (long long)b4->qty, (long long)b4->price);
  print_book(&book, msg);
  SLEEP_MS(ANIMATION_DELAY_MS);
  
  print_book(&book, "Book is ready. Now let's see some action! 🚀");
  SLEEP_MS(ANIMATION_DELAY_MS * 2);
  
  // === PHASE 2: Aggressive orders that cross ===
  
  // Aggressive buy that crosses the spread
  print_book(&book, "Incoming AGGRESSIVE BUY @ 103 for 15 units...");
  SLEEP_MS(ANIMATION_DELAY_MS);
  
  order_t *aggr_buy = make_order(SIDE_BUY, 103, 15);
  qty_t num_trades = match_order(&book, aggr_buy, trades, 10);
  
  for (qty_t i = 0; i < num_trades; i++) {
    print_trade(&trades[i]);
    SLEEP_MS(ANIMATION_DELAY_MS / 2);
  }
  
  // Add remaining qty to book if any
  if (aggr_buy->qty > 0) {
    book_add_order(&book, aggr_buy);
    snprintf(msg, sizeof(msg), "Remaining %lld units added to book @ %lld", 
             (long long)aggr_buy->qty, (long long)aggr_buy->price);
  } else {
    snprintf(msg, sizeof(msg), "Order fully filled! %lld trade(s) executed", (long long)num_trades);
  }
  print_book(&book, msg);
  SLEEP_MS(ANIMATION_DELAY_MS * 2);
  
  // Aggressive sell that crosses
  print_book(&book, "Incoming AGGRESSIVE SELL @ 99 for 50 units...");
  SLEEP_MS(ANIMATION_DELAY_MS);
  
  order_t *aggr_sell = make_order(SIDE_SELL, 99, 50);
  num_trades = match_order(&book, aggr_sell, trades, 10);
  
  for (qty_t i = 0; i < num_trades; i++) {
    print_trade(&trades[i]);
    SLEEP_MS(ANIMATION_DELAY_MS / 2);
  }
  
  if (aggr_sell->qty > 0) {
    book_add_order(&book, aggr_sell);
    snprintf(msg, sizeof(msg), "Remaining %lld units added to book @ %lld",
             (long long)aggr_sell->qty, (long long)aggr_sell->price);
  } else {
    snprintf(msg, sizeof(msg), "Order fully filled! %lld trade(s) executed", (long long)num_trades);
  }
  print_book(&book, msg);
  SLEEP_MS(ANIMATION_DELAY_MS * 2);
  
  // === PHASE 3: Order cancellation ===
  
  print_book(&book, "Now let's cancel some orders...");
  SLEEP_MS(ANIMATION_DELAY_MS);
  
  // Cancel an order
  order_id_t cancel_id = b2->id;
  snprintf(msg, sizeof(msg), "Cancelling order #%llu...", (unsigned long long)cancel_id);
  print_book(&book, msg);
  SLEEP_MS(ANIMATION_DELAY_MS);
  
  book_remove_order(&book, cancel_id);
  snprintf(msg, sizeof(msg), "Order #%llu cancelled! ❌", (unsigned long long)cancel_id);
  print_book(&book, msg);
  SLEEP_MS(ANIMATION_DELAY_MS * 2);
  
  // === FINALE ===
  
  print_book(&book, "🎉 Simulation complete! Press Ctrl+C to exit.");
  
  // Cleanup
  // Note: In a real app, you'd track and free all orders properly
  // For demo purposes, we just free the book
  book_free(&book);
  
  return 0;
}
