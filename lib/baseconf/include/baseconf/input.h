#ifndef BASECONF_INPUT_H
#define BASECONF_INPUT_H

#include <stdint.h>
#include <baseconf.h>

typedef enum { BC_MOUSE_LEFT = 1, BC_MOUSE_RIGHT = 2, BC_MOUSE_MIDDLE = 4 } bc_mouse_button_t;

typedef struct {
    uint8_t x, y, wheel, buttons;
} bc_mouse_snapshot_t;

typedef struct {
    int8_t dx, dy, wheel;
    uint8_t buttons;
} bc_mouse_delta_t;

/* Read Kempston mouse counters x/y (modulo 256), wheel (modulo 16), and pressed
 * button bits (bc_mouse_button_t). Counters are raw device values, not screen
 * coordinates. Returns BC_INVALID_ARGUMENT for NULL snapshot, otherwise BC_OK. */
bc_status_t bc_mouse_read(bc_mouse_snapshot_t *snapshot);
/* Subtract snapshots with wraparound: dx/dy are -128..127, wheel is -8..7;
 * buttons is copied from current. Both pointers must be non-NULL. Poll often
 * enough to avoid motion exceeding these signed ranges between snapshots. */
bc_mouse_delta_t bc_mouse_delta(const bc_mouse_snapshot_t *previous,
                                const bc_mouse_snapshot_t *current);

#endif
