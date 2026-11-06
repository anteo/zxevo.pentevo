# ZX Evolution computer kit

English | [Русский](README.ru.md)

This project is a fork of the official ZX Evolution `pentevo` SVN repository.
[Official SVN repository](http://svn.nedopc.com/listing.php?repname=pentevo)

- **z88dk target for BaseConf** lets you write C programs that use the platform's
  hardware features. [BaseConf library and examples](lib/baseconf/README.en.md)
- **Fully automated tests** cover the z88dk standard library on BaseConf and
  BaseConf-specific features. Running them requires a current build of the ZEsarUX
  fork with the BaseConf additions and compatibility fixes.
  [Test instructions](lib/baseconf/tests/README.md) ·
  [ZEsarUX fork](https://github.com/anteo/zesarux)
- **Fully revised ZX Evolution developer reference** covers hardware subsystems,
  I/O ports and ROM services. [English](MANUAL.en.md) · [Русский](MANUAL.ru.md)

[Original English project website](http://nedopc.com/zxevo/zxevo_eng.php)

**The hardware is supplied by an independent manufacturer. For purchasing enquiries, contact Vitaly
at [tetroid@inbox.ru](mailto:tetroid@inbox.ru).**

---

[![ZX Evolution rev.C, top](photos/zxevo_top_c.jpg)](photos/zxevo_top_c.jpg)

[![ZX Evolution rev.C, bottom](photos/zxevo_bot_c.jpg)](photos/zxevo_bot_c.jpg)

[![ZX Evolution rev.C, assembled](photos/zxevo_mnt_c.jpg)](photos/zxevo_mnt_c.jpg)

[![ZX Evolution rev.C, front](photos/zxevo_front_c.jpg)](photos/zxevo_front_c.jpg)

**ZX Evolution** is a Spectrum-compatible computer kit built around an FPGA and an AVR
microcontroller, with interfaces for connecting peripherals.

**Development:** hardware by Vadim Akimov (LVD), Roman Chunin (CHRV), and Dmitry Dmitriev (DDP);
software by Vyacheslav Savenkov (Savelij).

## Specifications

- Z80 at 3.5 MHz (classic), 7 MHz (turbo without WAIT delays for ordinary memory access), or 14 MHz
  (mega turbo with WAIT).
- 4 MB RAM, 512 KB ROM.
- Mini-ITX form factor (microATX for revisions A and B), two ZXBUS slots, ATX or +5 V / +12 V power.
- Configurable EP1K50 FPGA architecture.
- ATmega128 peripheral controller.
- PS/2 keyboard and mouse interfaces.
- Floppy controller (1818VG93), IDE (one channel, master/slave), SD(HC), RS-232.
- RGB and VGA video output with a hardware scan doubler that outputs each video line twice.
- AY, beeper, and hardware-PWM Covox audio.
- Mechanical keyboard and joystick support.
- Tape input/output support.
- Battery-backed clock/calendar.

See the
[list of tested VGA monitors](http://svn.nedopc.com/dl.php?repname=pentevo&path=%2Fdocs%2Fsupported_monitors.pdf)
and the
[PC monitor-compatibility test program](http://nedopc.com/zxevo/tools/zxevo_vga_test_20100221.rar).

## Documentation

- [Developer reference in this repository](MANUAL.en.md): subsystems, ATM extensions, ports,
  ROM/RST API, and source discrepancies.
- [User support forum](http://forum.nedopc.com/).
- [Revision C schematic](http://nedopc.com/zxevo/zxevo_sch_revc.pdf).
- [Revision C component placement](http://nedopc.com/zxevo/zxevo_mon_revc.pdf).
- [Revision C bill of materials](http://nedopc.com/zxevo/zxevo_bom_revc.pdf).
- [Revision C user manual](http://nedopc.com/zxevo/zxevo_user_manual_revc.pdf).
- [Revision C assembly guide](http://nedopc.com/zxevo/zxevo_solder_revc.pdf).
- [Revision C manufacturing files](http://nedopc.com/zxevo/zxevo_revc.zip) (PCAD2002). Commercial
  use requires agreement with [NedoPC representatives](http://nedopc.com/aboutus.php).
- [Prices](http://nedopc.com/catalog.php).

The firmware table below links the EVO Reset Service startup-menu manual, BASECONF programming
reference, and bootloader update instructions.

## Firmware and documentation

| Name                                           | Version    | Purpose                                                                                                                                                                                                                                                              | Documentation                                                                                                                                                                                                                                                             | Files                                                                          | SVN                                                                                                                                                                                                                                                                                                                                                |
| ---------------------------------------------- | ---------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------ | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **EVO RESET SERVICE FE, EVO DOS FE, EVO PROF** | latest     | Improved floppy emulation works with an unmodified TR-DOS ROM and requires matching FPGA firmware. Startup menu, service extensions, and Spectrum ROMs. Installed in ROM; update through an earlier version or **TEST&SERVICE**. Maintainer: Savelij.                | [EVO Reset Service manual](http://svn.nedopc.com/filedetails.php?repname=pentevo&path=%2Fdocs%2F%D0%9E%D0%BF%D0%B8%D1%81%D0%B0%D0%BD%D0%B8%D0%B5+%D1%81%D0%B5%D1%80%D0%B2%D0%B8%D1%81-%D0%BF%D1%80%D0%BE%D1%88%D0%B8%D0%B2%D0%BA%D0%B8+%27%27EVO+Reset+Service%27%27.pdf) |                                                                                | [zxevo_fe.rom](http://svn.nedopc.com/filedetails.php?repname=pentevo&path=%2From%2Fzxevo_fe.rom)                                                                                                                                                                                                                                                   |
| **BASECONF FE**                                | latest     | NedoPC's main working configuration. Floppy/VG93 port emulation requires zxevo_fe.rom. Adds the DDP palette without PWM (alone) and faster 14 MHz operation (psk7). Installed in ATmega128 through **BOOTLOADER**. Maintainers: CHRV (ATmega128), LVD (EP1K50QC208). | [ODT reference](http://svn.nedopc.com/filedetails.php?repname=pentevo&path=%2Fdocs%2Fzxevo_base_configuration.odt), [PDF reference](http://svn.nedopc.com/filedetails.php?repname=pentevo&path=%2Fdocs%2Fzxevo_base_configuration.pdf)                                    |                                                                                | [zxevo_fw.bin](http://svn.nedopc.com/filedetails.php?repname=pentevo&path=%2Fcfgs%2Fstandalone_base_trdemu%2Ftrunk%2Fzxevo_fw.bin)                                                                                                                                                                                                                 |
| **BOOTLOADER**                                 | 2012.01.24 | Loads and updates working configurations. Installed in ATmega128 using a hardware programmer. Maintainer: DDp.                                                                                                                                                       | [Update manual](http://nedopc.com/zxevo/rom/zxevo_firmware_update.pdf)                                                                                                                                                                                                    | [bootloader20120124.zip](http://nedopc.com/zxevo/rom/bootloader20120124.zip)   | [zxevo_bl.hex](http://svn.nedopc.com/filedetails.php?repname=pentevo&path=%2Favrboot%2Ftrunk%2Favr%2Fzxevo_bl.hex), [zxevo_bl.e2p](http://svn.nedopc.com/filedetails.php?repname=pentevo&path=%2Favrboot%2Ftrunk%2Favr%2Fzxevo_bl.e2p), [read_me.txt](http://svn.nedopc.com/filedetails.php?repname=pentevo&path=%2Favrboot%2Ftrunk%2Fread_me.txt) |
| **TEST&SERVICE**                               | 2016.01.04 | Service configuration for testing and setting up ZX Evolution, particularly useful for assembling a board. Installed in ATmega128 through **BOOTLOADER**. Maintainer: DDp.                                                                                           | [Service manual](http://nedopc.com/zxevo/rom/zxevo_testservice.pdf), [Flash manual](http://nedopc.com/zxevo/rom/zxevo_testservice_flash.pdf)                                                                                                                              | [testservice20160104.zip](http://nedopc.com/zxevo/rom/testservice20160104.zip) | [zxevo_fw.bin](http://svn.nedopc.com/filedetails.php?repname=pentevo&path=%2Ftest_n_service%2Ftrunk%2Frus%2Fzxevo_fw.bin)                                                                                                                                                                                                                          |

An SVN source mirror is available at [ZXEVO.RU](http://svn.zxevo.ru/listing.php?repname=pentevo).

## Third-party firmware

Contact the respective authors for support. NedoPC is not responsible for bugs or limitations in
third-party firmware.

| Name                                                         | Version          | Purpose                                                                                                                                                                                                                                | Documentation                                                 | Files                                                                                                                                             |
| ------------------------------------------------------------ | ---------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------- |
| [ZXEVO TS](http://forum.tslabs.info/viewtopic.php?f=12&t=20) |                  | Configuration with sprites and additional video modes. A complete working configuration with its own FPGA logic and ROM, plus dedicated tools, games, and other software. [Details](http://forum.tslabs.info/viewtopic.php?f=12&t=20). | [FAQ](http://forum.tslabs.info/viewtopic.php?f=12&t=157)      | [Installation instructions](http://forum.nedopc.com/viewtopic.php?f=30&t=622&start=652)                                                           |
| **ScorpEvo**                                                 | 6.1 (2011-11-05) | Scorpion Evolution was conceived as a complete alternative, and eventual replacement, for Sergey Zonov's well-known and controversial St Petersburg computer, the Scorpion ZS 256 turbo+.                                              | [scorpevo.pdf](http://nedopc.com/zxevo/scorpion/scorpevo.pdf) | [scorpevo061.zip](http://nedopc.com/zxevo/scorpion/scorpevo061.zip), [SVN](http://svn.nedopc.com/listing.php?repname=pentevo&path=%2Fscorpevo%2F) |

---

## Hardware revision history

### Revision C

Changes from the previous revision:

- Mini-ITX form factor (172×170 mm).
- Removed the onboard PAL encoder.
- Added an AY-printer interface.
- Added frequency-multiplier control.
- Added audio inputs, for three audio inputs in total.
- Changed to a QFP Z80 CPU.
- Removed the separate RGB connector; signals are available on VGA.
- Added an RS-232-to-USB bridge.
- Removed the AT power connector.
- Added a connector for an external PAL encoder.

### Revision B

Changes from the previous revision:

- Corrected the PS/2 keyboard/mouse footprint.
- Corrected wiring of the ICS501 output-enable signal (OE).
- Increased the 78M05 heatsink area.
- Added a jumper controlling the presence of 12 V on the slots.
- Added guides for the ATX power connector.
- Enlarged inductor pads.
- Reduced the board dimensions.
- Removed the controller-programming-enable jumper.
- Added universal pads for SMD and through-hole 4148 diodes.
- Added a ZXBUS pin-1 marking.
- Fixed VG93 reset by routing VG_RES to D19.
- Changed the FDD connector key to the PC convention (pin 5).
- Added “HDD Led”, “PWR Led”, “ZX Keyboard/ZX Joystick”, “Slot1”, and “Slot2” labels.
- Moved the VGA connector parallel to RGB-out.

Revision B resources:

- [Schematic](http://nedopc.com/zxevo/zxevo_sch_revb.pdf).
- [Component placement](http://nedopc.com/zxevo/zxevo_mon_revb.pdf).
- [Bill of materials](http://nedopc.com/zxevo/zxevo_bom_revb.pdf).
- [AY-printer modification](http://nedopc.com/zxevo/zxevo_ay_printer.pdf).
- [User manual](http://nedopc.com/zxevo/zxevo_user_manual_revb.pdf).
- [Assembly and startup guide](http://nedopc.com/zxevo/zxevo_solderind_and_start_revb.pdf).
- [Manufacturing files](http://nedopc.com/zxevo/zxevo_revb.zip) (PCAD2002). Commercial use requires
  agreement with [NedoPC](http://nedopc.com/aboutus.php).

### Revision A

The first, experimental revision was available only to the developers.

- [Schematic](http://nedopc.com/zxevo/zxevo_sch_reva.pdf).
- [Component placement](http://nedopc.com/zxevo/zxevo_mon_reva.pdf).
- [Bill of materials](http://nedopc.com/zxevo/zxevo_detali_reva.pdf).
- [Manufacturing files](http://nedopc.com/zxevo/zxevo_reva.zip) (PCAD2002). Commercial use requires
  agreement with [NedoPC](http://nedopc.com/aboutus.php).

---

Email: [nedopc@mail.ru](mailto:nedopc@mail.ru), [nedopc@yandex.ru](mailto:nedopc@yandex.ru).
