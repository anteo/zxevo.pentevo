#ifndef BASECONF_RTC_INTERNAL_H
#define BASECONF_RTC_INTERNAL_H

#include <baseconf.h>
#include <baseconf/rtc.h>
#include "../internal.h"

#define BC_RTC_CONTROL_C_INDEX 0x0cu
#define BC_RTC_EXTENSION_INDEX 0xf0u
#define BC_RTC_CONTROL_CAPS_LED 0x02u
#define BC_RTC_CONTROL_EEPROM_WINDOW 0x80u

bc_status_t bc__rtc_read_byte(uint8_t index, uint8_t *value);
bc_status_t bc__rtc_write_byte(uint8_t index, uint8_t value);
uint8_t bc__rtc_read_scoped(uint8_t index);
void bc__rtc_write_scoped(uint8_t index, uint8_t value);
typedef struct {
    uint8_t irq_state;
    uint8_t saved_bf;
    uint8_t caps_led;
    uint8_t eeprom_window;
    uint8_t previous_type;
    uint8_t active;
} bc_rtc_window_state_t;
extern uint8_t bc_rtc_window_type_shadow;
extern uint8_t bc_rtc_window_type_known;
bc_status_t bc__rtc_window_begin(bc_avr_window_t type, bc_rtc_window_state_t *state);
void bc__rtc_window_end(bc_rtc_window_state_t *state);
bc_status_t bc_rtc_read(bc_rtc_index_t index, uint8_t *value);
bc_status_t bc_rtc_write(bc_rtc_index_t index, uint8_t value);
bc_status_t bc_rtc_read_datetime(bc_datetime_t *datetime, bc_rtc_format_t format);
bc_status_t bc_nvram_read(uint8_t index, uint8_t *value);
bc_status_t bc_nvram_write(uint8_t index, uint8_t value);
bc_status_t bc_avr_window_select(bc_avr_window_t type);
bc_status_t bc_read_avr_version(bc_avr_version_t *version);
bc_status_t bc_read_firmware_version(bc_avr_version_t *version);
bc_status_t bc_read_video_output(bc_video_output_t *output);
bc_status_t bc_rtc_control_c_read(bc_rtc_control_c_t *state);
void bc_caps_led_set(uint8_t enabled);
bc_status_t bc_eeprom_window_set(uint8_t enabled);
bc_status_t bc_eeprom_read(bc_eeprom_address_t address, uint8_t *buffer, uint8_t length);
bc_status_t bc_eeprom_write(bc_eeprom_address_t address, const uint8_t *buffer, uint8_t length);

#endif
