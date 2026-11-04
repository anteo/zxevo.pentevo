#include <string.h>
#include "../internal.h"

bc_status_t bc_read_avr_version(bc_avr_version_t *version) {
    bc_rtc_window_state_t state;
    uint8_t bytes[16];
    uint16_t date;
    uint8_t i;
    bc_status_t status;

    if (!version) return BC_INVALID_ARGUMENT;
    status = bc__rtc_window_begin(BC_AVR_WINDOW_VERSION, &state);
    if (status != BC_OK) return status;
    for (i = 0; i < 16u; ++i) bytes[i] = bc__rtc_read_scoped((uint8_t)(0xf0u + i));
    bc__rtc_window_end(&state);

    date = (uint16_t)bytes[12] | ((uint16_t)bytes[13] << 8);
    memcpy(version->name, bytes, sizeof(version->name));
    version->date = date;
    version->crc = (uint16_t)bytes[14] | ((uint16_t)bytes[15] << 8);
    version->official = (uint8_t)((date >> 15) & 1u);
    version->year = (uint8_t)(2000u + ((date >> 9) & 0x3fu));
    version->month = (uint8_t)((date >> 5) & 0x0fu);
    version->day = (uint8_t)(date & 0x1fu);
    return BC_OK;
}
