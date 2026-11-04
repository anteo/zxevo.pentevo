#include <arch/z80.h>
#include <baseconf.h>
#include <string.h>

#include "bc_test.h"

static volatile uint8_t frame_count;
static volatile uint8_t nmi_count;

static void frame_handler(void) __naked {
    __asm
        push af
        ld a,(_frame_count)
        inc a
        ld (_frame_count),a
        pop af
        ei
        reti
    __endasm;
}

static void trap_target(void) __naked {
    __asm
        ret
    __endasm;
}

static void check_maskable_interrupts(void) {
    uint8_t *const vector = (uint8_t *)0x0038u;
    uint16_t handler = (uint16_t)(uintptr_t)frame_handler;

    vector[0] = 0xc3;
    vector[1] = (uint8_t)handler;
    vector[2] = (uint8_t)(handler >> 8);
    BC_ASSERT(bc_interrupt_save_disable() == 0);
    bc_interrupt_restore(0);
    BC_ASSERT(bc_wait_frame() == BC_UNSAFE_CONTEXT);
    bc_interrupt_enable(1);
    BC_ASSERT(bc_interrupt_save_disable() == 1);
    BC_ASSERT(bc_interrupt_save_disable() == 0);
    bc_interrupt_restore(1);
    BC_ASSERT(bc_wait_frame() == BC_OK);
    bc_interrupt_enable(0);
    BC_ASSERT(frame_count != 0);
    BC_ASSERT(bc_interrupt_save_disable() == 0);
    bc_test_step_marker("int");
}

static void install_nmi_handler(void) {
    /* NMI fetches an injected NOP at 0066h, then executes in RAM page FFh.
     * Save AF/BC, increment the resident counter, disarm the trap and RETN. */
    uint8_t handler[] = {
        0xf5, 0xc5, 0x3a, 0, 0, 0x3c, 0x32, 0, 0,
        0x01, 0xbf, 0x00, 0xed, 0x78, 0xcb, 0xa7, 0xed, 0x79,
        0xc1, 0xf1, 0xd3, 0xbe, 0xed, 0x45
    };
    uint16_t counter = (uint16_t)(uintptr_t)&nmi_count;
    handler[3] = handler[7] = (uint8_t)counter;
    handler[4] = handler[8] = (uint8_t)(counter >> 8);
    memcpy((void *)0xc067u, handler, sizeof(handler));
}

static void check_nmi(void) {
    bc_execution_trap_t trap = {(uint16_t)(uintptr_t)trap_target, 1};
    uint16_t polls = 0xffffu;
    bc_status_t status = bc_map_ram_page(BC_WINDOW_3, 0xff);
    BC_ASSERT(status == BC_OK);
    if (status != BC_OK) return;
    install_nmi_handler();

    BC_ASSERT(bc_request_nmi() == BC_OK);
    while (!nmi_count && --polls) {}
    BC_ASSERT(nmi_count == 1);
    BC_ASSERT(bc_interrupt_save_disable() == 0);
    bc_test_step_marker("nmi");

    bc_set_execution_trap(trap);
    BC_ASSERT(bc_readback_register(0x10) == (uint8_t)trap.address);
    BC_ASSERT(bc_readback_register(0x11) == (uint8_t)(trap.address >> 8));
    BC_ASSERT((z80_inp(0x00bf) & 0x10) != 0);
    trap_target();
    BC_ASSERT(nmi_count == 2);
    bc_set_execution_trap(trap);
    BC_ASSERT(bc_clear_execution_trap() == BC_OK);
    BC_ASSERT((z80_inp(0x00bf) & 0x10) == 0);
    BC_ASSERT(bc_readback_register(0x10) == (uint8_t)trap.address);
    BC_ASSERT(bc_readback_register(0x11) == (uint8_t)(trap.address >> 8));
    trap_target();
    BC_ASSERT(nmi_count == 2);
    bc_test_step_marker("trap");
}

int main(void) {
    if (bc_test_init() == BC_OK) {
        check_maskable_interrupts();
        check_nmi();
    }
    bc_test_finish();
    return 0;
}
