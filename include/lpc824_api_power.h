#ifndef LPC824_API_POWER_H
#define LPC824_API_POWER_H

#include <stdint.h>

typedef struct _pwrd {
  void (*set_pll)(uint32_t cmd[], uint32_t resp[]);
  void (*set_power)(uint32_t cmd[], uint32_t resp[]);
} pwrd_api_t;

#endif // LPC824_API_POWER_H
