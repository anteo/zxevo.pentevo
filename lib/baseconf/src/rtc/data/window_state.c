#include <baseconf.h>
#include "../internal.h"

/* AVR does not expose the selected extension type; this tracks library-owned selections only. */
uint8_t bc_rtc_window_type_shadow = 0;
uint8_t bc_rtc_window_type_known = 0;
