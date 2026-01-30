# LOB Simulator - Development Roadmap

## Overview

This document outlines the remaining work needed to complete the Limit Order Book Simulator. The project has a solid foundation with core order book mechanics implemented, and the simulator backbone is now complete.

---

## 1. Simulator Implementation ✅ COMPLETE

**File**: `src/sim/simulator.c`

### 1.1 Simulator State Management ✅

- [x] Pointer to the order book
- [x] Array/list of registered agents
- [x] Current simulation time
- [x] Time step (`dt`) for simulation loop
- [ ] Event queue (priority queue ordered by timestamp) — *deferred for event-driven version*
- [ ] Trade history buffer — *optional enhancement*
- [ ] Statistics collector — *see section 4*

```c
// Implemented structure
typedef struct simulator_t {
    order_book_t *book;
    agent_t **agents;
    size_t agent_count;
    size_t agent_capacity;
    timestamp_t current_time;
    timestamp_t dt;
} simulator_t;
```

### 1.2 `simulator_init()` ✅

- [x] Allocate and initialize simulator state
- [x] Store reference to order book
- [x] Initialize agent array (dynamic, starts at capacity 10)
- [x] Reset time to 0
- [x] Set default dt = 1
- [ ] Initialize statistics module — *see section 4*

### 1.3 `simulator_add_agent()` ✅

- [x] Add agent pointer to internal agent list
- [x] Grow array if needed (doubles capacity via realloc)
- [x] Validate agent is not NULL
- [x] Safe realloc pattern with temp pointer

### 1.4 `simulator_run()` ✅

Implemented **time-stepped simulation**:
- [x] Loop while `current_time < end_time`
- [x] Call each agent's `step(agent, book, current_time)`
- [x] Progress printing every 1000 ticks
- [x] Time advancement by `dt`

**Future enhancement (not implemented):**
- [ ] Event-driven simulation with priority queue

### 1.5 `simulator_free()` ✅

- [x] Free agents array
- [x] Reset all pointers to NULL
- [x] Reset counts to 0

### 1.6 Order Submission Interface (Future)

Agents currently submit orders directly via `book_add_order()`. Optional enhancement:
- [ ] `simulator_submit_order(order_t *order)` — centralized order submission
- [ ] `simulator_cancel_order(order_id_t id)` — centralized cancellation
- [ ] Global order ID generation

---

## 2. Agent Behaviors (In Progress)

All agents currently have empty `step()` functions.

### 2.1 Noise Trader (`src/agents/noise_trader.c`) — IN PROGRESS

**Purpose**: Generates random market activity (liquidity provider/taker)

**State structure**: ✅ COMPLETE
```c
typedef struct {
    order_id_t next_order_id;    // counter for unique IDs
    double act_probability;       // chance of acting per tick (default: 0.1)
    price_t price_range;          // deviation from mid-price (default: 10)
    qty_t min_qty;                // minimum order size (default: 1)
    qty_t max_qty;                // maximum order size (default: 10)
    unsigned int rng_seed;        // for rand_r() reproducibility
} noise_trader_state_t;
```

**`noise_trader_create()`**: ✅ COMPLETE
- [x] Allocate agent and state
- [x] Initialize all state fields with defaults
- [x] Proper error handling and memory cleanup

**`noise_step()`**: ⏳ TODO
- [ ] Decide whether to act (probability roll)
- [ ] Randomly choose BUY or SELL
- [ ] Get mid-price from book (need helper function)
- [ ] Calculate price with random offset
- [ ] Calculate random quantity
- [ ] Create and submit order

### 2.2 Market Maker (`src/agents/market_maker.c`)

**Purpose**: Provides liquidity by quoting bid/ask prices

- [ ] Define state structure:
  ```c
  typedef struct {
      price_t spread;            // half-spread from mid
      qty_t quote_size;          // size at each level
      int inventory;             // current position
      int max_inventory;         // risk limit
      order_id_t bid_id;         // track active orders
      order_id_t ask_id;
  } market_maker_state_t;
  ```
- [ ] Implement `mm_step()`:
  - [ ] Cancel existing quotes
  - [ ] Calculate mid-price from book
  - [ ] Adjust spread based on inventory (skew quotes)
  - [ ] Post new bid and ask orders
  - [ ] Track fills to update inventory

### 2.3 Informed Trader (`src/agents/informed_trader.c`)

**Purpose**: Trades based on private information about "true" value

- [ ] Define state structure:
  ```c
  typedef struct {
      price_t true_value;        // private signal
      double aggressiveness;     // how fast to trade
      int target_position;
      int current_position;
  } informed_trader_state_t;
  ```
- [ ] Implement `informed_step()`:
  - [ ] Compare true_value to current best bid/ask
  - [ ] If true_value > ask: buy aggressively
  - [ ] If true_value < bid: sell aggressively
  - [ ] Limit position size

---

## 3. Event System Enhancement (Medium Priority)

**Files**: `include/sim/event.h`, `src/sim/event.c`

### 3.1 Event Queue Implementation

