#include <baseconf.h>
#include <string.h>

#include "bc_test.h"

static uint8_t bcd_in_range(uint8_t value, uint8_t min, uint8_t max) {
    uint8_t units = value & 15u;
    uint8_t tens = value >> 4;
    uint8_t decimal = 10u * tens + units;
    return units <= 9 && tens <= 9 && decimal >= min && decimal <= max;
}

static void check_clock(void) {
    bc_datetime_t date;
    bc_rtc_control_c_t control;
    uint8_t format, after;
    BC_ASSERT(bc_rtc_read(BC_RTC_CONTROL_B, &format) == BC_OK);
    BC_ASSERT(bc_rtc_read_datetime(&date, BC_RTC_BCD) == BC_OK);
    BC_ASSERT(bcd_in_range(date.second, 0, 59));
    BC_ASSERT(bcd_in_range(date.minute, 0, 59));
    BC_ASSERT(bcd_in_range(date.hour, 0, 23));
    BC_ASSERT(bcd_in_range(date.weekday, 1, 7));
    BC_ASSERT(bcd_in_range(date.day, 1, 31));
    BC_ASSERT(bcd_in_range(date.month, 1, 12));
    BC_ASSERT(bcd_in_range(date.year, 0, 99));
    BC_ASSERT(bc_rtc_read(BC_RTC_CONTROL_B, &after) == BC_OK && after == format);
    BC_ASSERT(bc_rtc_control_c_read(&control) == BC_OK && control.eeprom_window == 0);
    BC_ASSERT(bc_rtc_write((bc_rtc_index_t)0x10, 0) == BC_INVALID_ARGUMENT);
    bc_test_step_marker("clock_bcd");
}

static void check_nvram(void) {
    uint8_t old, readback = 0;
    bc_status_t status = bc_nvram_read(0x70, &old);
    BC_ASSERT(status == BC_OK);
    if (status != BC_OK) return;

    BC_ASSERT(bc_nvram_write(0x70, old ^ 0xa5) == BC_OK);
    status = bc_nvram_read(0x70, &readback);
    BC_ASSERT(bc_nvram_write(0x70, old) == BC_OK);
    BC_ASSERT(status == BC_OK && readback == (uint8_t)(old ^ 0xa5));
    BC_ASSERT(bc_nvram_read(0x70, &readback) == BC_OK && readback == old);
    BC_ASSERT(bc_nvram_write(0xe7, 0) == BC_INVALID_ARGUMENT);
    bc_test_step_marker("nvram");
}

static void check_avr(void) {
    bc_avr_version_t version, alias;
    bc_video_output_t output;
    BC_ASSERT(bc_read_avr_version(&version) == BC_UNSAFE_CONTEXT);
    BC_ASSERT(bc_avr_window_select(BC_AVR_WINDOW_MODES) == BC_OK);
    BC_ASSERT(bc_read_avr_version(&version) == BC_OK);
    BC_ASSERT(memcmp(version.name, "ZXEvo", 5) == 0);
    BC_ASSERT(version.month >= 1 && version.month <= 12);
    BC_ASSERT(version.day >= 1 && version.day <= 31);
    BC_ASSERT(bc_read_firmware_version(&alias) == BC_OK);
    BC_ASSERT(memcmp(version.name, alias.name, sizeof(version.name)) == 0);
    BC_ASSERT(version.date == alias.date && version.crc == alias.crc);
    BC_ASSERT(bc_read_video_output(&output) == BC_OK);
    BC_ASSERT(output.raster <= BC_RASTER_128K);
    bc_test_step_marker("avr_metadata");
}

static void check_eeprom_contract(void) {
    const bc_eeprom_address_t last = {4095};
    const bc_eeprom_address_t end = {4096};
    uint8_t bytes[] = {0xa5, 0x5a};
    BC_ASSERT(bc_eeprom_window_set(1) == BC_UNSAFE_CONTEXT);
    BC_ASSERT(bc_eeprom_read(last, bytes, 1) == BC_UNSAFE_CONTEXT);
    BC_ASSERT(bc_eeprom_write(last, bytes, 1) == BC_UNSAFE_CONTEXT);
    BC_ASSERT(bytes[0] == 0xa5 && bytes[1] == 0x5a);
    BC_ASSERT(bc_eeprom_read(last, bytes, 2) == BC_INVALID_ARGUMENT);
    BC_ASSERT(bc_eeprom_write(last, bytes, 2) == BC_INVALID_ARGUMENT);
    BC_ASSERT(bc_eeprom_read(end, 0, 0) == BC_OK);
    BC_ASSERT(bc_eeprom_write(end, 0, 0) == BC_OK);
    bc_test_step_marker("eeprom_contract");
    bc_test_step_skip("eeprom_pages", "ZEsarUX does not model the 4 KiB EEPROM page selector");
}

int main(void) {
    if (bc_test_init() == BC_OK) {
        check_clock();
        check_nvram();
        check_eeprom_contract();
        check_avr();
    }
    bc_test_finish();
    return 0;
}
