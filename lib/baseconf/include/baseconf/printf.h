#ifndef BASECONF_PRINTF_H
#define BASECONF_PRINTF_H

#include <stdarg.h>

/* Format to the active console with bc_console_putc control codes and wrapping.
 * Output does not scroll.
 * Supports %c, %s, %d, %i, %u, %x, %X and %%, optional '-' then '0', and a
 * decimal field width (clamped to 255). Integers are 16-bit; precision, length
 * modifiers and floating point are unsupported. NULL %s is an empty string.
 * Unknown conversions print '%' and the conversion byte without consuming an
 * argument. Returns the number of characters passed to the console, or -1 for
 * NULL format or an output error; output may already be partially written. */
int bc_printf(const char *format, ...);
/* va_list form of bc_printf, with the same format rules and return values. */
int bc_vprintf(const char *format, va_list args);

#endif
