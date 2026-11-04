#ifndef BASECONF_RTC_H
#define BASECONF_RTC_H

#include <stdint.h>
#include <baseconf.h>

typedef enum {
    BC_RTC_SECONDS = 0,
    BC_RTC_MINUTES = 2,
    BC_RTC_HOURS = 4,
    BC_RTC_WEEKDAY = 6,
    BC_RTC_DAY = 7,
    BC_RTC_MONTH = 8,
    BC_RTC_YEAR = 9,
    BC_RTC_CONTROL_A = 10,
    BC_RTC_CONTROL_B = 11,
    BC_RTC_CONTROL_C = 12,
    BC_RTC_CONTROL_D = 13
} bc_rtc_index_t;

typedef enum { BC_RTC_BINARY, BC_RTC_BCD } bc_rtc_format_t;

typedef struct {
    uint8_t second, minute, hour, weekday, day, month, year;
} bc_datetime_t;

typedef struct {
    char name[12];
    uint16_t date;
    uint16_t crc;
    uint8_t official;
    uint8_t year;
    uint8_t month;
    uint8_t day;
} bc_avr_version_t;

typedef enum {
    BC_AVR_WINDOW_VERSION = 0,
    BC_AVR_WINDOW_BOOTLOADER = 1,
    BC_AVR_WINDOW_PS2_LOG = 2,
    BC_AVR_WINDOW_MODES = 3
} bc_avr_window_t;

typedef enum {
    BC_RASTER_PENTAGON = 0,
    BC_RASTER_60HZ = 1,
    BC_RASTER_48K = 2,
    BC_RASTER_128K = 3
} bc_raster_code_t;

typedef struct {
    bc_raster_code_t raster;
    uint8_t vga;
    uint8_t tape_out;
    uint8_t caps_led;
} bc_video_output_t;

typedef struct {
    uint8_t caps_led;
    uint8_t num_led;
    uint8_t sd_present;
    uint8_t sd_writable;
    uint8_t time_updated;
    uint8_t eeprom_window;
} bc_rtc_control_c_t;

typedef struct {
    uint16_t offset;
} bc_eeprom_address_t;

/* Select an AVR extension type and remember it for later temporary accesses.
 * Preserves Caps LED and EEPROM-window state; returns BC_INVALID_ARGUMENT for
 * an unknown type. Call before version, video-output, PS/2-log or EEPROM access.
 * If another caller changes the extension directly, select it again to resync. */
bc_status_t bc_avr_window_select(bc_avr_window_t type);
/* Set Caps Lock LED (zero off, nonzero on), preserving EEPROM-window selection.
 * Reads control C and therefore clears time_updated. Num Lock LED is read-only. */
void bc_caps_led_set(uint8_t enabled);
/* Read length bytes (0..255) at address.offset in the 4 KiB AVR EEPROM.
 * The range must end at or before 4096; zero length is allowed, including at
 * offset 4096 with NULL buffer. Nonempty reads require a non-NULL buffer and
 * known extension ownership (bc_avr_window_select). Restores extension and
 * EEPROM-window selection after access. Returns BC_INVALID_ARGUMENT for an
 * invalid range/buffer, BC_UNSAFE_CONTEXT for unknown ownership, otherwise BC_OK. */
bc_status_t bc_eeprom_read(bc_eeprom_address_t address, uint8_t *buffer, uint8_t length);
/* Enable (nonzero) or disable (zero) the global AVR EEPROM window, preserving
 * the remembered extension type and Caps LED. Reads control C, clearing
 * time_updated. Returns BC_UNSAFE_CONTEXT until bc_avr_window_select is called. */
bc_status_t bc_eeprom_window_set(uint8_t enabled);
/* Persistently write length bytes to AVR EEPROM. Range, buffer, ownership and
 * restoration rules are the same as bc_eeprom_read; a zero-length write is a
 * no-op. Bytes already written are not rolled back if a later access fails. */
bc_status_t bc_eeprom_write(bc_eeprom_address_t address, const uint8_t *buffer, uint8_t length);
/* Read one raw NVRAM byte at index 0Eh..EFh. Returns BC_INVALID_ARGUMENT for
 * an out-of-range index or NULL value, otherwise BC_OK. */
bc_status_t bc_nvram_read(uint8_t index, uint8_t *value);
/* Write one raw NVRAM byte at index 0Eh..E6h; E7h..EFh are not writable through
 * this API. Returns BC_INVALID_ARGUMENT outside this range, otherwise BC_OK. */
bc_status_t bc_nvram_write(uint8_t index, uint8_t value);
/* Read AVR build metadata through extension type 0 and restore the previous
 * extension/EEPROM selection. Requires ownership from bc_avr_window_select.
 * name is exactly 12 bytes and may lack a NUL terminator. date is the packed
 * build date; year stores (2000 + its six-bit year) modulo 256, not a full year.
 * Returns BC_INVALID_ARGUMENT for NULL version, BC_UNSAFE_CONTEXT for unknown
 * extension ownership, otherwise BC_OK. */
bc_status_t bc_read_avr_version(bc_avr_version_t *version);
/* Same AVR metadata and return values as bc_read_avr_version; this does not
 * read the FPGA or ERS ROM version. */
bc_status_t bc_read_firmware_version(bc_avr_version_t *version);
/* Read current raster, VGA, tape-output and Caps LED state through extension
 * type 3; restore the previous extension/EEPROM selection. Requires known
 * ownership; returns BC_UNSAFE_CONTEXT if unknown, BC_INVALID_ARGUMENT for NULL. */
bc_status_t bc_read_video_output(bc_video_output_t *output);
/* Read RTC control C; reading clears its time_updated bit in the AVR. */
bc_status_t bc_rtc_control_c_read(bc_rtc_control_c_t *state);
/* Read raw RTC register 00h..0Fh without converting its format. Reading control C
 * clears time_updated. Returns BC_INVALID_ARGUMENT for invalid index/NULL value. */
bc_status_t bc_rtc_read(bc_rtc_index_t index, uint8_t *value);
/* Read date/time in binary or BCD, temporarily changing control B's format bit
 * and restoring it afterward. INT is masked during the sequence, but RTC time
 * updates are not frozen: fields can span a clock rollover.
 * Returns BC_INVALID_ARGUMENT for NULL datetime or an unknown format. */
bc_status_t bc_rtc_read_datetime(bc_datetime_t *datetime, bc_rtc_format_t format);
/* Write a raw byte to RTC register 00h..0Fh; hardware defines writable bits and
 * side effects. Does not convert or validate value. Returns BC_INVALID_ARGUMENT
 * for an out-of-range index, otherwise BC_OK. */
bc_status_t bc_rtc_write(bc_rtc_index_t index, uint8_t value);

#endif
