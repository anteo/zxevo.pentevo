#include <arch/z80.h>
#include <baseconf.h>

#include "bc_test.h"

int main(void) {
    if (bc_test_init() == BC_OK) {
        BC_ASSERT(bc_set_video_mode(BC_VIDEO_ATM_320) == BC_OK);
        BC_ASSERT(bc_set_ulaplus_palette(3, 0x9e) == BC_UNAVAILABLE);
        BC_ASSERT(bc_set_video_mode(BC_VIDEO_ZX_256) == BC_OK);
        BC_ASSERT(bc_set_ulaplus_palette(64, 0) == BC_INVALID_ARGUMENT);
        BC_ASSERT(bc_set_ulaplus_palette(3, 0x9e) == BC_OK);
        BC_ASSERT(z80_inp(0xff3b) == 0x9e);
        BC_ASSERT(bc_set_ulaplus_palette(63, 0x31) == BC_OK);
        BC_ASSERT(z80_inp(0xff3b) == 0x31);
        bc_set_ulaplus_enabled(1);
        BC_ASSERT(z80_inp(0xff3b) == 1);
        bc_set_ulaplus_enabled(0);
        BC_ASSERT(z80_inp(0xff3b) == 0);
        bc_test_step_marker("ulaplus_registers");
    }
    bc_test_finish();
    return 0;
}
