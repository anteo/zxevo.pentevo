#include "../internal.h"
#include "../../memory/internal.h"

bc_status_t bc_avr_window_select(bc_avr_window_t type) {
    uint8_t irq_state;
    uint8_t saved_bf;
    uint8_t control;
    uint8_t caps;
    uint8_t eeprom;

    if (type > BC_AVR_WINDOW_MODES) return BC_INVALID_ARGUMENT;
    irq_state = bc_interrupt_save_disable();
    saved_bf = bc__mem_shadow_scope_begin();
    control = bc__rtc_read_scoped(BC_RTC_CONTROL_C_INDEX);
    caps = (uint8_t)(control & BC_RTC_CONTROL_CAPS_LED);
    eeprom = (uint8_t)(control & BC_RTC_CONTROL_EEPROM_WINDOW);
    if (eeprom) bc__rtc_write_scoped(BC_RTC_CONTROL_C_INDEX, caps);
    bc__rtc_write_scoped(BC_RTC_EXTENSION_INDEX, (uint8_t)type);
    bc_rtc_window_type_shadow = (uint8_t)type;
    bc_rtc_window_type_known = 1;
    if (eeprom)
        bc__rtc_write_scoped(BC_RTC_CONTROL_C_INDEX,
                             (uint8_t)(caps | BC_RTC_CONTROL_EEPROM_WINDOW));
    bc__mem_shadow_scope_end(saved_bf);
    bc_interrupt_restore(irq_state);
    return BC_OK;
}
