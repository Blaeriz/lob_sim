#include "kdb/kdb_logger.h"
#include "kdb/k.h"
#include <stdio.h>

#define BATCH_SIZE 1000

static int kdb_handle = 0;

static order_t order_batch[BATCH_SIZE];
static int order_count = 0;

static trade_t trade_batch[BATCH_SIZE];
static int trade_count = 0;

int kdb_logger_init(const char* host, int port) {
    if (!host) return 0;
    kdb_handle = khp((S)host, port);
    if (kdb_handle <= 0) {
        fprintf(stderr, "Failed to connect to kdb+ on %s:%d\n", host, port);
        kdb_handle = 0;
        return -1;
    }
    return 0;
}

static void flush_orders() {
    if (order_count == 0 || kdb_handle <= 0) return;
    
    K colNames = ktn(KS, 7);
    kS(colNames)[0] = ss((S)"ts");
    kS(colNames)[1] = ss((S)"id");
    kS(colNames)[2] = ss((S)"side");
    kS(colNames)[3] = ss((S)"type");
    kS(colNames)[4] = ss((S)"price");
    kS(colNames)[5] = ss((S)"qty");
    kS(colNames)[6] = ss((S)"agent_id");
    
    K ts = ktn(KJ, order_count);
    K id = ktn(KJ, order_count);
    K side = ktn(KI, order_count);
    K type = ktn(KI, order_count);
    K price = ktn(KJ, order_count);
    K qty = ktn(KJ, order_count);
    K agent_id = ktn(KJ, order_count);
    
    for (int i = 0; i < order_count; i++) {
        kJ(ts)[i] = (J)order_batch[i].ts;
        kJ(id)[i] = (J)order_batch[i].id;
        kI(side)[i] = (I)order_batch[i].side;
        kI(type)[i] = (I)order_batch[i].type;
        kJ(price)[i] = (J)order_batch[i].price;
        kJ(qty)[i] = (J)order_batch[i].qty;
        kJ(agent_id)[i] = (J)order_batch[i].agent_id;
    }
    
    K colValues = knk(7, ts, id, side, type, price, qty, agent_id);
    K dict = xD(colNames, colValues);
    K table = xT(dict);
    
    K tblName = ks((S)"orders");
    k(-kdb_handle, (S)"insert", tblName, table, (K)0);
    // Note: async k() consumes all its arguments (tblName, table, and their children) 
    // and returns a dummy pointer which must NOT be freed.
    
    order_count = 0;
}

void kdb_log_order(const order_t* order) {
    if (kdb_handle <= 0) return;
    order_batch[order_count++] = *order;
    if (order_count >= BATCH_SIZE) flush_orders();
}

static void flush_trades() {
    if (trade_count == 0 || kdb_handle <= 0) return;
    
    K colNames = ktn(KS, 6);
    kS(colNames)[0] = ss((S)"ts");
    kS(colNames)[1] = ss((S)"id");
    kS(colNames)[2] = ss((S)"buy_id");
    kS(colNames)[3] = ss((S)"sell_id");
    kS(colNames)[4] = ss((S)"price");
    kS(colNames)[5] = ss((S)"qty");

    K ts = ktn(KJ, trade_count);
    K id = ktn(KJ, trade_count);
    K buy_id = ktn(KJ, trade_count);
    K sell_id = ktn(KJ, trade_count);
    K price = ktn(KJ, trade_count);
    K qty = ktn(KJ, trade_count);
    
    for (int i = 0; i < trade_count; i++) {
        kJ(ts)[i] = (J)trade_batch[i].ts;
        kJ(id)[i] = (J)trade_batch[i].id;
        kJ(buy_id)[i] = (J)trade_batch[i].buy_id;
        kJ(sell_id)[i] = (J)trade_batch[i].sell_id;
        kJ(price)[i] = (J)trade_batch[i].price;
        kJ(qty)[i] = (J)trade_batch[i].qty;
    }
    
    K colValues = knk(6, ts, id, buy_id, sell_id, price, qty);
    K dict = xD(colNames, colValues);
    K table = xT(dict);
    
    K tblName = ks((S)"trades");
    k(-kdb_handle, (S)"insert", tblName, table, (K)0);
    
    trade_count = 0;
}

void kdb_log_trade(const trade_t* trade) {
    if (kdb_handle <= 0) return;
    trade_batch[trade_count++] = *trade;
    if (trade_count >= BATCH_SIZE) flush_trades();
}

void kdb_logger_teardown(void) {
    flush_orders();
    flush_trades();
}
