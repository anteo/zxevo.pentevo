#include <baseconf/printf.h>

int bc_printf(const char *format, ...) {
    va_list args;
    int count;
    va_start(args, format);
    count = bc_vprintf(format, args);
    va_end(args);
    return count;
}
