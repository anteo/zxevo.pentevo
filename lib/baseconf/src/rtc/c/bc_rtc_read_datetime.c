#include <baseconf.h>
#include <baseconf/rtc.h>
#include "../internal.h"
#include "../../memory/internal.h"

bc_status_t bc_rtc_read_datetime(bc_datetime_t *datetime, bc_rtc_format_t format) {
    uint8_t *dst;
    uint8_t i;
    uint8_t irq_state, bf, control_b;
    if (!datetime || format > BC_RTC_BCD) return BC_INVALID_ARGUMENT;
    irq_state = bc_interrupt_save_disable();
    bf = bc__mem_shadow_scope_begin();
    control_b = bc__rtc_read_scoped(BC_RTC_CONTROL_B);
    bc__rtc_write_scoped(BC_RTC_CONTROL_B, format == BC_RTC_BINARY
                                               ? (uint8_t)(control_b | 4u)
                                               : (uint8_t)(control_b & (uint8_t)~4u));
    dst = (uint8_t *)datetime;
    for (i = 0; i < 7; i++) {
        uint8_t index = (uint8_t)(i == 0 ? 0 : i == 1 ? 2 : i == 2 ? 4 : i + 3);
        dst[i] = bc__rtc_read_scoped(index);
    }
    bc__rtc_write_scoped(BC_RTC_CONTROL_B, control_b);
    bc__mem_shadow_scope_end(bf);
    bc_interrupt_restore(irq_state);
    return BC_OK;
}
