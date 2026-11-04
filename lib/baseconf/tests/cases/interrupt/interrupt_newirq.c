#include <baseconf.h>

#include "bc_test.h"

int main(void) {
    bc_newirq_status_t status = {0xa5, 0x5a};
    if (bc_test_init() == BC_OK) {
        BC_ASSERT(bc_newirq_status(0) == BC_INVALID_ARGUMENT);
        BC_ASSERT(bc_newirq_status(&status) == BC_UNAVAILABLE);
        BC_ASSERT(status.pending == 0xa5 && status.source == 0x5a);
        BC_ASSERT(bc_newirq_enable_sources(1) == BC_UNAVAILABLE);
        BC_ASSERT(bc_newirq_acknowledge(1) == BC_UNAVAILABLE);
        bc_test_step_marker("disabled");
        bc_test_step_skip("sources", "ZEsarUX has no BaseConf newirq registers");
    }
    bc_test_finish();
    return 0;
}
