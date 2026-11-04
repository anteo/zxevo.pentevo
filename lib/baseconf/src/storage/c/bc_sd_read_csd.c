#include "../internal.h"

bc_status_t bc_sd_read_csd(uint8_t csd[16], uint16_t max_polls) {
    return bc__sd_read_register(9, csd, max_polls);
}
