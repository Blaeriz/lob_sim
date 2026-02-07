# LOB Simulator - TODO

Prioritized improvements to make this project "quant dev interview ready" for Optiver/IMC.

---

## 🔥 High Priority (Do First)

### 1. ~~Add Latency Benchmarking~~ ✅ DONE
**Completed!**

Results achieved:
```
match_order:      p50: 15 ns,   p99: 38 ns
book_add_order:   p50: 52 ns,   p99: 104 ns
book_remove_order: p50: 5175 ns, p99: 7496 ns
Throughput: 72k+ ticks/sec with 9 agents
```

---

### 2. Add Deterministic Seed (`--seed`)
**Time: 30 min | Impact: High**

- [ ] Add `--seed <int>` CLI option in `main.c`
- [ ] Pass seed to all agent creation functions
- [ ] Update agent structs to accept seed parameter
- [ ] Document in README

```bash
./bin/lob_sim --seed 42   # Reproducible run
./bin/lob_sim --seed 42   # Same results
```

---

### 3. Add Microstructure Statistics
**Time: 1-2 hours | Impact: Very High**

- [ ] **Spread tracking**: min/avg/max spread over simulation
- [ ] **Order book imbalance**: `(bid_qty - ask_qty) / (bid_qty + ask_qty)`
- [ ] **Depth tracking**: total qty at top N levels
- [ ] **Fill ratio by agent type**: % of orders that got filled
- [ ] **Trade sign autocorrelation** (advanced)

---

### 4. Optimize `book_remove_order()` Performance
**Time: 1-2 hours | Impact: High**

Current bottleneck: p50 = 5175 ns (vs 52 ns for add)

- [ ] Profile to find slowest part (tree lookup vs list traversal)
- [ ] Consider order pool instead of malloc/free
- [ ] Consider storing level pointer in order for O(1) level access

---

## ⚠️ Medium Priority

### 4. Add IOC/FOK Order Types
**Time: 1 hour | Impact: Medium**

- [ ] Add `ORDER_IOC` (Immediate-or-Cancel)
- [ ] Add `ORDER_FOK` (Fill-or-Kill)
- [ ] Add `ORDER_MARKET`
- [ ] Update matching engine to handle new types

---

### 5. Add Memory Layout Notes to README
**Time: 30 min | Impact: Free signal**

- [ ] Document struct sizes and alignment
- [ ] Mention cache line considerations
- [ ] Discuss allocation strategy (malloc vs pool)

---

### 6. Add Replay Mode (`--replay`)
**Time: 2 hours | Impact: Medium**

- [ ] Add `--replay <file.csv>` CLI option
- [ ] Parse CSV format: `timestamp,side,price,qty,type`
- [ ] Replay orders in timestamp order

---

## 💡 Lower Priority (Nice to Have)

### 7. Event-Driven Engine (Branch)
**Time: 3-4 hours**

- [ ] Create `branch: event-driven`
- [ ] Replace tick-based loop with event queue
- [ ] Priority queue by timestamp

---

### 8. Multi-Thread Experiment (Branch)
**Time: 3-4 hours**

- [ ] Create `branch: mt-experimental`
- [ ] Lock-free queue for incoming orders
- [ ] Matching engine on dedicated thread

---

### 9. ~~Benchmark Harness~~ ✅ DONE
**Completed!**

- [x] Created `include/bench/latency.h`
- [x] Created `src/bench/latency.c`
- [x] Compile with `-DBENCHMARK` flag for latency tracking
- [x] Integrated into `book.c` and `matching.c`

---

### 10. Design Document
**Time: 1-2 hours**

- [ ] Create `DESIGN.md`
- [ ] Explain data structure choices
- [ ] Discuss tradeoffs (RB-tree vs heap, hash map vs tree)

---

## ✅ Completed

- [x] Core order book engine
- [x] Price-time priority matching
- [x] Red-black tree for price levels
- [x] Hash map for O(1) order lookup
- [x] Noise traders
- [x] Market makers with inventory
- [x] Informed traders with fair value
- [x] Trade statistics (count, volume)
- [x] CLI with options
- [x] Visual terminal display
- [x] README documentation
- [x] Heap-allocated orders (fixed dangling pointer bug)
- [x] Matching engine integration in book_add_order
- [x] Simulator state management
- [x] `simulator_init()`, `simulator_add_agent()`, `simulator_run()`, `simulator_free()`
- [x] `stats.c`, `clock.c`, `event.c` (stub)
- [x] **Latency benchmarking** (p50/p99/mean for match, add, remove)
- [x] **Benchmark harness** (`include/bench/latency.h`, `src/bench/latency.c`)

---

## 🎯 Resume Goal

With latency benchmarking complete, you can now claim:

> Implemented a high-performance limit order book matching engine in C using red-black trees and O(1) order indexing. **Median match latency: 15 ns, p99: 38 ns. Order insertion: p50 52 ns, p99 104 ns.** Simulated heterogeneous trading agents including noise traders, market makers, and informed traders.

---

## Order of Attack

1. ~~Latency benchmarking~~ ✅ DONE
2. `--seed` (30 min) — quick win
3. Microstructure stats (1-2 hours) — very quant-relevant
4. Optimize `book_remove_order` (1-2 hours) — performance win
5. Memory notes in README (30 min) — free signal

**Remaining: ~4-5 hours for HIGH + MED priority items**
