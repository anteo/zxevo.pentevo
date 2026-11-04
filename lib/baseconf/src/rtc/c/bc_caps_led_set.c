#include "../internal.h"
#include "../../memory/internal.h"

void bc_caps_led_set(uint8_t enabled) {
    uint8_t irq_state;
    uint8_t saved_bf;
    uint8_t control;
    uint8_t desired;
    irq_state = bc_interrupt_save_disable();
    saved_bf = bc__mem_shadow_scope_begin();
    control = bc__rtc_read_scoped(BC_RTC_CONTROL_C_INDEX);
    desired = (uint8_t)((control & BC_RTC_CONTROL_EEPROM_WINDOW) |
                        (enabled ? BC_RTC_CONTROL_CAPS_LED : 0u));
    if ((control & BC_RTC_CONTROL_CAPS_LED) != (desired & BC_RTC_CONTROL_CAPS_LED))
        bc__rtc_write_scoped(BC_RTC_CONTROL_C_INDEX, desired);
    bc__mem_shadow_scope_end(saved_bf);
    bc_interrupt_restore(irq_state);
}
