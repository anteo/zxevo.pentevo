#include <baseconf.h>
#include <baseconf/rtc.h>
#include "../internal.h"
#include "../../internal.h"

static bc_status_t eeprom_transfer_byte(uint16_t offset, uint8_t *byte, uint8_t write) {
    bc_rtc_window_state_t state;
    bc_status_t status;

    status = bc__rtc_window_begin((bc_avr_window_t)bc_rtc_window_type_shadow, &state);
    if (status != BC_OK) return status;
    bc__rtc_write_scoped(0x0au, (uint8_t)(offset >> 4));
    bc__rtc_write_scoped(0x0cu, (uint8_t)(state.caps_led | 0x80u));
    if (write)
        bc__rtc_write_scoped((uint8_t)(0xf0u | (offset & 0x0fu)), *byte);
    else
        *byte = bc__rtc_read_scoped((uint8_t)(0xf0u | (offset & 0x0fu)));
    /* Turn off EEPROM selection before restoring the remembered type. */
    bc__rtc_write_scoped(0x0cu, state.caps_led);
    bc__rtc_window_end(&state);
    return BC_OK;
}

static bc_status_t eeprom_transfer(bc_eeprom_address_t address, uint8_t *buffer, uint8_t length,
                                   uint8_t write) {
    uint16_t offset = address.offset;
    uint16_t end = (uint16_t)(offset + length);
    uint8_t i;
    bc_status_t status;

    if ((!buffer && length) || offset > 4096u || end > 4096u) return BC_INVALID_ARGUMENT;
    if (!length) return BC_OK;
    if (!bc_rtc_window_type_known) return BC_UNSAFE_CONTEXT;
    for (i = 0; i < length; ++i) {
        status = eeprom_transfer_byte(offset, &buffer[i], write);
        if (status != BC_OK) return status;
        ++offset;
    }
    return BC_OK;
}

bc_status_t bc_eeprom_write(bc_eeprom_address_t address, const uint8_t *buffer, uint8_t length) {
    return eeprom_transfer(address, (uint8_t *)buffer, length, 1u);
}

bc_status_t bc_eeprom_read(bc_eeprom_address_t address, uint8_t *buffer, uint8_t length) {
    return eeprom_transfer(address, buffer, length, 0u);
}
