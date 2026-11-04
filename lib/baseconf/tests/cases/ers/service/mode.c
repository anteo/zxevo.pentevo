#include <baseconf.h>

#include "bc_test.h"

static void check_mode(uint8_t request, uint8_t video_turbo, uint8_t slow_clock) {
    uint8_t saved = 0;

    BC_ASSERT(bc_ers_set_mode(request, &saved) == BC_OK);
    BC_ASSERT((bc_readback_register(0x0c) & 0x0fu) == video_turbo);
    BC_ASSERT((bc_readback_register(0x0b) & 0x10u) == slow_clock);
    BC_ASSERT(bc_ers_set_mode(0, &saved) == BC_OK);
    BC_ASSERT(saved == request);
}

int main(void) {
    if (bc_test_init() == BC_OK) {
        bc_ers_set_profile(BC_ERS_SERVICES_ROM);
        check_mode(0x1b, 3, 0x10); /* ZX 256 at 3.5 MHz */
        bc_test_step_marker("zx_256");
        check_mode(0x3f, 0x0f, 0); /* Evolution text at 14 MHz */
        bc_test_step_marker("evo_text");
        check_mode(0x1b, 3, 0x10);
        bc_test_step_marker("restore_zx");
    }
    bc_test_finish();
    return 0;
}
