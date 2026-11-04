#include <arch/z80.h>
#include <baseconf.h>

#include "bc_test.h"

int main(void) {
    const bc_mouse_snapshot_t previous = {253, 2, 15, BC_MOUSE_RIGHT};
    const bc_mouse_snapshot_t current = {3, 254, 1, BC_MOUSE_LEFT | BC_MOUSE_MIDDLE};
    const bc_mouse_snapshot_t extremes = {131, 125, 9, 0};
    bc_mouse_snapshot_t sample;
    bc_mouse_delta_t delta;
    uint8_t flags;
    if (bc_test_init() == BC_OK) {
        BC_ASSERT(bc_mouse_read(0) == BC_INVALID_ARGUMENT);
        BC_ASSERT(bc_mouse_read(&sample) == BC_OK);
        flags = z80_inp(0xfadf);
        BC_ASSERT(sample.x == z80_inp(0xfbdf) && sample.y == z80_inp(0xffdf));
        BC_ASSERT(sample.wheel == (flags >> 4));
        BC_ASSERT(sample.buttons == ((uint8_t)~flags & 7u));
        bc_test_step_marker("ports");

        delta = bc_mouse_delta(&previous, &current);
        BC_ASSERT(delta.dx == 6 && delta.dy == -4 && delta.wheel == 2);
        BC_ASSERT(delta.buttons == (BC_MOUSE_LEFT | BC_MOUSE_MIDDLE));
        delta = bc_mouse_delta(&current, &previous);
        BC_ASSERT(delta.dx == -6 && delta.dy == 4 && delta.wheel == -2);
        BC_ASSERT(delta.buttons == BC_MOUSE_RIGHT);
        delta = bc_mouse_delta(&current, &extremes);
        BC_ASSERT(delta.dx == -128 && delta.dy == 127 && delta.wheel == -8);
        BC_ASSERT(delta.buttons == 0);
        bc_test_step_marker("wraparound");
    }
    bc_test_finish();
    return 0;
}
