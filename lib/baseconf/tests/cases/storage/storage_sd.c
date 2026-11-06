#include <arch/z80.h>
#include <baseconf.h>
#include <string.h>

#include "bc_test.h"

static bc_sd_transaction_t transaction;
static uint8_t sector[512];

static void check_capacity(void) {
    uint8_t csd[16] = {0};
    uint32_t capacity;

    csd[0] = 0x40; /* CSD v2, C_SIZE = 0x10000 (uses bits above the low word). */
    csd[7] = 1;
    BC_ASSERT(bc_sd_sector_count(BC_SD_SDHC, csd, &capacity) == BC_OK && capacity == 0x04000400ul);
    BC_ASSERT(bc_sd_size(BC_SD_SDHC, csd, &capacity) == BC_UNSUPPORTED);

    csd[0] = 0;
    csd[5] = 9; /* CSD v1, 512-byte blocks, C_SIZE = 3, C_SIZE_MULT = 0. */
    csd[7] = 0;
    csd[8] = 0xc0;
    BC_ASSERT(bc_sd_size(BC_SD_SDSC, csd, &capacity) == BC_OK && capacity == 8192);
}

static void emulator_card_select(uint8_t selected) {
    bc_paging_snapshot_t snapshot;
    uint8_t shadow;
    if (!transaction.control_port_high) return;
    shadow = BC_CONFIG_SHADOW_PORTS(z80_inp(0x00bf));
    /* ZEsarUX treats shadow 8057h writes as SPI data. Select the
     * test card through the ordinary 77h port, then restore decoding. */
    BC_ASSERT(bc_get_paging_state(&snapshot) == BC_OK);
    BC_ASSERT(bc_set_shadow_ports_override(1) == BC_OK);
    z80_outp(snapshot.saved_xx77_address | 0x0200u, snapshot.saved_xx77_data);
    BC_ASSERT(bc_set_shadow_ports_override(0) == BC_OK);
    z80_outp(0x0077, selected ? 1 : 3);
    BC_ASSERT(bc_set_shadow_ports_override(1) == BC_OK);
    z80_outp(snapshot.saved_xx77_address, snapshot.saved_xx77_data);
    BC_ASSERT(bc_set_shadow_ports_override(shadow) == BC_OK);
}

static uint8_t receive(void) {
    uint8_t value = 0xff;
    BC_ASSERT(bc_sd_receive_previous(&transaction, &value) == BC_OK);
    return value;
}

static uint8_t read_sector(uint16_t offset) {
    uint8_t response;
    bc_status_t status = bc_sd_command(&transaction, 17, offset, &response);
    BC_ASSERT(status == BC_OK && response == 0);
    if (status != BC_OK || response != 0) return 0;
    status = bc_sd_wait_data(&transaction, 16);
    BC_ASSERT(status == BC_OK);
    if (status != BC_OK) return 0;
    BC_ASSERT(bc_sd_read_block(&transaction, sector) == BC_OK);
    (void)receive();
    (void)receive();
    return 1;
}

static void check_card(void) {
    bc_sd_transaction_t other = {0};
    uint8_t shadow = BC_CONFIG_SHADOW_PORTS(z80_inp(0x00bf));
    uint8_t response;
    uint16_t i;
    bc_status_t status = bc_sd_begin(&transaction);
    BC_ASSERT(status == BC_OK);
    if (status != BC_OK) return;
    BC_ASSERT(bc_sd_begin(&other) == BC_BUSY);
    BC_ASSERT(bc_sd_receive_previous(&transaction, &response) == BC_BUSY);
    BC_ASSERT(bc_sd_end(&other) == BC_INVALID_ARGUMENT);
    BC_ASSERT(bc_sd_wait_data(&transaction, 0) == BC_INVALID_ARGUMENT);
    BC_ASSERT(bc_sd_command(&other, 0, 0, &response) == BC_INVALID_ARGUMENT);
    emulator_card_select(1);
    BC_ASSERT(bc_sd_command(&transaction, 0, 0, &response) == BC_OK && response == 1);
    check_capacity();
    bc_test_step_marker("transaction");

    if (read_sector(0)) {
        BC_ASSERT(sector[450] == 6 && sector[455] == 8);
        BC_ASSERT(sector[510] == 0x55 && sector[511] == 0xaa);
    }
    bc_test_step_marker("read_mbr");

    for (i = 0; i < sizeof(sector); ++i) sector[i] = (uint8_t)(i ^ (i >> 8) ^ 0x6du);
    status = bc_sd_command(&transaction, 24, 512, &response);
    BC_ASSERT(status == BC_OK && response == 0);
    if (status == BC_OK && response == 0) {
        BC_ASSERT(bc_sd_transmit(&transaction, 0xfe) == BC_OK);
        BC_ASSERT(bc_sd_write_block(&transaction, sector) == BC_OK);
        BC_ASSERT(bc_sd_transmit(&transaction, 0xff) == BC_OK);
        BC_ASSERT(bc_sd_transmit(&transaction, 0xff) == BC_OK);
    }
    BC_ASSERT(bc_sd_end(&transaction) == BC_OK);
    BC_ASSERT(BC_CONFIG_SHADOW_PORTS(z80_inp(0x00bf)) == shadow);
    BC_ASSERT(bc_sd_end(&transaction) == BC_INVALID_ARGUMENT);
    emulator_card_select(0);

    status = bc_sd_begin(&transaction);
    BC_ASSERT(status == BC_OK);
    if (status != BC_OK) return;
    emulator_card_select(1);
    memset(sector, 0, sizeof(sector));
    if (read_sector(512)) {
        for (i = 0; i < sizeof(sector); ++i)
            BC_ASSERT(sector[i] == (uint8_t)(i ^ (i >> 8) ^ 0x6du));
    }
    BC_ASSERT(bc_sd_end(&transaction) == BC_OK);
    BC_ASSERT(BC_CONFIG_SHADOW_PORTS(z80_inp(0x00bf)) == shadow);
    emulator_card_select(0);
    bc_test_step_marker("block_roundtrip");
}

int main(void) {
    if (bc_test_init() == BC_OK) check_card();
    bc_test_finish();
    return 0;
}
