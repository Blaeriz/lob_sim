# LOB Simulator - TODO

Prioritized improvements to make this project "quant dev interview ready" for Optiver/IMC.

---

## 🔥 High Priority (Do First)

### 1. Add Latency Benchmarking
**Time: 1-2 hours | Impact: Huge**

- [ ] Add `clock_gettime(CLOCK_MONOTONIC)` to measure nanoseconds per match
- [ ] Track min/median/p99/max latency
- [ ] Add latency histogram (buckets: <100ns, <500ns, <1µs, <10µs, >10µs)
- [ ] Output stats at end of simulation

```c
struct timespec start, end;
clock_gettime(CLOCK_MONOTONIC, &start);
match_order(...);
clock_gettime(CLOCK_MONOTONIC, &end);
uint64_t latency_ns = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
```

**Target metrics:**
```
Median match latency: ~200 ns
p99 latency: ~800 ns
Throughput: 2M+ order events/sec (-O3)
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

### 9. Benchmark Harness
**Time: 1 hour**

- [ ] Create `bench/` directory
- [ ] `bench/synthetic_flow.c` — random order stream
- [ ] `bench/latency_test.c` — measure match latency
- [ ] Makefile target: `make bench`

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

---

## 🎯 Resume Goal

After completing HIGH priority items:

> Implemented a high-performance limit order book matching engine in C using red-black trees and O(1) order indexing, achieving **~2M events/sec** with **median match latency of ~200ns**. Simulated heterogeneous trading agents with microstructure metrics including spread, imbalance, and fill ratios.

---

## Order of Attack

1. `--seed` (30 min) — quick win
2. Latency benchmarking (1-2 hours) — biggest impact
3. Microstructure stats (1-2 hours) — very quant-relevant
4. Memory notes in README (30 min) — free signal
5. IOC/FOK orders (1 hour) — shows completeness

**Total: ~5-6 hours for HIGH + MED priority items**
