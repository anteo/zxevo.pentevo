#include "../internal.h"
#include "../../memory/internal.h"

bc_status_t bc_fdd_overlay_probe(void) {
    uint8_t irq, bf, old, first, second, restored;
    irq = bc_interrupt_save_disable();
    bf = z80_inp(BC_PORT_BF);
    if (bf == 0xffu) {
        bc_interrupt_restore(irq);
        return BC_UNAVAILABLE;
    }
    old = z80_inp(0x13bdu);
    z80_outp(0x13bd, 0x05u);
    first = z80_inp(0x13bdu);
    z80_outp(0x13bd, 0x0au);
    second = z80_inp(0x13bdu);
    z80_outp(0x13bd, (uint8_t)(old & 0x0fu));
    restored = z80_inp(0x13bdu);
    bc_interrupt_restore(irq);
    if ((restored & 0x0fu) != (old & 0x0fu)) return BC_UNSAFE_CONTEXT;
    return ((first & 0x0fu) == 0x05u && (second & 0x0fu) == 0x0au) ? BC_OK : BC_UNAVAILABLE;
}
