#include <baseconf.h>

#include "bc_test.h"

int main(void) {
    if (bc_test_init() == BC_OK) {
        bc_ers_set_profile(BC_ERS_SERVICES_ROM);
        BC_ASSERT(bc_ers_cmos_init() == BC_OK);
        BC_ASSERT(bc_set_video_mode(BC_VIDEO_ZX_256) == BC_OK);
        BC_ASSERT(bc_select_screen(BC_SCREEN_0) == BC_OK);

        BC_ASSERT(bc_ers_nmi_file_browser_lock(1) == BC_OK);
        bc_test_step_marker("nmi_locked");
        BC_ASSERT(bc_ers_nmi_file_browser_lock(0) == BC_OK);
        bc_test_step_marker("nmi_unlocked");
    }
    bc_test_finish();
    return 0;
}
