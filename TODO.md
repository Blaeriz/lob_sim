# LOB Simulator - Development Roadmap

## Overview

This document outlines the remaining work needed to complete the Limit Order Book Simulator. The project has a solid foundation with core order book mechanics implemented, but the simulation layer and agent behaviors need development.

---

## 1. Simulator Implementation (High Priority)

**File**: `src/sim/simulator.c`

The simulator is currently stubbed out. This is the backbone that drives the entire simulation.

### 1.1 Simulator State Management

Create internal state to track:
- [ ] Pointer to the order book
- [ ] Array/list of registered agents
- [ ] Current simulation time
- [ ] Event queue (priority queue ordered by timestamp)
- [ ] Trade history buffer
- [ ] Statistics collector

```c
// Suggested internal state structure
typedef struct {
    order_book_t *book;
    agent_t **agents;
    size_t agent_count;
    size_t agent_capacity;
    timestamp_t current_time;
    // event_queue_t events;  // if using event-driven
} simulator_state_t;
```

### 1.2 Implement `simulator_init()`

- [ ] Allocate and initialize simulator state
- [ ] Store reference to order book
- [ ] Initialize agent array
- [ ] Reset time to 0
- [ ] Initialize statistics module

### 1.3 Implement `simulator_add_agent()`

- [ ] Add agent pointer to internal agent list
- [ ] Grow array if needed (dynamic resizing)
- [ ] Validate agent is not NULL

### 1.4 Implement `simulator_run()`

Two design options:

**Option A: Time-stepped simulation (simpler)**
```c
for (timestamp_t t = 0; t < end_time; t++) {
    for (size_t i = 0; i < agent_count; i++) {
        agents[i]->step(agents[i], t);
    }
}
```

**Option B: Event-driven simulation (more realistic)**
- [ ] Use a priority queue of events
- [ ] Process events in timestamp order
- [ ] Agents schedule their next action as events

### 1.5 Order Submission Interface

Agents need a way to submit orders. Add functions:
- [ ] `simulator_submit_order(order_t *order)` — runs matching, adds remainder to book
- [ ] `simulator_cancel_order(order_id_t id)` — removes order from book
- [ ] Generate unique order IDs internally

---

## 2. Agent Behaviors (Medium Priority)

All agents currently have empty `step()` functions.

### 2.1 Noise Trader (`src/agents/noise_trader.c`)

**Purpose**: Generates random market activity (liquidity provider/taker)

- [ ] Define state structure:
  ```c
  typedef struct {
      double order_probability;  // chance of submitting per step
      price_t price_range;       // deviation from mid-price
      qty_t max_qty;
  } noise_trader_state_t;
  ```
- [ ] Implement `noise_step()`:
  - [ ] With some probability, generate a random order
  - [ ] Randomly choose BUY or SELL
  - [ ] Pick price: mid-price ± random offset
  - [ ] Pick quantity: random within range
  - [ ] Submit order via simulator

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

| Phase | Task | Effort |
|-------|------|--------|
| 1 | Simulator state + basic `run()` loop | 2-3 hours |
| 2 | `simulator_submit_order()` integration | 1-2 hours |
| 3 | Noise trader implementation | 1-2 hours |
| 4 | Basic statistics output | 1 hour |
| 5 | Market maker implementation | 2-3 hours |
| 6 | Informed trader implementation | 1-2 hours |
| 7 | Configuration loading | 2-3 hours |
| 8 | Event-driven simulation (optional) | 3-4 hours |

---

## Notes

- The core order book (`book.c`, `matching.c`, `price_tree.c`) is **complete and tested**
- Focus on getting a minimal working simulation first, then iterate
- Consider adding a random number generator utility for reproducible simulations
