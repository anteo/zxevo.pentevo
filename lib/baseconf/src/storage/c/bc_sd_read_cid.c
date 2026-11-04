#include "../internal.h"

bc_status_t bc_sd_read_cid(uint8_t cid[16], uint16_t max_polls) {
    return bc__sd_read_register(10, cid, max_polls);
}
