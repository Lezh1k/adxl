#ifndef LPC824_API_DIV_H
#define LPC824_API_DIV_H

#include <stdint.h>

typedef struct {
  int32_t quot; /*!< Quotient */
  int32_t rem; /*!< Remainder */
} idiv_return_t;

typedef struct {
  uint32_t quot; /*!< Quotient */
  uint32_t rem; /*!< Reminder */
} uidiv_return_t;

typedef struct {
  int32_t (*sidiv)(int32_t numerator, int32_t denominator); /*!< Signed integer division */
  uint32_t (*uidiv)(uint32_t numerator, uint32_t denominator); /*!< Unsigned integer division */
  idiv_return_t (*sidivmod)(int32_t numerator, int32_t denominator); /*!< Signed integer division with remainder */
  uidiv_return_t (*uidivmod)(uint32_t numerator, uint32_t denominator); /*! <Unsigned integer division with remainder */
} rom_div_api_t;

#endif // LPC824_API_DIV_H
