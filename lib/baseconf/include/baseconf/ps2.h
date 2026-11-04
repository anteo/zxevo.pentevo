#ifndef BASECONF_PS2_H
#define BASECONF_PS2_H

#include <stdint.h>
#include <baseconf.h>

#define BC_PS2_MODIFIER_MASK 0x7fu
/* Log refills: 00 is empty, FF marks overflow and is returned once. */
#define BC_PS2_SCANCODE_EMPTY 0x00u
#define BC_PS2_SCANCODE_OVERFLOW 0xffu

/* Physical PS/2 modifier state from RTC register 0D; each present key sets its bit. */
typedef enum {
    BC_PS2_LCTRL = 0x01,
    BC_PS2_RCTRL = 0x02,
    BC_PS2_LALT = 0x04,
    BC_PS2_RALT = 0x08,
    BC_PS2_LSHIFT = 0x10,
    BC_PS2_RSHIFT = 0x20,
    BC_PS2_F12 = 0x40
} bc_ps2_modifier_t;

/* Read pressed physical PS/2 modifier bits (bc_ps2_modifier_t); excludes the AVR
 * marker bit. Returns BC_INVALID_ARGUMENT for NULL modifiers, BC_UNSUPPORTED
 * when RTC 0Dh lacks the marker, otherwise BC_OK. */
bc_status_t bc_ps2_modifiers(uint8_t *modifiers);
/* Remove one raw PS/2 log byte, temporarily selecting the log extension and
 * restoring the previous AVR window afterward. Establish window ownership with
 * bc_avr_window_select first, or this returns BC_UNSAFE_CONTEXT.
 * BC_OK also covers BC_PS2_SCANCODE_EMPTY and BC_PS2_SCANCODE_OVERFLOW in *code;
 * returns BC_INVALID_ARGUMENT for NULL code. */
bc_status_t bc_ps2_scancode_read(uint8_t *code);
/* Discard the AVR PS/2 log, preserving Caps LED and EEPROM-window state.
 * Does not require known extension ownership; returns BC_OK. */
bc_status_t bc_ps2_log_clear(void);

#endif
