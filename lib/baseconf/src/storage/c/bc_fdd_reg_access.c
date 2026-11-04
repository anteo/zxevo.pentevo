#include "../internal.h"

static uint16_t fdd_reg_port(bc_fdd_reg_t reg) {
    static const uint8_t ports[] = {0x1f, 0x3f, 0x5f, 0x7f};
    return (uint16_t)ports[(uint8_t)reg];
}

static bc_status_t fdd_reg_context(void) {
    /* FE interception swallows WD1793 accesses when its other conditions hold;
     * the library cannot safely infer ROM/DOS/window state from 13BD alone. */
    return (z80_inp(0x13bdu) & 0x0fu) ? BC_UNSAFE_CONTEXT : BC_OK;
}

bc_status_t bc_fdd_reg_read(bc_fdd_reg_t reg, uint8_t *value) {
    uint8_t irq, bf;
    bc_status_t status;
    if (!value) return BC_INVALID_ARGUMENT;
    if ((uint8_t)reg > (uint8_t)BC_FDD_DATA) return BC_INVALID_ARGUMENT;
    status = bc__storage_fdd_shadow_begin(&irq, &bf);
    if (status != BC_OK) return status;
    status = fdd_reg_context();
    if (status == BC_OK) *value = z80_inp(fdd_reg_port(reg));
    bc__storage_fdd_shadow_end(irq, bf);
    return status;
}

bc_status_t bc_fdd_reg_write(bc_fdd_reg_t reg, uint8_t value) {
    uint8_t irq, bf;
    bc_status_t status;
    if ((uint8_t)reg > (uint8_t)BC_FDD_DATA) return BC_INVALID_ARGUMENT;
    status = bc__storage_fdd_shadow_begin(&irq, &bf);
    if (status != BC_OK) return status;
    status = fdd_reg_context();
    if (status == BC_OK) z80_outp(fdd_reg_port(reg), value);
    bc__storage_fdd_shadow_end(irq, bf);
    return status;
}
