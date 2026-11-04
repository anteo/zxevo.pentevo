#include "../internal.h"
#include "../../memory/internal.h"

bc_status_t bc_fdd_overlay_set(uint8_t mask) {
    uint8_t irq, bf, old, readback;
    if (mask & 0xf0u) return BC_INVALID_ARGUMENT;
    irq = bc_interrupt_save_disable();
    bf = z80_inp(BC_PORT_BF);
    if (bf == 0xffu) {
        bc_interrupt_restore(irq);
        return BC_UNAVAILABLE;
    }
    old = (uint8_t)(z80_inp(0x13bdu) & 0x0fu);
    z80_outp(0x13bd, mask);
    readback = z80_inp(0x13bdu);
    if ((readback & 0x0fu) != mask) {
        z80_outp(0x13bdu, old);
        readback = z80_inp(0x13bdu);
    }
    bc_interrupt_restore(irq);
    return ((readback & 0x0fu) == mask) ? BC_OK : BC_UNSAFE_CONTEXT;
}
