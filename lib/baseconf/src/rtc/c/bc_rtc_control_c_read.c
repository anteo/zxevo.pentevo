#include "../internal.h"

#define BC_RTC_CONTROL_NUM_LED 0x01u
#define BC_RTC_CONTROL_SD_WRITABLE 0x04u
#define BC_RTC_CONTROL_SD_PRESENT 0x08u
#define BC_RTC_CONTROL_TIME_UPDATED 0x10u

bc_status_t bc_rtc_control_c_read(bc_rtc_control_c_t *state) {
    uint8_t value;
    bc_status_t status;
    if (!state) return BC_INVALID_ARGUMENT;
    status = bc__rtc_read_byte(BC_RTC_CONTROL_C_INDEX, &value);
    if (status != BC_OK) return status;
    state->caps_led = (uint8_t)((value & BC_RTC_CONTROL_CAPS_LED) != 0);
    state->num_led = (uint8_t)((value & BC_RTC_CONTROL_NUM_LED) != 0);
    state->sd_writable = (uint8_t)((value & BC_RTC_CONTROL_SD_WRITABLE) != 0);
    state->sd_present = (uint8_t)((value & BC_RTC_CONTROL_SD_PRESENT) != 0);
    state->time_updated = (uint8_t)((value & BC_RTC_CONTROL_TIME_UPDATED) != 0);
    state->eeprom_window = (uint8_t)((value & BC_RTC_CONTROL_EEPROM_WINDOW) != 0);
    return BC_OK;
}
