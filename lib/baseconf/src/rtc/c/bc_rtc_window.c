#include "../internal.h"
#include "../../memory/internal.h"

bc_status_t bc__rtc_window_begin(bc_avr_window_t type, bc_rtc_window_state_t *state) {
    uint8_t control;

    if (!state || type > BC_AVR_WINDOW_MODES) return BC_INVALID_ARGUMENT;
    if (!bc_rtc_window_type_known) return BC_UNSAFE_CONTEXT;

    state->irq_state = bc_interrupt_save_disable();
    state->saved_bf = bc__mem_shadow_scope_begin();
    control = bc__rtc_read_scoped(BC_RTC_CONTROL_C_INDEX);
    state->caps_led = (uint8_t)(control & BC_RTC_CONTROL_CAPS_LED);
    state->eeprom_window = (uint8_t)(control & BC_RTC_CONTROL_EEPROM_WINDOW);
    state->previous_type = bc_rtc_window_type_shadow;
    state->active = 1;

    /* RTC C bit 0 is read-only Num LED state and clears the PS/2 log on write. */
    if (state->eeprom_window) bc__rtc_write_scoped(BC_RTC_CONTROL_C_INDEX, state->caps_led);
    bc__rtc_write_scoped(BC_RTC_EXTENSION_INDEX, (uint8_t)type);
    bc_rtc_window_type_shadow = (uint8_t)type;
    return BC_OK;
}

void bc__rtc_window_end(bc_rtc_window_state_t *state) {
    if (!state || !state->active) return;
    bc__rtc_write_scoped(BC_RTC_EXTENSION_INDEX, state->previous_type);
    bc_rtc_window_type_shadow = state->previous_type;
    if (state->eeprom_window)
        bc__rtc_write_scoped(BC_RTC_CONTROL_C_INDEX,
                             (uint8_t)(state->caps_led | BC_RTC_CONTROL_EEPROM_WINDOW));
    bc__mem_shadow_scope_end(state->saved_bf);
    state->active = 0;
    bc_interrupt_restore(state->irq_state);
}
