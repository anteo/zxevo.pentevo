#include <baseconf.h>

#include "bc_test.h"

static void check_cpu_speed(void) {
    bc_cpu_speed_t initial, current, speed;
    BC_ASSERT(bc_get_cpu_speed(&initial) == BC_OK);
    BC_ASSERT(bc_get_cpu_speed(0) == BC_INVALID_ARGUMENT);
    for (speed = BC_CPU_3_5_MHZ; speed <= BC_CPU_14_MHZ; ++speed) {
        BC_ASSERT(bc_set_cpu_speed(speed) == BC_OK);
        BC_ASSERT(bc_get_cpu_speed(&current) == BC_OK && current == speed);
    }
    BC_ASSERT(bc_set_cpu_speed((bc_cpu_speed_t)3) == BC_INVALID_ARGUMENT);
    BC_ASSERT(bc_get_cpu_speed(&current) == BC_OK && current == BC_CPU_14_MHZ);
    BC_ASSERT(bc_set_cpu_speed(initial) == BC_OK);
    bc_test_step_marker("cpu_speed");
}

static void check_frame_period(void) {
    static const uint32_t periods[] = {71680u, 58688u, 69888u, 70908u};
    bc_video_output_t output;
    uint32_t period = 0;
    BC_ASSERT(bc_frame_period(0) == BC_INVALID_ARGUMENT);
    BC_ASSERT(bc_frame_period(&period) == BC_UNSAFE_CONTEXT);
    BC_ASSERT(period == 0);
    BC_ASSERT(bc_avr_window_select(BC_AVR_WINDOW_VERSION) == BC_OK);
    BC_ASSERT(bc_read_video_output(&output) == BC_OK);
    BC_ASSERT(bc_frame_period(&period) == BC_OK);
    BC_ASSERT(period == periods[output.raster]);
    bc_test_step_marker("frame_period");
}

int main(void) {
    if (bc_test_init() == BC_OK) {
        check_cpu_speed();
        check_frame_period();
    }
    bc_test_finish();
    return 0;
}
