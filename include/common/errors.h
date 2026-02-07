#ifndef ERRORS_H
#define ERRORS_H

typedef enum
{
  ERR_OK = 0,
  ERR_OUT_OF_MEMORY,
  ERR_INVALID_ORDER,
  ERR_BOOK_EMPTY,
  ERR_PRICE_OUT_OF_RANGE,
  ERR_INTERNAL
} error_t;

#endif
