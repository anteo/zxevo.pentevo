#include "../internal.h"

bc_status_t bc_fdd_system_write(const bc_fdd_system_t *cfg) {
    uint8_t irq, bf, data, config;
    uint16_t close_palette, restore_palette;
    bc_status_t status;
    if (!cfg || (uint8_t)cfg->drive > (uint8_t)BC_DRIVE_D || cfg->side > 1u || cfg->hrdy > 1u ||
        cfg->reset > 1u)
        return BC_INVALID_ARGUMENT;
    status = bc__storage_fdd_shadow_begin(&irq, &bf);
    if (status != BC_OK) return status;
    config = bc_readback_register(0x0c);
    close_palette = bc__storage_fdd_system_port(config, 1);
    restore_palette = bc__storage_fdd_system_port(config, 0);
    z80_outp(close_palette, (uint8_t)(config & 0x0fu));
    if (!(bc_readback_register(0x0c) & 0x80u)) {
        z80_outp(restore_palette, (uint8_t)(config & 0x0fu));
        bc__storage_fdd_shadow_end(irq, bf);
        return BC_UNSAFE_CONTEXT;
    }
    data = (uint8_t)((uint8_t)cfg->drive | (cfg->hrdy << 3) | (cfg->side << 4));
    if (!cfg->reset) data |= 0x04u; /* D2=0 asserts controller reset. */
    z80_outp(0x00ffu, data);
    z80_outp(restore_palette, (uint8_t)(config & 0x0fu));
    status = ((bc_readback_register(0x0c) ^ config) & 0x80u) ? BC_UNSAFE_CONTEXT : BC_OK;
    bc__storage_fdd_shadow_end(irq, bf);
    return status;
}
