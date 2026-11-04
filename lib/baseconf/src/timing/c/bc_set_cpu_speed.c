#include <baseconf.h>
#include <baseconf/timing.h>
#include "../internal.h"
#include "../../memory/internal.h"
#include "../../video/internal.h"

bc_status_t bc_set_cpu_speed(bc_cpu_speed_t speed) {
    uint8_t eff7, sys, bf;
    bc_status_t status;
    uint16_t port;
    if (speed > BC_CPU_14_MHZ) return BC_INVALID_ARGUMENT;
    eff7 = bc_readback_register(0x0b);
    sys = (uint8_t)(bc_readback_register(0x0c) & 0x0fu);
    if (speed == BC_CPU_14_MHZ)
        sys |= 8u;
    else {
        sys &= (uint8_t)~8u;
        eff7 =
            speed == BC_CPU_3_5_MHZ ? (uint8_t)(eff7 | 0x10u) : (uint8_t)(eff7 & (uint8_t)~0x10u);
        status = bc__mem_write_eff7(eff7);
        if (status != BC_OK) return status;
    }
    port = bc__video_system_port(0);
    bf = bc__mem_shadow_scope_begin();
    z80_outp(port, sys);
    bc__mem_shadow_scope_end(bf);
    {
        bc_cpu_speed_t observed;
        status = bc_get_cpu_speed(&observed);
        if (status != BC_OK) return status;
        return observed == speed ? BC_OK : BC_UNSAFE_CONTEXT;
    }
}
