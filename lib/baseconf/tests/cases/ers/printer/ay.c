#include <baseconf.h>
#include <arch/z80.h>

#include "bc_test.h"

#define CMOS_PRINTER_INDEX 0xedu
#define CMOS_PRINTER_ENABLE 0x20u
#define AY_MIXER 7u
#define AY_PORT_A 14u
#define AY_PORT_B 15u
#define AY_SELECT_PORT 0xfffdu
#define AY_WRITE_PORT 0xbffdu
#define BASIC_FLAGS_PRINTER 0x02u

void printer_test_copy_screen(void);

static uint8_t read_ay(uint8_t reg) {
    z80_outp(AY_SELECT_PORT, reg);
    return z80_inp(AY_SELECT_PORT);
}

static void seed_printer_data(void) {
    z80_outp(AY_SELECT_PORT, AY_PORT_A);
    z80_outp(AY_WRITE_PORT, 0x55);
}

static void check_screen_copy(void) {
    volatile uint8_t *const flags = (volatile uint8_t *)0x5c3bu;
    volatile uint8_t *const printer_buffer = (volatile uint8_t *)0x5b00u;
    uint8_t original_flags = *flags;

    *flags = original_flags | BASIC_FLAGS_PRINTER;
    printer_buffer[0] = 0x55;
    printer_buffer[255] = 0x55;
    seed_printer_data();
    BC_ASSERT(bc_ers_printer_write_screen() == BC_OK);
    BC_ASSERT(read_ay(AY_PORT_A) == 0x07);
    BC_ASSERT(read_ay(AY_MIXER) == 0x7f);
    BC_ASSERT(printer_buffer[0] == 0x55 && printer_buffer[255] == 0x55);
    bc_test_step_marker("copy_screen_api");

    seed_printer_data();
    printer_test_copy_screen();
    BC_ASSERT(read_ay(AY_PORT_A) == 0x07);
    BC_ASSERT(read_ay(AY_MIXER) == 0x7f);
    BC_ASSERT(printer_buffer[0] == 0 && printer_buffer[255] == 0);
    *flags = original_flags;
}

int main(void) {
    uint8_t original;
    bc_status_t status;

    if (bc_test_init() != BC_OK) goto finish;
    bc_ers_set_profile(BC_ERS_SERVICES_ROM);
    BC_ASSERT(bc_ers_cmos_init() == BC_OK);
    status = bc_ers_cmos_read(CMOS_PRINTER_INDEX, &original);
    BC_ASSERT(status == BC_OK);
    if (status != BC_OK) goto finish;
    BC_ASSERT(bc_ers_cmos_write(CMOS_PRINTER_INDEX, original | CMOS_PRINTER_ENABLE) == BC_OK);

    seed_printer_data();
    BC_ASSERT(bc_ers_printer_init() == BC_OK);
    BC_ASSERT(read_ay(AY_MIXER) == 0x7f);
    BC_ASSERT(read_ay(AY_PORT_B) == 0xfe);
    BC_ASSERT(read_ay(AY_PORT_A) == 0x07);
    bc_test_step_marker("initialize");

    BC_ASSERT(bc_ers_printer_write('Z') == BC_OK);
    BC_ASSERT(read_ay(AY_PORT_A) == 'Z');
    BC_ASSERT(read_ay(AY_MIXER) == 0x7f);
    bc_test_step_marker("write_character");

    BC_ASSERT(bc_ers_printer_put_token(0x01) == BC_OK);
    BC_ASSERT(read_ay(AY_PORT_A) == 'Z');
    BC_ASSERT(bc_ers_printer_put_token(0x90) == BC_OK);
    BC_ASSERT(read_ay(AY_PORT_A) == 'A');
    BC_ASSERT(bc_ers_printer_put_token('\r') == BC_OK);
    BC_ASSERT(read_ay(AY_PORT_A) == '\n');
    bc_test_step_marker("print_tokens");

    check_screen_copy();
    bc_test_step_marker("copy_screen");

    BC_ASSERT(bc_ers_cmos_write(CMOS_PRINTER_INDEX, original & ~CMOS_PRINTER_ENABLE) == BC_OK);
    seed_printer_data();
    BC_ASSERT(bc_ers_printer_write_screen() == BC_OK);
    BC_ASSERT(read_ay(AY_PORT_A) == 0x55);
    BC_ASSERT(bc_ers_cmos_write(CMOS_PRINTER_INDEX, original) == BC_OK);
    bc_test_step_marker("copy_screen_disabled");

finish:
    bc_test_finish();
    return 0;
}
