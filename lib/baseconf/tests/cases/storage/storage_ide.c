#include <baseconf.h>
#include <string.h>

#include "bc_test.h"

enum { ATA_WORD = 0x8010, ATA_POLLS = 1024 };
static uint8_t sector[512];

static void check_block(void) {
    bc_ata_info_t info;
    uint16_t i;
    bc_status_t status;
    status = bc_ide_get_info(&info, sector, ATA_POLLS);
    BC_ASSERT(status == BC_OK && info.lba28_sectors > 8 && info.model[0]);
    BC_ASSERT(bc_ide_read_sector(0, sector, 0) == BC_INVALID_ARGUMENT);
    BC_ASSERT(bc_ide_read_sector(0x10000000ul, sector, ATA_POLLS) == BC_INVALID_ARGUMENT);
    memset(sector, 0xff, sizeof(sector));
    status = bc_ide_read_sector(0, sector, ATA_POLLS);
    BC_ASSERT(status == BC_OK);
    if (status != BC_OK) return;
    BC_ASSERT(sector[450] == 0x0c && sector[455] == 8);
    BC_ASSERT(sector[510] == 0x55 && sector[511] == 0xaa);
    bc_test_step_marker("read_mbr");

    /* LBA 1 is outside the MBR and the partition, on a disposable image. */
    for (i = 0; i < sizeof(sector); ++i) sector[i] = (uint8_t)(i ^ (i >> 8) ^ 0xa5u);
    status = bc_ide_write_sectors(1, sector, 1, ATA_POLLS);
    BC_ASSERT(status == BC_OK);
    if (status != BC_OK) return;
    memset(sector, 0, sizeof(sector));
    status = bc_ide_read_sectors(1, sector, 1, ATA_POLLS);
    BC_ASSERT(status == BC_OK);
    if (status != BC_OK) return;
    for (i = 0; i < sizeof(sector); ++i)
        BC_ASSERT(sector[i] == (uint8_t)(i ^ (i >> 8) ^ 0xa5u));
    bc_test_step_marker("block_roundtrip");
}

static void check_words(void) {
    uint16_t i, word;
    bc_ata_status_t ata;
    bc_status_t status = bc_ide_start_sector(BC_ATA_WRITE, 1, ATA_POLLS);
    BC_ASSERT(status == BC_OK);
    if (status != BC_OK) return;
    BC_ASSERT(bc_ide_status(0, &ata) == BC_OK);
    BC_ASSERT((ata.value & (BC_ATA_BUSY | BC_ATA_DRQ | BC_ATA_ERROR)) == BC_ATA_DRQ);
    for (i = 0; i < 256; ++i)
        BC_ASSERT(bc_ide_write_word(ATA_WORD, (uint16_t)(0xa500u | i)) == BC_OK);
    BC_ASSERT(bc_ide_wait_complete(ATA_POLLS) == BC_OK);
    status = bc_ide_start_sector(BC_ATA_READ, 1, ATA_POLLS);
    BC_ASSERT(status == BC_OK);
    if (status != BC_OK) return;
    for (i = 0; i < 256; ++i) {
        BC_ASSERT(bc_ide_read_word(ATA_WORD, &word) == BC_OK);
        BC_ASSERT(word == (uint16_t)(0xa500u | i));
    }
    BC_ASSERT(bc_ide_wait_complete(ATA_POLLS) == BC_OK);
    bc_test_step_marker("word_roundtrip");
}

int main(void) {
    if (bc_test_init() == BC_OK) {
        check_block();
        check_words();
    }
    bc_test_finish();
    return 0;
}
