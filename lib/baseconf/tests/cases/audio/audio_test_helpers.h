#ifndef BASECONF_TEST_AUDIO_HELPERS_H
#define BASECONF_TEST_AUDIO_HELPERS_H

#include <stdint.h>

static void audio_test_hold(void) {
    volatile uint16_t delay;
    for (delay = 0; delay != 30000u; ++delay) {
    }
}

#endif
