#ifndef BASECONF_PRINTF_INTERNAL_H
#define BASECONF_PRINTF_INTERNAL_H

#include <stdint.h>

/* Write the digits backwards from end into a five-byte buffer. */
uint8_t bc__printf_unsigned_string(uint16_t value, uint8_t radix, uint8_t upper, char *end);

#endif
