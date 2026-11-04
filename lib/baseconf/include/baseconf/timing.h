#ifndef BASECONF_TIMING_H
#define BASECONF_TIMING_H

#include <stdint.h>
#include <baseconf.h>
#include <baseconf/rtc.h>

/* Read the AVR raster setting and report its nominal frame period in 3.5 MHz
 * Z80 T-states, independent of CPU turbo speed. Requires known AVR extension
 * ownership (bc_avr_window_select); propagates errors from bc_read_video_output.
 * Returns BC_INVALID_ARGUMENT for NULL tstates. */
bc_status_t bc_frame_period(uint32_t *tstates);
/* Decode the current CPU clock setting from live FPGA register readback.
 * Returns BC_INVALID_ARGUMENT for NULL speed, otherwise BC_OK. */
bc_status_t bc_get_cpu_speed(bc_cpu_speed_t *speed);
/* Select 3.5, 7 or 14 MHz and read back the setting; mismatch returns BC_UNSAFE_CONTEXT. */
bc_status_t bc_set_cpu_speed(bc_cpu_speed_t speed);
/* HALT until an interrupt wakes the CPU; returns BC_UNSAFE_CONTEXT if INT is
 * disabled, otherwise BC_OK after wakeup. To use this as a frame wait, arrange
 * for frame INT to be the waking source and exclude NMI. Does not change the
 * interrupt-enable state; do not call from an interrupt handler. */
bc_status_t bc_wait_frame(void);

#endif
