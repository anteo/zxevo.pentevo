#include <baseconf/palette.h>

#include "bc_test.h"

int main(void) {
    uint16_t packed;

    if (bc_test_init() != BC_OK) {
        BC_ASSERT(0);
        bc_test_finish();
        return 0;
    }

    for (packed = 0; packed < 256u; ++packed) {
        const uint8_t value = (uint8_t)packed;
        const uint8_t left = bc_unpack_pentagon_left(value);
        const uint8_t right = bc_unpack_pentagon_right(value);

        BC_ASSERT(left <= 15u);
        BC_ASSERT(right <= 15u);
        BC_ASSERT(BC_PACK_PENTAGON_PIXELS(left, right) == value);
    }

    bc_test_finish();
    return 0;
}