- [ ] Implement priority queue (min-heap by timestamp)
- [ ] Functions needed:
  ```c
  void event_queue_init(event_queue_t *q);
  void event_queue_push(event_queue_t *q, event_t *event);
  event_t *event_queue_pop(event_queue_t *q);
  int event_queue_empty(event_queue_t *q);
  void event_queue_free(event_queue_t *q);
  ```

### 3.2 Event Types

Expand `event_type_t` if needed:
- [ ] `EVENT_AGENT_WAKEUP` — agent scheduled action
- [ ] `EVENT_ORDER_ARRIVAL` — new order
- [ ] `EVENT_ORDER_CANCEL` — cancellation request
- [ ] `EVENT_TRADE` — trade execution (for logging)

---

## 4. Statistics & Output (Medium Priority)

**Files**: `include/sim/stats.h`, `src/sim/stats.c`

### 4.1 Metrics to Track

- [ ] Total number of orders submitted
- [ ] Total number of trades executed
- [ ] Total volume traded
- [ ] Bid-ask spread over time
- [ ] Price time series (trade prices)
- [ ] Order book depth snapshots

### 4.2 Statistics Functions

- [ ] `stats_init()` — initialize counters
- [ ] `stats_on_order(order_t *order)` — record order
- [ ] `stats_on_trade(trade_t *trade)` — record trade
- [ ] `stats_on_cancel(order_id_t id)` — record cancel
- [ ] `stats_print_summary()` — output final stats
- [ ] `stats_export_csv(const char *filename)` — export data

---

## 5. Configuration System (Low Priority)

**File**: `data/params.json` (currently empty)

### 5.1 JSON Parser

- [ ] Add a lightweight JSON parser (e.g., cJSON, or write minimal parser)
- [ ] Create `config.c` to load parameters

### 5.2 Configuration Parameters

```json
{
  "simulation": {
    "duration": 10000,
    "seed": 42
  },
  "agents": {
    "noise_traders": {
      "count": 10,
      "order_probability": 0.1,
      "price_range": 5,
      "max_qty": 100
    },
    "market_makers": {
      "count": 2,
      "spread": 2,
      "quote_size": 50
    },
    "informed_traders": {
      "count": 1,
      "aggressiveness": 0.5
    }
  },
  "book": {
    "tick_size": 1,
    "initial_mid_price": 1000
  }
}
```

---

## 6. Testing (Ongoing)

### 6.1 Existing Tests

- [x] `tests/book_test.c` — order book operations
- [x] `tests/matching_test.c` — matching engine
- [x] `tests/order_map_test.c` — hash map
- [x] `tests/price_tree_test.c` — red-black tree

### 6.2 New Tests Needed

- [ ] `tests/simulator_test.c` — basic simulation flow
- [ ] `tests/agent_test.c` — agent behavior validation
- [ ] `tests/integration_test.c` — full end-to-end test
- [ ] `tests/stress_test.c` — performance/memory testing

---

## 7. Code Quality & Documentation (Low Priority)

### 7.1 Documentation

- [ ] Add README.md with build/run instructions
- [ ] Document public API in header files
- [ ] Add inline comments for complex algorithms

### 7.2 Error Handling

- [ ] Define error codes in `include/common/errors.h`
- [ ] Add proper error returns throughout codebase
- [ ] Consider logging framework

### 7.3 Memory Management

- [ ] Audit for memory leaks (use Valgrind)
- [ ] Add cleanup functions (`simulator_free()`, `agent_destroy()`)
- [ ] Document ownership semantics

---

## Suggested Order of Implementation

| Phase | Task | Effort | Status |
|-------|------|--------|--------|
| 1 | Simulator state + basic `run()` loop | 2-3 hours | ✅ DONE |
| 2 | Noise trader state + create | 1 hour | ✅ DONE |
| 3 | Noise trader step function | 1-2 hours | ⏳ IN PROGRESS |
| 4 | Helper: `book_best_bid/ask` functions | 30 min | TODO |
| 5 | Basic statistics output | 1 hour | TODO |
| 6 | Market maker implementation | 2-3 hours | TODO |
| 7 | Informed trader implementation | 1-2 hours | TODO |
| 8 | Configuration loading | 2-3 hours | TODO |
| 9 | Event-driven simulation (optional) | 3-4 hours | TODO |

---

## Notes

- The core order book (`book.c`, `matching.c`, `price_tree.c`) is **complete and tested**
- The simulator backbone is **complete** (time-stepped approach)
- Noise trader state and creation is **complete**, step function in progress
- Next: implement `noise_step()` and helper functions for getting best bid/ask
- Consider adding a random number generator utility for reproducible simulations

---

## Recent Progress (Updated: Jan 30, 2026)

### Completed Today:
1. **Simulator Implementation** — full time-stepped simulator with:
   - `simulator_init()` — setup with dynamic agent array
   - `simulator_add_agent()` — with automatic capacity growth
   - `simulator_run()` — main loop calling agent steps
   - `simulator_free()` — proper cleanup

2. **Noise Trader Foundation**:
   - State structure with all needed fields
   - `noise_trader_create()` with proper memory management

### Next Steps:
1. Implement `noise_step()` — the actual trading logic
2. Add `book_best_bid()` / `book_best_ask()` helper functions
3. Test end-to-end simulation flow
