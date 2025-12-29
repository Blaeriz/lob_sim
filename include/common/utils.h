#ifndef UTILS_H
#define UTILS_H

#include "types.h"

/* ---- Math ---- */
static inline price_t max_price(price_t a, price_t b) { return a > b ? a : b; }

static inline price_t min_price(price_t a, price_t b) { return a < b ? a : b; }

#endif
