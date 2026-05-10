#ifndef KDB_LOGGER_H
#define KDB_LOGGER_H

#include "core/order.h"
#include "core/trade.h"

// Initialize kdb+ connection. Returns 0 on success.
int kdb_logger_init(const char* host, int port);

// Log an order creation
void kdb_log_order(const order_t* order);

// Log a matched trade
void kdb_log_trade(const trade_t* trade);

// Flush batched records
void kdb_logger_teardown(void);

#endif
