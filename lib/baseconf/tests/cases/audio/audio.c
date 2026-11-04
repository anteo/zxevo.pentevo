#include <baseconf.h>
#include <baseconf/audio.h>
#include <psg.h>

#include "bc_test.h"
#include "audio_test_helpers.h"

extern const uint8_t audio_pt3_module[];

static void test_ay(void) {
    static const uint16_t periods[] = {418u, 332u, 279u, 209u}; /* C, E, G, C */
    uint8_t note;

    psg_init();
    psg_volume(0, 15);
    for (note = 0; note < sizeof(periods) / sizeof(periods[0]); ++note) {
        psg_tone(0, periods[note]);
        audio_test_hold();
    }
}

static void test_covox(void) {
    /* One continuous 32-sample sine period, high nibble first. */
    static const uint8_t wave[] = {0x89u, 0xabu, 0xdeu, 0xefu, 0xffu, 0xedu, 0xbau, 0x98u,
                                   0x76u, 0x54u, 0x21u, 0x10u, 0x00u, 0x12u, 0x45u, 0x67u};
    static uint8_t samples[256];
    uint16_t index;
    uint8_t repeat;

    BC_ASSERT(bc_covox_play(NULL, sizeof(samples)) == BC_INVALID_ARGUMENT);
    BC_ASSERT(bc_covox_play(samples, 0u) == BC_INVALID_ARGUMENT);
    for (index = 0; index < sizeof(samples); ++index) samples[index] = wave[index & 15u];
    for (repeat = 0; repeat < 30u; ++repeat) {
        BC_ASSERT(bc_covox_play(samples, sizeof(samples)) == BC_OK);
    }
}

static void test_pt3(void) {
    uint8_t tick;

    BC_ASSERT(bc_pt3_init(NULL) == BC_INVALID_ARGUMENT);
    BC_ASSERT(bc_pt3_init(audio_pt3_module) == BC_OK);
    bc_pt3_play();
    bc_pt3_start();
    for (tick = 0; tick < 8u; ++tick) {
        bc_pt3_play();
        audio_test_hold();
    }

    bc_pt3_stop();
    bc_pt3_play();
    BC_ASSERT(bc_pt3_init(audio_pt3_module) == BC_OK);
    bc_pt3_start();
    bc_pt3_mute();
    bc_pt3_play();
    bc_pt3_stop();

    BC_ASSERT(bc_pt3_init(audio_pt3_module) == BC_OK);
    bc_pt3_start();
    bc_pt3_play();
}

int main(void) {
    static const uint8_t covox_silence = 0x88u;

    if (bc_test_init() != BC_OK) {
        BC_ASSERT(0);
        bc_test_finish();
        return 0;
    }

    test_ay();
    bc_test_step_marker("ay");
    psg_volume(0, 0);

    test_covox();
    BC_ASSERT(bc_covox_play(&covox_silence, 1u) == BC_OK);
    bc_test_step_marker("covox");

    test_pt3();
    bc_test_step_marker("pt3");
    bc_pt3_stop();

    bc_test_finish();
    return 0;
}
