#include <baseconf.h>
#include <baseconf/rtc.h>
#include "../internal.h"
#include "../../internal.h"
#include "../../memory/internal.h"

bc_status_t bc_eeprom_window_set(uint8_t enabled) {
    uint8_t irq_state;
    uint8_t saved_bf;
    uint8_t control;
    uint8_t desired;

    if (!bc_rtc_window_type_known) return BC_UNSAFE_CONTEXT;

    irq_state = bc_interrupt_save_disable();
    saved_bf = bc__mem_shadow_scope_begin();
    control = bc__rtc_read_scoped(0x0cu);
    desired = (uint8_t)((control & 0x02u) | (enabled ? 0x80u : 0u));
    if ((control & 0x80u) != (desired & 0x80u)) {
        bc__rtc_write_scoped(0x0cu, desired);
        if (!enabled) bc__rtc_write_scoped(0xf0u, bc_rtc_window_type_shadow);
    }
    bc__mem_shadow_scope_end(saved_bf);
    bc_interrupt_restore(irq_state);
    return BC_OK;
}
