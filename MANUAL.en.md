# ZX Evolution developer documentation

[Русская версия](MANUAL.ru.md)

This is a programmer's reference for **ZX Evolution / PentEvo**, organized around the facilities
that extend or change a ZX Spectrum 48K. It covers the inherited ATM Turbo 2+ programming model as
well as Evolution-specific hardware and ROM services. Ordinary Z80 instructions, BASIC, and the
unchanged Spectrum 48K ROM API are outside its scope.

The hardware baseline is BASECONF FE at `a3daae83`, primarily
[`fpga/base_trdemu/trunk`](fpga/base_trdemu/trunk/) and [`avr/baseconf/trunk`](avr/baseconf/trunk/).
The ROM/RST contracts include this fork's ROM fixes. [`rom/version.a80`](rom/version.a80) still
declares ERS **0.61.01** and DOS **0.60**, so builds with the same ERS version can behave differently.
When checking a particular board, identify the source revisions used for its AVR/FPGA and ROM images.

## Reading this reference

This reference assumes familiarity with Z80 instructions and the Spectrum 48K. Start with the
section for your task.

- For C programs, start with the z88dk setup and examples in `lib/baseconf`. They also link to
  the ZEsarUX test instructions. [BaseConf library README](lib/baseconf/README.en.md)
- For direct hardware access, check which mode exposes the port and which pages the CPU can
  see. [Port index](#2-io-rules-and-complete-port-index) · [Memory maps](#3-memory-paging-rom-and-flash)
- For ROM calls, check the window 0 requirements and register conventions first. The storage
  section explains how to select a volume before using files or disk images.
  [ROM/RST API](#17-rom-and-rst-api) · [Device, FAT, and image services](#rst-storage)

The following terms and notation apply throughout the reference.

- **CPU** means the Z80 executing the program. The **FPGA** implements memory control, video, and
  most ports. The **AVR** is the ATmega128 microcontroller that services peripherals.
- A **device register** is a setting or status value accessed through a port; it need not be a Z80
  register. An **API** is a set of software calls with defined arguments and results. **ERS** means
  EVO Reset Service, the service software stored in ROM.
- A **port** is an I/O address used by `IN`/`OUT`, separate from memory accessed by `LD`. A
  **bank/page** is a physical RAM or ROM block; a **window** is the CPU address range into which
  that block is mapped.
- **Active-low** means 0 represents an asserted signal or pressed button; **active-high** means 1. A
  signal name ending in `_n` usually means active-low.
- **WAIT** makes the CPU wait for an access to finish. **Contention** means extra delays in CPU
  memory/I/O accesses while the video controller is active. **M1** is a Z80 opcode-fetch cycle; its
  refresh phase refreshes dynamic RAM.
- `BF.3` means bit 3 of port register `xxBF`; bit numbering starts at zero. `D7:0` denotes data
  bits; `A15:A8` is the high address byte. `0xFF` and `FFh` are the same hexadecimal number. In
  formulas, `&`, `|`, and `~` mean bitwise AND, OR, and inversion; `<<`/`>>` are shifts; `{...}`
  joins listed bits from most significant to least significant. For example, `{D6,D2:0}` forms four
  bits in the order D6,D2,D1,D0. `||` means logical OR.

## Contents

1. [Architecture and inheritance](#1-architecture-and-inheritance)
2. [I/O rules and complete port index](#2-io-rules-and-complete-port-index)
3. [Memory, paging, ROM, and Flash](#3-memory-paging-rom-and-flash)
4. [CPU clocks and raster timing](#4-cpu-clocks-and-raster-timing)
5. [Video, attributes, fonts, and palettes](#5-video-attributes-fonts-and-palettes)
6. [Interrupts and NMI](#6-interrupts-and-nmi)
7. [Keyboard, joystick, mouse, and tape input](#7-keyboard-joystick-mouse-and-tape-input)
8. [Sound and printer interfaces](#8-sound-and-printer-interfaces)
9. [SD and IDE storage](#9-sd-and-ide-storage)
10. [Floppy controller and hardware-assisted emulation](#10-floppy-controller-and-hardware-assisted-emulation)
11. [RTC, NVRAM, firmware identification, and EEPROM](#11-rtc-nvram-firmware-identification-and-eeprom)
12. [RS-232 UART](#12-rs-232-uart)
13. [Reset, boot, and ERS-owned memory](#13-reset-boot-and-ers-owned-memory)
14. [AVR/FPGA interface and firmware development](#14-avrfpga-interface-and-firmware-development)
15. [Documented extensions and source discrepancies](#15-documented-extensions-and-source-discrepancies)
16. [Source map](#16-source-map)
17. [ROM and RST API](#17-rom-and-rst-api)

## 1. Architecture and inheritance

| Subsystem        | Difference from a Spectrum 48K                                                                                          | Origin / implementation                                                     |
| ---------------- | ----------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------- |
| CPU              | 3.5, 7, and 14 MHz; memory/I/O stalls and selectable contention                                                         | Evolution FPGA clock and memory controllers                                 |
| RAM              | 4 MiB in 256 physical 16 KiB pages; four independently mapped CPU windows                                               | ATM pager, extended to 4 MiB                                                |
| ROM              | 512 KiB Flash, pageable into any CPU window; software-controlled write gate                                             | ATM mapping plus Evolution Flash gate                                       |
| Memory models    | 48K, 128K, Pentagon 1024K; two programmable maps selected by `7FFD.4`                                                   | 128K/Pentagon/ATM inheritance                                               |
| I/O              | At least eight low address bits decoded; normal and shadow port maps                                                    | ATM/Beta Disk model, extended by Evolution                                  |
| Video            | Alternate screen, per-byte attributes, 16-colour graphics, 80-column text, writable font                                | Pentagon and ATM modes, Evolution additions                                 |
| Palette          | 16 programmable colours; RGB444 extension; ULAplus registers                                                            | ATM palette and Evolution extensions; see implementation restrictions below |
| Interrupts       | Frame interrupt, NMI page overlay, and NMI request on a selected instruction address                                    | Evolution; newer interrupt API is version-dependent                         |
| Input            | PS/2 keyboard translated into a Spectrum matrix; raw scan-code access; mechanical keyboard; Kempston joystick and mouse | AVR services plus FPGA input latches                                        |
| Audio            | AY/YM, selectable beeper/tape output, 8-bit PWM Covox                                                                   | 128K/ATM-related peripherals, Evolution implementation                      |
| Storage          | Beta Disk/VG93, SD SPI, Nemo IDE with sequential byte transfers, virtual floppy images and RAM disk                     | FPGA ports plus ERS drivers                                                 |
| Persistent state | Gluk-compatible RTC/NVRAM, AVR EEPROM access, build identification                                                      | AVR emulation over PCF8583                                                  |
| Serial           | Polling 16550-like register interface backed by AVR UART                                                                | Kondratyev register layout, Evolution implementation                        |
| Firmware         | AVR bootloader and embedded FPGA image, independent Z80 Flash ROM, ERS `RST 08h` API                                    | Evolution                                                                   |
| Expansion        | Two ZXBUS slots; motherboard ports take priority                                                                        | Evolution bus arbitration                                                   |

ATM compatibility is **subsystem-specific**. Inherited features include two memory maps, `xx77`
control, `xFF7` paging, DOS/shadow entry, palette writes through `xxFF`, and the 320×200, 640×200,
and 80×25 displays. Do not assume that ATM's i8031 keyboard/serial protocol, ADC, Centronics,
external-device port, attribute port, or original IDE interface exists merely because it appears in
[`docs/atm2_arch.pdf`](docs/atm2_arch.pdf). Evolution uses its own AVR, UART, mouse, RTC, SD, and
Nemo IDE interfaces. AY printer support is a separate attachment and ROM driver.

This guide describes BASECONF FE, not TS-Conf or ScorpEvo. It does not imply sprites, tile maps, a
blitter, or a DMA API in this FPGA configuration.

## 2. I/O rules and complete port index

<a id="shadow-mode"></a>

### 2.1 Notation, address decoding, and shadow mode

Port addresses in the tables are hexadecimal. `xxFE` means any address whose low byte is FE; the
high byte can vary **unless its bits are assigned a function in the description**. In patterns such
as `xFF7`, each `x` is one unspecified hexadecimal digit; the supported complete addresses are
listed nearby.

`R` means read, `W` means write, and `RW` means both. These labels describe supported operations. An
electrical response to another operation does not necessarily have a defined software meaning. Use
the listed 16-bit addresses. Partial address decoding may let a device respond at additional
addresses (aliases), but those aliases can change in another FPGA version.

Unlike a 48K ULA, an arbitrary even port is not necessarily a border/keyboard port. There are
deliberate exceptions, notably `xxF6` and `xxFC`; these are described separately. Prefer
`LD BC,port` followed by `IN A,(C)` / `OUT (C),A` when the high byte matters. With
`IN A,(n)` and `OUT (n),A`, the high address byte is the value of `A` **before** the instruction
executes. Block I/O changes `B` between accesses, which matters for the shadow SD interface.

The FPGA enables shadow mode when its internal DOS signal is active or bit 0 of port BF is set:

```text
shadow = DOS || BF.bit0
```

DOS here is a hardware state controlling port access and page selection. It does not mean that the
program is currently executing a disk command.

"Shadow ports" are I/O registers available only in this state. For example, reading low port `1Fh`
returns joystick state in normal mode and floppy-controller status in shadow mode. The same I/O
address selects a different device according to machine state.

`BF.0` forces access to shadow ports without forcing a DOS ROM into memory. Clearing it does not
close the shadow ports if DOS is still active. DOS can be held active by writing `xx77` with address
bit A9=0. It can also be entered by fetching a ROM opcode at page offset `3Dxx` with the paging
settings described in §3.4. Memory selection and shadow-port visibility are therefore related but
are not the same state.

An operation that needs normal `EFF7`, `DFF7`, or `BFF7` must run with `DOS=0` and `BF.0=0`. In
shadow mode, the latter two addresses are ATM pager writes. RTC shadow access uses `DEF7` / `BEF7`.
In this source, `EEF7` does not select the `EFF7` register.

### 2.2 Complete Z80 port index

| Port(s)                                                | Access        | Visibility         | Subsystem / purpose                                                                                   |
| ------------------------------------------------------ | ------------- | ------------------ | ----------------------------------------------------------------------------------------------------- |
| `xxFE`                                                 | RW            | Always             | [Spectrum keyboard/tape input, border, beeper/tape output](#keyboard)                                 |
| `xxF6`                                                 | RW            | Always             | [Keyboard/tape input alias; border colours 8–15 on write; no beeper strobe here](#border)             |
| `xxFC`                                                 | W             | Always             | [Implementation alias affecting border and, when A15=0, `7FFD`; avoid in new code](#spectrum-control) |
| `7FFD`                                                 | W             | Always             | [Active memory map, screen, RAM bank, 48K paging lock](#spectrum-control)                             |
| `EFF7`                                                 | W             | Normal only        | [Video modifiers, 128K/1024K selection, RAM0 overlay, clock and RTC gate](#spectrum-control)          |
| `xxBF`                                                 | RW            | Always             | [Shadow, Flash/font writes, NMI, breakpoint, RGB444 controls](#bf-control)                            |
| `xx77`                                                 | W             | Shadow             | [ATM video/clock control; address bits control pager, DOS hold, palette writes](#atm-control)         |
| `3FF7`, `7FF7`, `BFF7`, `FFF7`                         | W             | Shadow             | [ATM page/type/substitution for windows 0–3 of active map](#window-mapping)                           |
| `37F7`, `77F7`, `B7F7`, `F7F7`                         | W             | Shadow             | [Full 8-bit inverted RAM page for windows 0–3](#window-mapping)                                       |
| `3BF7`, `7BF7`, `BBF7`, `FBF7`                         | W             | Shadow             | [Per-window write protection for active map](#window-mapping)                                         |
| `00BD`–`0FBD`, `12BD`                                  | R             | Always             | [Configuration, page, palette, font, border, protection readback](#configuration-readback)            |
| `10BD`, `11BD`                                         | RW            | Always             | [Instruction address for NMI request, low/high byte](#execution-breakpoint)                           |
| `13BD`                                                 | RW            | Always             | [Drive mask for hardware-assisted floppy emulation](#floppy-overlay)                                  |
| `xxBE`                                                 | W             | Always             | [Leave NMI overlay / floppy-emulator overlay](#nmi-entry)                                             |
| `BF3B`                                                 | W             | Always             | [ULAplus palette index or mode-register selection](#rgb444)                                           |
| `FF3B`                                                 | W             | Always             | [ULAplus palette data or mode](#rgb444)                                                               |
| `xx3B`                                                 | R             | Always             | [Last data byte written with A14=1; not selected palette-entry readback](#rgb444)                     |
| `FFFD`                                                 | RW            | Always             | [AY register selection (W), selected-register data (R)](#ay-sound)                                    |
| `BFFD`                                                 | W             | Always             | [AY register data](#ay-sound)                                                                         |
| `xxFB`                                                 | W             | Always             | [8-bit unsigned Covox sample](#beeper-covox)                                                          |
| `xx1F`                                                 | R             | Normal only        | [Kempston joystick](#joystick)                                                                        |
| `FADF`, `FBDF`, `FFDF`                                 | R             | Always             | [Mouse buttons/wheel, X, Y](#mouse)                                                                   |
| `DFF7`, `BFF7`                                         | W, RW         | Normal, `EFF7.7=1` | [RTC address, RTC data](#rtc-access)                                                                  |
| `DEF7`, `BEF7`                                         | W, RW         | Shadow             | [RTC address, RTC data, independent of `EFF7.7`](#rtc-access)                                         |
| `xx77`                                                 | RW            | Normal only        | [SD chip select (W); constant zero status (R)](#sd-interface)                                         |
| `xx57`                                                 | RW            | Always             | [SD transfer; shadow writes with A15=1 set chip select](#sd-interface)                                |
| `xx10`, `xx11`                                         | RW            | Always             | [IDE low/sequential data, IDE high-byte latch](#ide-interface)                                        |
| `xx30`, `xx50`, `xx70`, `xx90`, `xxB0`, `xxD0`, `xxF0` | RW            | Always             | [IDE task file](#ide-interface)                                                                       |
| `xxC8`                                                 | RW            | Always             | [IDE alternate status / device control](#ide-interface)                                               |
| `xx1F`, `xx3F`, `xx5F`, `xx7F`                         | RW            | Shadow             | [VG93 command/status, track, sector, data](#floppy-registers)                                         |
| `xxFF`                                                 | RW            | Shadow             | [FDD system latch/status; palette write when enabled](#floppy-registers)                              |
| `F8EF`–`FFEF`                                          | RW / R        | Always             | [UART register bank; AVR-serviced WAIT cycles](#uart-interface)                                       |
| `14BD`–`16BD`                                          | `newirq` tree | See §15            | [`newirq` interrupt controller; registers absent from `trunk`](#enhanced-interrupts)                  |

This index lists the supported motherboard interfaces and the `FC` alias, whose writes can change
more than one setting. The decoder also accepts additional IDE aliases; §9.2 gives their
implementation and the reason to use the canonical addresses.

### 2.3 ZXBUS priority and unmapped reads

When the Z80 executes `IN` or `OUT`, the FPGA uses the port address and current machine mode to
determine whether the access belongs to an onboard device. If it recognizes an onboard port, it
blocks the I/O request signal `IORQ` from reaching both ZXBUS slots. An expansion card therefore
does not receive that access through the standard bus interface. For example, an external IDE
controller using the same port addresses cannot service accesses in place of the onboard controller.
Onboard keyboard, SD, and mouse ports have the same priority.

For accesses passed to ZXBUS, slot priority applies: a card in slot 1 can use `IORQGE` to prevent
`IORQ` from reaching slot 2. The service order is therefore onboard devices, then slot 1, then
slot 2.

The FPGA sometimes recognizes an entire address group by its low byte, even though not every
high-byte value selects an implemented register. For example, all `xxBD` addresses are blocked from
ZXBUS in the baseline, including addresses without an implemented register. Expansion cards cannot
use those addresses through the standard bus interface.

Covox is a separate case: the FPGA handles writes to `xxFB`, but that address is absent from its
ZXBUS-blocking condition (`porthit`). The I/O request for that port can therefore also reach
expansion cards.

Reading a port serviced by neither an onboard device nor an expansion card returns `FF`. Interrupt
acknowledge also drives `FF` in this baseline. This differs from the original 48K floating bus.
Undefined `BD` register values are not a dependable feature-detection result.

Sources: [`zports.v`](fpga/base_trdemu/trunk/z80/zports.v),
[`zbus.v`](fpga/base_trdemu/trunk/z80/zbus.v), [`top.v`](fpga/base_trdemu/trunk/top.v).

## 3. Memory, paging, ROM, and Flash

### 3.1 Physical pages versus ROM source constants

The Z80 address space is divided into four 16 KiB ranges called windows: 0=`0000–3FFF`,
1=`4000–7FFF`, 2=`8000–BFFF`, 3=`C000–FFFF`. The page manager selects which physical RAM or ROM
block the CPU sees in each window. The settings for all four windows together form a memory map.

The FPGA stores two memory maps, numbered 0 and 1. A program can configure them differently in
advance and then switch between them. The CPU uses only one map at a time, called the active map.
Switching maps applies the selected map's settings to all four windows at once. Section 3.5 explains
how a program selects the active map.

RAM has 256 physical pages. Flash has 32 physical banks.

The C library's window-mapping calls accept physical RAM page numbers `0..255`.
`bc_ram_page_pool_t` and the banked heap that uses it cover pages `0..63`, one bit per page in
an eight-byte mask. [memory.h](lib/baseconf/include/baseconf/memory.h)

The page-number inputs to the ATM/Evolution ports are inverted. Keep three different concepts
distinct:

| Name used here              | Meaning                                                                          | Example                                                                   |
| --------------------------- | -------------------------------------------------------------------------------- | ------------------------------------------------------------------------- |
| Physical RAM page           | Linear DRAM page, `0..FF`                                                        | NMI overlay is physical `FF`                                              |
| ERS ROM-source RAM constant | Number passed as the data byte to the `x7F7` page selector; `FF - physical_page` | `RAM_NMI=00` selects physical `FF`; `RAM_EVODOS=01` selects physical `FE` |
| ERS ROM constant            | Value written to `xFF7`, with type/control bits added where needed               | `ROM_ERS=0` selects the last Flash bank; `ROM_RST83=9` selects bank `16h` |

`RAM_NMI` is a constant symbol declared with `SETVAR` in
[`rom/global_vars.a80`](rom/global_vars.a80). Its value is `00h`; ERS code uses it as data when
selecting a RAM page. The port address is supplied separately. Because page numbering is inverted,
writing `00h` to the `x7F7` selector chooses physical RAM page `FF`.

For a plain ROM constant `r=00h..1Fh`, physical Flash bank is `1Fh-r`. For a linear ROM file, its 16
KiB block offset is `(1Fh-r)*4000h`. The reset service occupies the **final** 16 KiB block of the
512 KiB image.

RAM pages and Flash banks belong to separate memory spaces. `RAM_NMI=00` is a RAM constant that
selects DRAM page `FF`, which holds the NMI/RST handler and ERS workspace. `ROM_ERS=00` is a ROM
constant that selects Flash bank `1F`, which contains the reset service. The source directory
`rom/page0` corresponds to the `ROM_ERS` ROM slot; physical RAM page `00` is a separate DRAM page.

In the rest of the hardware chapters, page numbers are **physical RAM pages** unless explicitly
called port values or ERS constants.

<a id="bf-control"></a>

### 3.2 `xxBF`: Evolution configuration

Port `xxBF` is available in both normal and shadow mode. A write sets all the bits below at once; a
read returns the stored settings.

| Bit | Effect                                                                                         | Hardware reset           |
| --- | ---------------------------------------------------------------------------------------------- | ------------------------ |
| 0   | 1 opens shadow ports without switching ROM to TR-DOS                                           | 0                        |
| 1   | 1 permits CPU writes to reach the Flash chip; 0 blocks them                                    | 0                        |
| 2   | 1 copies every CPU memory write into font RAM as well                                          | 0                        |
| 3   | A **1→0 transition** requests NMI; writing 1 alone does not trigger it                         | 0                        |
| 4   | 1 compares opcode-fetch addresses with the configured breakpoint address; a match requests NMI | 0                        |
| 5   | 1 selects four bits per R, G, B palette component; 0 selects two                               | 0                        |
| 7:6 | Reserved; write zero                                                                           | 0 on read in this source |

**Bit 0 — shadow-port access.** This allows access to the ATM pager, floppy controller, and other
shadow ports even while executing from RAM without entering TR-DOS. Clearing the bit restores normal
port access only if the hardware DOS signal is also inactive. See
[normal and shadow port modes](#shadow-mode).

**Bit 1 — Flash write permission.** When a Flash bank is mapped into a CPU window, BF.1=0 prevents
writes to that address range from reaching the chip. BF.1=1 removes this global block, but the
window's own write protection still applies. The bit does not erase or program Flash by itself:
software must issue the chip's programming commands. See [Flash programming](#flash-programming).

**Bit 2 — font loading.** While set, every CPU memory write also stores the same byte in font RAM at
`CPU_address & 07FFh`. For example, a RAM write to 8000h also changes font byte 000h. The original
RAM write still occurs. Even an interrupt's return-address stack writes can therefore corrupt the
font; enable this bit only for the upload. See [font layout and loading](#font-ram).

**Bit 3 — software NMI request.** Write 1 and then 0 to this bit, preserving the other BF settings.
The FPGA remembers the falling edge and signals a non-maskable interrupt to the Z80 at the start of
the next frame interrupt. See [NMI entry and the handler-page override](#nmi-entry).

**Bit 4 — NMI request on an instruction address.** Write the low address byte to port 10BD and the
high byte to 11BD. With BF.4=1, the FPGA compares each opcode-fetch address with that 16-bit
address. A match requests NMI immediately. Data reads and writes at the address do not trigger it.
See [NMI request address and behavior](#execution-breakpoint).

**Bit 5 — palette component precision.** The normal format uses two bits for each of R, G, and B,
allowing 64 colour combinations. RGB444 uses four bits per component, allowing 4096 combinations.
The ATM palette still has 16 entries. Setting this bit does not upload a new palette: it changes the
colour-write format at xxFF and extended-colour processing. See
[RGB444 encoding, address bits, and output limitations](#rgb444).

All implemented fields can be read back in this source. Preserve other bits when setting one
feature: writing `01` just to open shadow ports also disables Flash/font writes, the breakpoint, and
extended colour. A read-modify-write that clears bit 3 intentionally or accidentally can request
NMI.

<a id="atm-control"></a>

### 3.3 `xx77`: ATM system control

To write `xx77`, two conditions must hold: shadow ports are already open and the low address byte is
`77`. In `OUT (C),A`, `BC` supplies the port address and `A` supplies the data byte. So in `FD77`,
`FD` is the high byte of the **address** and `77` is its low byte. Neither is the data byte.

The FPGA handles parts of the operation separately:

- In the address high byte `FD`, bits A14, A9, and A8 matter. Here A9=0 holds the DOS hardware
  state, A8=1 keeps the memory pager enabled, and A14=1 disables palette writes through `xxFF`.
  Other address bits do not set a mode in this function.
- In the **data byte** placed in `A`, D2:D0 select the video mode and D3 selects 14 MHz. This mode
  latch does not use D7:D4.

For example, writing value `03` to port `FD77` selects the ZX video family (D2:D0=3), uses `EFF7.4`
to choose 7 or 3.5 MHz (D3=0), holds the DOS state (address A9=0), keeps the pager enabled (A8=1),
and disables palette writes (A14=1). `03` is the **data byte**; `FD77` is the **port address**. The
data value is not video-memory content.

```asm
        ld bc,0xFD77      ; address: A9=0 holds DOS, A8=1 enables the pager
        ld a,0x03         ; data: D2:D0=3, ZX video family; D3=0
        out (c),a
```

`BF.0` and address bit A9 in `xx77` serve different purposes. `BF.0=1` forces shadow-port access
without changing the DOS state; this is useful when a RAM program needs to configure the pager.
Address A9=0 holds the DOS state itself. Clearing `BF.0` therefore does not close shadow ports while
DOS is held through A9. Conversely, A9=1 allows normal automatic DOS entry and exit, but `BF.0=1`
still keeps shadow ports open. A9 alone does not select a DOS ROM: the window mapping and pager
rules below determine which ROM is visible.

The canonical addresses below keep A8=1 (pager enabled). The palette column shows the effect of A14.
The DOS column describes A9: “held” means A9=0; “automatic” means A9=1.

| Port address | A9 / DOS state              | A14 / palette writes |
| ------------ | --------------------------- | -------------------- |
| `FD77`       | A9=0, hold DOS              | A14=1, disabled      |
| `BD77`       | A9=0, hold DOS              | A14=0, enabled       |
| `BF77`       | A9=1, normal automatic mode | A14=0, enabled       |
| `FF77`       | A9=1, normal automatic mode | A14=1, disabled      |

To disable the pager, clear A8 in the address. `FC77` clears A8; `FD77` keeps it set. Keep the
control **port address** separate from the data byte in `A`. Whenever software changes the data
byte, it must preserve D2:D0 (video mode) and D3 (14 MHz), or those settings change too. See
[memory maps](#window-mapping), [video modes](#video-modes), and the
[palette procedure](#atm-palette).

At hardware reset the pager is disabled, DOS is held active, video mode is 3, and 14 MHz is off. The
ROM startup code must configure both memory maps before enabling the pager. Hardware reset alone
does not initialize every page number, RAM/ROM selection, and substitution setting to a usable
memory map.

<a id="window-mapping"></a>

### 3.4 Mapping a window

The CPU address space is divided into four 16 KiB windows. The CPU sees one active map of four
windows at a time; `7FFD.4` selects the map (§3.5). A write to one of the ports below changes one
window in the active map. The two maps store their settings separately.

Each row lists three ports because each port changes a different window setting. `x` stands for one
hexadecimal digit determined by the window; the complete addresses for all four windows are shown.
For window 3 (`C000–FFFF`), the ports are `FFF7`, `F7F7`, and `FBF7`.

| CPU window     | Set type and base page | Set full RAM page | Protect from writes |
| -------------- | ---------------------- | ----------------- | ------------------- |
| 0: `0000–3FFF` | `3FF7`                 | `37F7`            | `3BF7`              |
| 1: `4000–7FFF` | `7FF7`                 | `77F7`            | `7BF7`              |
| 2: `8000–BFFF` | `BFF7`                 | `B7F7`            | `BBF7`              |
| 3: `C000–FFFF` | `FFF7`                 | `F7F7`            | `FBF7`              |

The diagram below shows bits of the **port address**. `A15` is the most significant address bit and
`A0` the least significant. `A15:A14` select the window; the other shown address bits select one of
its three registers.

| Address family | `A15:A14` window  | `A13:A12` | `A11:A10` register | `A9:A8` | `A7:A0` |
| -------------- | ----------------- | --------- | ------------------ | ------- | ------- |
| `xFF7`         | 00, 01, 10, or 11 | `11`      | `11`               | `11`    | `F7`    |
| `x7F7`         | 00, 01, 10, or 11 | `11`      | `01`               | `11`    | `F7`    |
| `xBF7`         | 00, 01, 10, or 11 | `11`      | `10`               | `11`    | `F7`    |

For example, in `FBF7`, `A15:A14=11` selects window 3 and `A11:A10=10` selects its protection
register. `A13:A12=11`, `A9:A8=11`, and low byte `F7` are fixed. These address bits **do not enable
protection**. One data bit sets it: `D0=1` enables protection, `D0=0` clears it; `D7:D1` are
ignored.

**The address selects a register; the data byte is supplied separately.** With `OUT (C),A`, `BC`
contains the port address and `A` contains the setting byte. For example, writing data `40h` to
`FFF7` writes `40h` into window 3's type/base-page register. The number `FFF7` is not a RAM page
number.

These ports require shadow mode and change only the active map. To configure the other map, select
it with `7FFD.4`, make the writes, then switch back if needed. Selecting a map immediately applies
all four of its window settings.

**1. Select RAM or ROM and page substitution (`xFF7`).**

The FPGA interprets the data byte `D` written to this port as follows:

- D6=1 selects RAM; D6=0 selects Flash ROM.
- D7=1 allows part of the page number to come from `7FFD` (for RAM) or the DOS signal (for ROM);
  D7=0 uses the base page without substitution.
- D5:D0 set an inverted six-bit base page number: physical page is `(~D) & 3Fh`. Here `D` is the
  byte written to the port, and `~` inverts that byte's bits.

Data-byte layouts for the three window registers:

| Data port | Used bits | Meaning                                               |
| --------- | --------- | ----------------------------------------------------- |
| `xFF7`    | `D7`      | 1 enables substitution from `7FFD`/DOS; 0 disables it |
| `xFF7`    | `D6`      | 1 selects RAM; 0 selects Flash ROM                    |
| `xFF7`    | `D5:D0`   | Inverted base page number                             |
| `x7F7`    | `D7:D0`   | Inverted full RAM page number (8 bits)                |
| `xBF7`    | `D0`      | 1 enables protection; 0 allows writes                 |
| `xBF7`    | `D7:D1`   | Ignored                                               |

A write to `xFF7` replaces the stored page number completely: the FPGA sets the top two page bits to
zero and loads the inverse of D5:D0 into the lower six bits. Thus, with D7=0, a write to `xFF7` sets
the physical RAM page in range `00h–3Fh`. This is already the complete page number for that range,
not a low part to be combined with a separate value. For example, data `7Ah` to `FFF7` selects RAM
page `05h`: `(~7Ah) & 3Fh = 05h`. ROM uses inverted ERS constants; the reset-service source contains
mapping examples.

**2. Set the upper RAM page bits when needed (`x7F7`).**

The data byte written to this port is the inverted full RAM page number: physical page is `~D`
(eight bits). This write also selects RAM. For physical page 80h, write `7Fh` because `~7Fh = 80h`.
`x7F7` sets all eight page bits, including the upper two, so pages above `3Fh` require two
sequential settings, but their values are **not combined**: first use `xFF7` to select RAM and
disable substitution, then `x7F7` replaces the page number with the full eight-bit value.

A later write to `xFF7` also replaces the page number completely, but sets its top two bits to `00`.
So writing only `xFF7` after selecting a page above `3Fh` returns the mapping to the `00h–3Fh`
range. If substitution is already disabled and you want to change one page above `3Fh` to another, a
new `x7F7` write is enough. But `x7F7` does **not** clear D7 substitution previously set through
`xFF7`; if it may be on, first write `xFF7` with D7=0.

The formulas show how the pager calculates a physical RAM page when `xFF7` enables substitution from
`7FFD` (D7=1). `EFF7.2` selects the address scheme: 1=128K, 0=1024K. When the `7FFD` bank changes,
its bits fill the shown page positions; the remaining bits come from the window's base page:

```text
128K:   page = (base_page & F8h) | (7FFD & 07h)
1024K:  page = (base_page & C0h) | ((7FFD >> 2) & 38h) | (7FFD & 07h)
```

With `EFF7.2=1`, 128K page addressing is selected and three low page bits come from `7FFD`. With
`EFF7.2=0`, 1024K page addressing is selected and six bits come from `7FFD`. This is the
page-addressing scheme, not the amount of RAM physically installed; `7FFD` supplies the substituted
bit values. To disable substitution, set D7=0; the pager then uses the window settings for the page
number. See [§3.5](#spectrum-control) for the memory mode and `7FFD` bit meanings.

**3. Protect the window from writes (`xBF7`).**

Write `01h` to prevent the CPU from changing RAM in this window; write `00h` to remove protection.
Only D0 of the data byte is checked. For window 3, write `01h` to `FBF7` to protect `C000–FFFF`, or
`00h` to `FBF7` to permit writes. `FBF7` selects window 3's protection register; `01h` or `00h`
supplies its new state. Protection applies only to the active map. It also blocks Flash writes
through this window, even if Flash writes are enabled through `BF.1`. Hardware reset clears window
protection.

**Example: map physical RAM page 80h at C000.**

Before running this example, open the shadow ports and make sure the page manager is enabled. From RAM, you can
open the shadow ports by setting `BF.0` while preserving the other bits. Keep the running code,
stack and data used during the switch outside `C000–FFFF`. If an INT or NMI handler uses this
window, keep it from running while the temporary mapping is active. Save the window settings
and BF before changing them. When finished, restore the window before writing back BF.
[ATM system control](#atm-control) · [Configuration readback](#configuration-readback)

First write `40h` to `FFF7` (RAM with substitution disabled), then `7Fh` to `F7F7` (the inverted
page number for physical page 80h). The final two writes demonstrate setting and clearing write
protection.

```asm
        ld bc,0xFFF7      ; window 3 type/base-page register
        ld a,0x40         ; data: RAM, substitution disabled
        out (c),a
        ld bc,0xF7F7      ; window 3 full RAM page register
        ld a,0x7F         ; data: inverted physical page 80h
        out (c),a

        ld bc,0xFBF7      ; window 3 write-protection register
        ld a,0x01         ; prevent CPU writes to C000–FFFF
        out (c),a
        xor a             ; A=00: allow writes again
        out (c),a
```

Forced RAM0 and NMI mappings have priority over a normal window and can bypass its protection. The
FE floppy-emulation overlay has its own temporary write block.

<a id="spectrum-control"></a>

### 3.5 `7FFD`, `EFF7`, and the normal Spectrum map

`7FFD` is a control port to which the program writes a byte of memory and screen settings. One bit
selects the active memory map: bit 4=0 selects map 0, and bit 4=1 selects map 1. This bit is written
as `7FFD.4` below. Changing maps applies the previously stored settings for all four windows at
once; the program does not need to rewrite each window's settings.

The remaining bits are listed in the table below. `7FFD` itself is write-only; the last stored byte
can be read through port `0ABD`.

| `7FFD` field | 128K mode                                                                             | Pentagon 1024K mode                    |
| ------------ | ------------------------------------------------------------------------------------- | -------------------------------------- |
| 2:0          | Low three substituted RAM page bits                                                   | Low three of six substituted page bits |
| 3            | 0 displays screen set 0, 1 displays set 1; [pages depend on video mode](#video-modes) | Same                                   |
| 4            | Active programmable map 0/1                                                           | Active programmable map 0/1            |
| 5            | Lock further `7FFD` writes when `EFF7.2=1`                                            | RAM page bit 3                         |
| 7:6          | Not used for 128K page number                                                         | RAM page bits 5:4                      |

The lock depends on the 128K/1024K setting, so returning to 1024K mode can release it. The raw
stored byte and the values exposed to downstream logic are not always identical: the FPGA masks
expansion bits in 128K mode. Use `0ABD` for saved-register state.

| Bit  | Purpose                           | Effect                                                                                                                                                                       |
| ---- | --------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 0    | 16-colour graphics mode           | With `xx77.D2:0=3`, 1 selects 256×192 with a colour per pixel; 0 lets bit 5 select the format. [Modes](#video-modes), [byte layout](#video-addressing)                       |
| 2    | RAM address scheme: 128K or 1024K | 1 substitutes three low page bits from `7FFD` and enables the `7FFD.5` lock; 0 substitutes six bits and disables the lock. Applies to windows with page substitution enabled |
| 3    | Temporary RAM0 overlay            | 1 shows physical RAM page 0 at `0000–3FFF`; 0 restores window 0's mapping. Higher-priority overlays are listed below                                                         |
| 4    | CPU clock selection               | 1=3.5 MHz, 0=7 MHz, unless `xx77.D3` selects 14 MHz                                                                                                                          |
| 5    | ZX 8×1 attributes                 | With `xx77.D2:0=3` and bit 0=0, 1 gives each 8×1 pixel group its own attribute; 0 uses standard 8×8 attributes. [Attribute addresses](#video-addressing)                     |
| 7    | Normal-mode RTC access            | 1 enables index selection through `DFF7` and data access through `BFF7`; 0 disables them. Shadow `DEF7`/`BEF7` do not depend on this bit. [Access sequence](#rtc-access)     |
| 1, 6 | Unassigned                        | No function established in this source; preserve or write zero                                                                                                               |

`EFF7` resets to zero and is writable only in normal mode. For overlays, pager-disabled service ROM
has priority; with the pager enabled, NMI physical page `FF` wins over FE-emulation page `FE`, which
wins over RAM0.

ERS configures the familiar map: RAM 5 at `4000`, RAM 2 at `8000`, a bank selected by `7FFD` at
`C000`, and BASIC/DOS ROM at `0000`. Its ROM set uses inverted ROM constants use inverted values
relative to physical Flash numbering. The Spectrum map is a **software setup of the ATM pager**; the
hardware supports other maps as well.

<a id="configuration-readback"></a>

### 3.6 Configuration readback

| Port           | Result                                                                                                                                       |
| -------------- | -------------------------------------------------------------------------------------------------------------------------------------------- |
| `00BD`–`03BD`  | Inverted base-page bytes for map 0, windows 0–3                                                                                              |
| `04BD`–`07BD`  | Inverted base-page bytes for map 1, windows 0–3                                                                                              |
| `08BD`         | `ramnrom` bits, bit `map*4+window`; 1=RAM                                                                                                    |
| `09BD`         | Per window: 1 enables page-bit substitution from `7FFD` or DOS, 0 uses the base page; [substitution rules](#window-mapping)                  |
| `0ABD`         | Raw stored `7FFD` byte                                                                                                                       |
| `0BBD`         | Raw stored `EFF7` byte                                                                                                                       |
| `0CBD`         | D7=A14, D6=A9, D5=A8 from `xx77`; D4=current DOS; D3:0=last video/turbo bits                                                                 |
| `0DBD`         | Current displayed palette colour in the `xxFF` bit layout; see §5                                                                            |
| `0EBD`         | Current text glyph scan-line byte, not indexed font RAM access                                                                               |
| `0FBD`         | Border colour in bits 3:0; upper bits undefined                                                                                              |
| `10BD`, `11BD` | Low/high byte of the instruction address whose fetch triggers NMI when `BF.4=1`; [execution breakpoint](#execution-breakpoint)               |
| `12BD`         | One bit per window: 1 prevents writes, 0 permits them. Same bit order as `08BD`; change through `xBF7`                                       |
| `13BD`         | D0–D3 enable interception for floppy drives A–D respectively (1=intercept); upper bits undefined. [Interception conditions](#floppy-overlay) |

For `08BD`, `09BD`, and `12BD`, bits 0–3 describe windows 0–3 of map 0, and bits 4–7 describe
windows 0–3 of map 1. For example, bit 6 describes window 2 of map 1. The address bits in `0CBD` are
settings retained from the last `OUT xx77`, not the address of the current `IN`. `0DBD` and `0EBD`
depend on what the video controller is displaying at the time of the read; neither port lets
software select a colour or character to read.

Page-number reads return the inverse of the **stored base page number**. They do not apply
replacement bits from `7FFD`, the DOS signal, or temporary NMI/emulator page overrides. The result
therefore need not identify the physical page currently visible to the CPU. In this source, `12BD`
is **read-only**; change protection through the `xBF7` writes.

Each map is read through its own ports **whatever map is active**: while code runs in map 0,
`04BD`–`07BD` still return the settings of map 1, so a read does not require switching. Because the
value is inverted, `IN` from `0?BD` masked with `3Fh` gives exactly the six page bits written to
`xFF7` (or the full number after a `x7F7` write).

To restore both maps, select each map with `7FFD.4`, restore RAM/ROM and substitution through
`xFF7`, extend RAM page numbers through `x7F7` where necessary, and restore protection through
`xBF7`. Finally restore `EFF7`, `7FFD`, `xx77`, and `BF` with the required shadow visibility and
with executing code and stack still accessible. Restoring `7FFD` too early can select the wrong map
or lock out subsequent writes.

<a id="configuration-readback-port"></a>

**The read-back port depends on the FPGA build.** This document describes
[`fpga/base_trdemu`](fpga/base_trdemu/trunk/), where the configuration is read at `xxBD`
(`ZXEVBD = 8'hBD`, the `portbdmux` table in
[`zports.v`](fpga/base_trdemu/trunk/z80/zports.v)). In the
[`fpga/baseconf`](fpga/baseconf/trunk/) and [`fpga/sdload`](fpga/sdload/trunk/) builds the same
multiplexer moved to `xxBE` (`ZXEVBE`, `portbemux`), and `xxBD` became the execution-breakpoint
address port (`ZXEVBRK`). Software that has to run on both builds reads `xxBD` and, if the value
does not match what was written, repeats the read from `xxBE`.

<a id="flash-programming"></a>

### 3.7 Flash programming

`BF.1` is a write gate, not an erase/program command. A mapped Flash bank remains subject to the
Flash chip's command sequences, erase geometry, and ID. The repository contains datasheets for
several fitted Flash parts; the command algorithm must match the actual device. A normal
`LD (address),A` with the gate open does not turn Flash into RAM. Use the ROM flasher implementation
in [`rom/mainmenu/src`](rom/mainmenu/src/) as the firmware-level reference.

Program from RAM, arrange the necessary Flash banks without removing executing code, disable
font-write mirroring, and restore the write gate/protection after the operation. The lower-address
64 KiB custom-ROM region is distinct from the AVR firmware image.

Whether the commands reach the chip can be checked with the autoselect mode (`AAh`/`55h`/`90h`), which
only reads. The Am29F040B answers as follows (Table 3 of the
[datasheet](docs/datasheets/am29f040b.pdf)): `A1=0,A0=0` gives the manufacturer `01h`, `A1=0,A0=1` the
device `A4h`, and with `A1=1` the chip returns the **sector protection bit** (`01h` protected, `00h`
not), where `A0` no longer matters. Addresses `XX02` and `XX03` therefore repeat the same byte, so four
different bytes are not required and checking that the device code repeats proves nothing. The device
code of compatible parts may differ from `A4h`; check the datasheet of the part actually fitted. The
`AAh`/`55h` unlock writes need delays the chip can follow, otherwise there is no answer at all.

Sources: [`atm_pager.v`](fpga/base_trdemu/trunk/mem/atm_pager.v),
[`zdos.v`](fpga/base_trdemu/trunk/z80/zdos.v), [`zmem.v`](fpga/base_trdemu/trunk/z80/zmem.v),
[`zports.v`](fpga/base_trdemu/trunk/z80/zports.v), [`global_vars.a80`](rom/global_vars.a80),
[`services.a80`](rom/page0/source/services.a80).

## 4. CPU clocks and raster timing

<a id="cpu-clock"></a>

### 4.1 Clock selection and stalls

| `xx77.D3` | `EFF7.D4` | CPU clock                   |
| --------- | --------- | --------------------------- |
| 0         | 1         | 3.5 MHz                     |
| 0         | 0         | 7 MHz                       |
| 1         | Either    | 14 MHz with variable stalls |

The hardware reset selection is 7 MHz. ERS can subsequently apply a persisted speed. Clock changes
are synchronized to the refresh part of an M1 cycle; they need not take effect on the exact `OUT`
edge.

3.5/7 MHz modes normally provide sufficient memory bandwidth for the implemented video modes, but
"no wait" does not mean every I/O access has fixed timing. RTC and UART data operations wait for the
AVR. At 14 MHz RAM arbitration, small code/data read caches, DOS ROM entry, and AY/VG93 accesses add
stalls. The effective rate depends on the instruction stream and memory/I/O accesses; do not convert
14 MHz into an exact 4× Spectrum performance claim.

<a id="raster-timing"></a>

### 4.2 Z80 video mode and board raster

A Z80 program can change the **display format** through `xx77` and `EFF7` ports, for example
selecting ZX, ATM graphics, or text. See the [video-mode table](#video-modes).

**The Z80 does not directly select a raster code.** The AVR supplies it to the FPGA as part of
video-output configuration. In this baseline, Scroll Lock cycles through TV/VGA combined with the
four raster choices. The table gives the line and frame duration for each raster. Raster timing also
affects border phase and frame-interrupt timing, which matters for Z80 programs that need precise
synchronization.

The “raster code” column is the value of `modes_raster[1:0]`, a two-bit field that the AVR sends in
bits D5:4 of the FPGA's internal SPI configuration register. A Z80 program cannot access that SPI
register as an I/O port.

| Raster code | Timing              | Lines/frame | 3.5 MHz T-states/line | T-states/frame |
| ----------- | ------------------- | ----------- | --------------------- | -------------- |
| 0           | Pentagon            | 320         | 224                   | 71680          |
| 1           | Approximately 60 Hz | 262         | 224                   | 58688          |
| 2           | Spectrum 48K        | 312         | 224                   | 69888          |
| 3           | Spectrum 128K       | 311         | 228                   | 70908          |

These totals follow the counter periods in the source; border/interrupt phase also differs by
raster. Raster-dependent effects should use the implementation of `video_sync_h.v` /
`video_sync_v.v`, not only the frame total.

Contention is enabled in the baseline top level, effective at 3.5 MHz for raster codes 2/3. The 48K
address region is `4000–7FFF`; 128K additionally contends `C000–FFFF` when `7FFD` selects an odd
bank. Even-address I/O is also included in contention timing despite the stricter actual peripheral
decoding. The `+2A/+3` contention selector is wired to zero; comments describing it as TODO are not
evidence of a complete +3 timing mode.

VGA is a line-doubled output of these raster timings. It does not give software a PC VGA framebuffer
or a conventional 640×480/60 Hz interface. The 60 Hz raster changes the timing seen by software.

Sources: [`zclock.v`](fpga/base_trdemu/trunk/z80/zclock.v),
[`zmem.v`](fpga/base_trdemu/trunk/z80/zmem.v),
[`video_sync_h.v`](fpga/base_trdemu/trunk/video/video_sync_h.v),
[`video_sync_v.v`](fpga/base_trdemu/trunk/video/video_sync_v.v),
[`zx.c`](avr/baseconf/trunk/src/zx.c).

## 5. Video, attributes, fonts, and palettes

<a id="video-modes"></a>

### 5.1 Mode selection and screen pages

| `xx77.D2:0` | `EFF7.D0` | `EFF7.D5` | Display mode                                                   | Screen 0 pages             | Screen 1 pages             |
| ----------- | --------- | --------- | -------------------------------------------------------------- | -------------------------- | -------------------------- |
| 3           | 0         | 0         | Spectrum 256×192, 8×8 attributes                               | 5                          | 7                          |
| 3           | 0         | 1         | ZX hardware multicolour, 256×192, one attribute per 8×1 pixels | 5                          | 7                          |
| 3           | 1         | 0         | Pentagon 256×192, 16 colours; 8 pixels/group, 4 bytes          | 4+5                        | 6+7                        |
| 0           | 0         | 0         | ATM 320×200, 16 colours; 8 pixels/group, 4 bytes               | 1+5                        | 3+7                        |
| 2           | 0         | 0         | ATM 640×200, one attribute per 8×1 pixels                      | 1+5                        | 3+7                        |
| 6           | 0         | 0         | ATM 80×25 text, two pages                                      | attributes 1, characters 5 | attributes 3, characters 7 |
| 7           | 0         | 0         | Evolution 80×25 text, one page                                 | 8                          | 10                         |

`+` means one screen uses two RAM pages. The table lists physical RAM page numbers in decimal.
For example, page 10 for screen 1 in Evo 80×25 mode is `0Ah`. The pager takes inverted values.
In 320×200 mode with `7FFD.3=0`, the display reads pages 1 and 5 for screen 0. See the
[byte layout within these pages](#video-addressing).

`7FFD.3` selects the screen set independently of the CPU's window map. The video controller fetches
these physical pages even if the CPU maps something else at `4000`. Software must page in the actual
display RAM to draw into it. Other mode combinations are not a documented programming interface.

<a id="video-addressing"></a>

### 5.2 Addressing formulas

All offsets below are **byte offsets within a physical 16 KiB page**. `x=0` and `y=0` are the
top-left pixel/character. These formulas do not include the CPU window address where the application
maps the page.

For Spectrum-family bitmap addressing, define:

```text
zx(y, xb) = ((y & C0h) << 5) | ((y & 07h) << 8)
          | ((y & 38h) << 2) | xb
```

Here `xb=0..31` is an 8-pixel byte column, `y=0..191`. This formula uses the same interleaved line
order as the Spectrum 48K bitmap: line 0 starts at `0000h`, line 1 at `0100h`, line 7 at `0700h`,
line 8 at `0020h`, and line 16 at `0040h`. Adjacent lines therefore do not occupy adjacent memory
addresses.

**Spectrum mode:** pixels are at `zx(y,xb)`, attributes at `1800h+(y>>3)*32+xb`. Bits 7→0 draw
left→right. INK=`{attr6,attr2:0}`, PAPER=`{attr6,attr5:3}`, and bit 7 enables FLASH.

**ZX hardware multicolour:** bitmap addresses are unchanged; the corresponding attribute is at
`2000h+zx(y,xb)`, with the same INK/PAPER/FLASH interpretation.

**Two 16-colour pixels per byte (Pentagon/ATM):** the colour-byte format is `IiGRBgrb`. The left
pixel index is `{D6,D2,D1,D0}`, the right pixel index is `{D7,D5,D4,D3}`. It is not ordinary
high-nibble/low-nibble packed RGBI.

For **256×192 16-colour**, each 8-pixel group at offset `z=zx(y,xb)` reads, in left-to-right order:

| Byte | Screen 0          | Screen 1          |
| ---- | ----------------- | ----------------- |
| 0    | Page 4, `z`       | Page 6, `z`       |
| 1    | Page 5, `z`       | Page 7, `z`       |
| 2    | Page 4, `2000h+z` | Page 6, `2000h+z` |
| 3    | Page 5, `2000h+z` | Page 7, `2000h+z` |

Each byte supplies two consecutive pixels using the colour-byte format above. Rows 0 and 1 therefore
give four pixels: one byte from each of pages 4 and 5 (or 6 and 7) at offset `z`. Rows 2 and 3 give
four more pixels from those same pages at `2000h+z`. In total, four bytes × two pixels = eight
pixels/group.

**ATM graphics use linear row order:** `o=y*40+g`, with `y=0..199`, `g=0..39`. Each row starts after
the previous row's 40 groups. For 320×200 each group is 8 pixels; for 640×200 each is 16 pixels.

| Lane | Screen 0          | Screen 1          |
| ---- | ----------------- | ----------------- |
| 0    | Page 1, `o`       | Page 3, `o`       |
| 1    | Page 5, `o`       | Page 7, `o`       |
| 2    | Page 1, `2000h+o` | Page 3, `2000h+o` |
| 3    | Page 5, `2000h+o` | Page 7, `2000h+o` |

In ATM 320×200, lanes 0, 1, 2, and 3 each supply two pixels: four bytes for each 8-pixel group. In
640×200, lanes 1/3 are the left/right 8-pixel bitmap bytes; lanes 0/2 are their attributes. ATM
attributes use INK=`{D6,D2:0}`, PAPER=`{D7,D5:3}`. Bit 7 is independent PAPER intensity, **not
FLASH**.

For **ATM text**, let `g=x>>1`, `y=0..24`, `o=y*64+g`.

| Field                 | Two-page mode 6, screen 0 | One-page mode 7, screen 0 |
| --------------------- | ------------------------- | ------------------------- |
| Even-column character | Page 5, `01C0h+o`         | Page 8, `01C0h+o`         |
| Even-column attribute | Page 1, `21C0h+o`         | Page 8, `31C0h+o`         |
| Odd-column character  | Page 5, `21C0h+o`         | Page 8, `11C0h+o`         |
| Odd-column attribute  | Page 1, `01C1h+o`         | Page 8, `21C1h+o`         |

For screen 1, substitute pages 7/3 in mode 6 or page 10 in mode 7. The `+1` in odd-column attribute
addressing is intentional. The layout is neither a linear 80-byte character row nor alternating
character/attribute bytes. Attributes have the ATM interpretation above. Character codes index the
loaded 8×8 font; KOI8/CP866 is a choice of font image, not a hardware character decoder.

<a id="border"></a>

### 5.3 Border

Writing `xxFE` sets colour `D2:0` (0–7); writing `xxF6` sets `8|D2:0` (8–15). `xxFC` aliases `xxFE`
for colours 0–7 without changing the beeper or tape outputs. With `A15=0`, however, `xxFC` is also
decoded as a `7FFD` write and can change the RAM page and screen selection (unless `7FFD` is locked).
To write only the border colour, use `80FC` or another `xxFC` address with `A15=1`. The renderer can
synchronize border changes to the raster; an `OUT` is not necessarily visible at the same pixel phase
on every raster/turbo setting. The border uses the programmable palette too. Read its latched index
at `0FBD`.

<a id="atm-palette"></a>

### 5.4 ATM palette: 16 colours out of 64

The ATM palette is applied **in every video mode**: after the FPGA forms a pixel, it uses that
pixel's 4-bit index to look up one of 16 RGB entries. The border colour goes through the same
lookup. Changing an ATM palette entry therefore changes that index in ZX, Pentagon, ATM graphics,
and text modes. `BF.5` changes the component precision of this same palette; it does not select a
video mode. ULAplus has a separate 64-colour path when enabled on its supported raster; see below.

`xxFF` is a port address: `FF` is the low byte and `xx` is the high byte. With the standard palette
(`BF.5=0`), `xx` does not select a colour index. In the FPGA, the write address comes from the current
`zxcolor` signal: in the active raster area this is the 4-bit index of the pixel being output; outside
it this is the border index. When border synchronisation is enabled, the latched border value is used
outside the active area. That index selects the palette entry on the `atm_palwr` strobe; there is no
separate entry-select register. A write while pixel index 2 is output changes entry 2, and a write while
border index 2 is output changes the same entry. In the active area the index can change every pixel,
so the long top border during vertical blank is normally used for controlled writes. When border
synchronisation is enabled, `border_sync` latches the value every **8 raster clocks**, not once per
scanline: `video_sync_h.v` tests only the low three bits of `hcount`. Allow the latch to update after
changing the index before writing `xxFF`; a whole scanline delay is unnecessary. With border
synchronisation disabled, the current border index is used directly (`video_palframe.v`). To program
the whole palette, wait once for the start of a long border interval, then select each border index
and write its colour while remaining outside the active area.

Enable writes with A14=0 on `xx77`, write `xxFF`, then disable palette writes. In particular, `xxFF`
still updates the floppy system latch during palette writes. With extended RGB444 (`BF.5=1`), some
bits of `xx` supply the low bits of the colour components, as described in §5.5; the palette index is
still selected by the same signal at the write strobe.

Each colour channel uses two bits: the uppercase letter is its high bit and the lowercase letter its
low bit. For example, `Rr=10` gives red level 2 of 3, `Gg=01` gives green level 1, and `Bb=11` gives
blue level 3. In `grbG00RB`, the order is green/red/blue low bits, green high bit, two control bits,
then red and blue high bits.

| Data bit written to `xxFF` | Meaning                                       |
| -------------------------- | --------------------------------------------- |
| 7                          | `~g`                                          |
| 6                          | `~r`                                          |
| 5                          | `~b`                                          |
| 4                          | `~G`                                          |
| 3, 2                       | Keep 1 for normal floppy-controller operation |
| 1                          | `~R`                                          |
| 0                          | `~B`                                          |

The byte written to `xxFF` is the bitwise inverse of the `grbG00RB` pattern. For example, `Rr=10`,
`Gg=01`, `Bb=11` produces pattern `A3h`, so software writes `5Ch`; its bits 3:2 are 1. ERS palette
routines pass colours in this format. It is not RGB222 bit order. ROM initializes the palette; FPGA
RAM is not guaranteed to start cleared.

`0DBD` reads the colour selected by `zxcolor`. For the standard palette, if `W` is the byte written to
`xxFF` and `R` is the value read from `0DBD`, the write and read permutations cancel each other: the
colour bits match (`R7:4=W7:4`, `R1:0=W1:0`), while D3:2 read as 1. The read byte can therefore be used
to restore a saved palette entry. In the FPGA the palette read is registered, so the colour corresponds
to the selection signal from the previous pixel-clock cycle. This is not independent readback by
palette index and does not read the entry last selected by `OUT xxFF`. Reading the whole palette
requires displaying each desired index (for example, with the border) and accounting for raster timing.

<a id="rgb444"></a>

### 5.5 RGB444 extension and ULAplus restrictions

Set `BF.5` for the extended ATM palette input. The `xxFF` data byte supplies the top two bits of
each 4-bit component; high-address bits supply the bottom two bits using the same permutation:

| Channel bits  | Data bits    | Address bits   |
| ------------- | ------------ | -------------- |
| R3,R2 / R1,R0 | `~D1`, `~D6` | `~A9`, `~A14`  |
| G3,G2 / G1,G0 | `~D4`, `~D7` | `~A12`, `~A15` |
| B3,B2 / B1,B0 | `~D0`, `~D5` | `~A8`, `~A13`  |

Use `OUT (C),A` to set address and data independently. The video circuit stores 12-bit colours (four
bits per channel), while the physical video output carries two bits per channel. The internal FPGA
module [`video_palframe_mk3bit`](fpga/base_trdemu/trunk/video/video_palframe.v) converts each 4-bit
channel level to 2 bits and alternates adjacent levels across pixels, lines, and frames. This
approximates intermediate shades. A 12-bit register value does not imply a 12-bit DAC or identical
analogue colour on every board. `0DBD` exposes only a six-bit portion and is **not sufficient to
save the complete RGB444 palette**; keep a software copy.

ULAplus has a separate 64-entry palette. Write `0..63` to `BF3B` to select an entry, then a
`GGGRRRBB` byte to `FF3B`. Write `40h` to `BF3B`, then D0=1/0 to `FF3B` to enable/disable. Reads of
`xx3B` return the last byte written to `FF3B`. They do not read the selected palette entry or
separately report the current ULAplus enable state.

With ULAplus attributes, `attr7:6` choose subpalette 0–3; INK index is `(attr>>6)*16+(attr&7)`,
PAPER index is `(attr>>6)*16+8+((attr>>3)&7)`. FLASH/BRIGHT become subpalette bits. The border uses
the first eight entries.

**Baseline restriction:** `video_palframe.v` selects the ULAplus palette only when `up_ena` is set
**and raster code is 3 (128K raster)**. The hardware manual describes ULAplus for ZX/ZX-multicolour
modes more generally. Do not assume enabling its register on the default Pentagon raster is
sufficient in this tree. The expansion of ULAplus colours into the shared 12-bit palette also passes
through this implementation's dithering stage.

<a id="font-ram"></a>

### 5.6 Font RAM

ATM text mode (mode 6) and Evolution text mode (mode 7) use the same 2 KiB font RAM. There is no
separate font for each mode or text page: changing the mode or page does not change the glyph
shapes. Loading a new font changes the glyphs in both text modes.

There are 256 glyphs of 8 bytes each. Glyph `n`, row `r`, is stored at `n*8+r`; bits 7→0 represent
pixels left→right. Set `BF.2` to mirror **every CPU memory write** to font address
`CPU_address & 07FFh`. Normal memory writes continue at the same time. Interrupt stack pushes and
unrelated stores can therefore corrupt the font while the gate is open.

To upload an uncompressed 2048-byte font, disable interrupts and set `BF.2`. Keep the source and
destination mapped throughout the copy. Choose a RAM destination whose contents can be overwritten
and whose low 11 address bits match the font addresses. For example, copying to `8000–87FF` also
writes font-RAM addresses `000–7FF`. Clear `BF.2` before restoring interrupts. Preserve the other
`BF` bits. This low-level method takes raw glyph bytes; ERS `57h` has a different, compressed-font
contract in this source.

Font reading through `0EBD` returns the scan-line byte currently emitted by the font circuit. There
is no CPU-indexed font-RAM read port. The timed display sampling procedure in
[`docs/fontread.src`](docs/fontread.src) illustrates the intended approach; its timing depends on
clock and raster.

Sources: [`video_modedecode.v`](fpga/base_trdemu/trunk/video/video_modedecode.v),
[`video_addrgen.v`](fpga/base_trdemu/trunk/video/video_addrgen.v),
[`video_render.v`](fpga/base_trdemu/trunk/video/video_render.v),
[`video_palframe.v`](fpga/base_trdemu/trunk/video/video_palframe.v),
[`video_fontrom.v`](fpga/base_trdemu/trunk/video/video_fontrom.v),
[`zports.v`](fpga/base_trdemu/trunk/z80/zports.v).

## 6. Interrupts and NMI

### 6.1 Frame interrupt in the baseline

The video timing generator requests one maskable interrupt (INT) from the Z80 per frame. INT remains
active until the CPU acknowledges it or a counter reaches 256 ticks of the 28 MHz clock. The counter
pauses while WAIT is active. With WAIT inactive, the pulse lasts about 9.14 μs: this equals 32 Z80
T-states at 3.5 MHz. At turbo speeds, the number of CPU T-states during the pulse depends on the
selected CPU frequency. On interrupt acknowledge, the Z80 reads `FFh`; this supports the usual
Spectrum IM 2 table and IM 0 `RST 38h` handler.

The INT request is not queued. If CPU interrupts remain disabled throughout the pulse, the program
misses that frame interrupt. There is no baseline programmable raster interrupt or UART interrupt to
the Z80. The newer latched interrupt API is described in §15.

<a id="nmi-entry"></a>

### 6.2 NMI entry

NMI can be requested by Print Screen / the AVR-serviced NMI input, a falling edge of `BF.3`, or a
match with the instruction address in `10BD` / `11BD`. Print Screen, the external input, and `BF.3`
request NMI at the start of the next frame. An address match requests it immediately after the
opcode fetch.

On NMI entry, the CPU saves its return address on the interrupted stack and sets PC to `0066h`, a
hexadecimal address in the Z80 address space. The FPGA supplies a one-time `NOP` at that address.
After the CPU fetches it, the FPGA maps RAM page **FF** into window 0 (`0000–3FFF`). ERS names it
`RAM_NMI=00h`, the inverted page number. The next instruction is fetched from CPU address `0067h`,
which corresponds to offset `0067h` in page FF. Place the first byte of the NMI handler there.

A custom handler should save the old SP immediately, move to its own stack, save the necessary
registers and IFF2, and capture the mapping/configuration through `BD` ports before changing them.
`LD A,I` / `LD A,R` exposes IFF2 in P/V. Entry stack writes occur before the page overlay, so the
interrupted stack must be writable and safe. The injected NOP can disrupt an instruction if one of
its opcode fetches falls at 0066, for example in a prefixed instruction. The initial return-address
writes can also damage code or data if the stack overlaps them. A custom handler therefore requires
careful code and stack placement.

ERS already owns physical page `FF` and installs its own NMI/RST workspace there. Replacing that
handler also replaces part of ERS's execution environment.

### 6.3 NMI exit

Restore registers, the interrupted stack, and the desired IFF2 state. End with:

```asm
        out (0xBE),a       ; data is ignored
        retn               ; must immediately follow
```

For NMI, `BE` arms delayed overlay removal across the next two M1/refresh events, allowing both
opcode bytes of `RETN` to execute before the normal map returns. It does not decode a `RETN`
instruction; inserting other opcodes is unsafe. `RETN` restores IFF1 from IFF2. `DI`/`EI` inside the
handler also change IFF2. If the handler uses DI/EI, restore the interrupt-enable state required by
the interrupted program before returning.

An NMI scheduled at frame INT may have displaced the expected frame handler. A debugger/monitor that
resumes cycle-sensitive code must account for this.

<a id="execution-breakpoint"></a>

### 6.4 NMI request on an instruction address (breakpoint)

Write the CPU address to `10BD` (low byte) and `11BD` (high byte), then set `BF.4`. When the Z80
fetches an opcode from that address (an M1 cycle), the FPGA requests NMI. The comparison uses the
CPU address, so bank switching can place different code at the same address. Data reads and writes
do not trigger NMI. The opcode at the matched address executes before control passes to the handler.
The enable remains set after a match; the handler must clear it or set a new address. For multi-byte
instructions, account for prefix fetches.

Sources: [`zint.v`](fpga/base_trdemu/trunk/z80/zint.v),
[`znmi.v`](fpga/base_trdemu/trunk/z80/znmi.v), [`zbreak.v`](fpga/base_trdemu/trunk/z80/zbreak.v),
[`nmi_service.a80`](rom/page5/source/nmi_service.a80).

## 7. Keyboard, joystick, mouse, and tape input

<a id="keyboard"></a>

### 7.1 Spectrum matrix and raw PS/2 codes

`IN` from `xxFE` or `xxF6` returns D7=1, D6=tape input, D5=0, D4:0=active-low keyboard matrix. Zero
bits in A15:A8 select rows, which combine with AND in the usual Spectrum manner. The FPGA reads
already-latched input state without an AVR round trip for each matrix read.

The AVR combines mechanical keyboard state and translated PS/2 keys. A PS/2 key can map to two
Spectrum keys, including a shift chord. The default mapping and optional EEPROM map are implemented
in `kbmap.c` / `kbmap.h`; the historic `docs/kbd-map*.txt` diagrams are not authoritative for
current defaults. For example, current `kbmap.c` maps left Shift to CAPS SHIFT, left Ctrl to SYMBOL
SHIFT, and left Alt to CAPS SHIFT+SYMBOL SHIFT. Do not infer physical PS/2 modifier state from the
emulated Spectrum matrix.

For raw PS/2 input, disable the EEPROM window, select extension type 2 through any RTC cell `F0–FF`,
then read those cells. Each read consumes one buffered scan-code byte; `00` means empty and `FF`
means overflow. Reset the buffer by writing bit 0 to RTC register `0C`. The log is a byte stream
with PS/2 prefixes, not ASCII; its capture handles short make/break sequences and is not a promise
to preserve arbitrary long keyboard response sequences. Modifier status is also available
independently through RTC `0D` (§11).

Assigning Ctrl or Alt to Spectrum keys in the EEPROM keyboard map also affects management shortcuts.
The AVR excludes assigned Ctrl/Alt keys from some computer-control combinations so that using those
keys in a program does not also trigger a management action.

### 7.2 Management keys and reset controls

| Control                                   | Baseline behavior                                                         |
| ----------------------------------------- | ------------------------------------------------------------------------- |
| Short F12                                 | Soft Z80/FPGA-register reset, without reloading the FPGA                  |
| Held F12, about 5 seconds                 | Full firmware reset; does not request ATX power-off                       |
| Ctrl+Alt+Delete                           | Full reset, subject to the firmware's mapped-control-key rules            |
| Print Screen / NMI contact                | Request NMI, normally synchronized to frame start                         |
| Num Lock                                  | Toggle beeper/Covox versus tape-output routing; LED indicates tape output |
| Scroll Lock                               | Cycle output/raster states; see below                                     |
| Both mouse buttons + keypad `*`, `+`, `-` | Reset/increase/decrease mouse resolution, if supported                    |

**Source-specific Scroll Lock behavior:** `zx.c` increments the combined `MODE_VIDEO_MASK=31h`
field, cycling TV/VGA for each of the four raster codes. The old Shift+Scroll Lock special case is
commented out. The PDFs' description of a two-state TV/VGA toggle plus Shift+Scroll Lock for 60 Hz
does not describe this exact AVR source.

The physical soft-reset/ATX button turns on power when off, resets on a short press while running,
and requests shutdown after about five seconds. Hard reset restarts the AVR and reloads FPGA
configuration. RAM retention is only promised for soft reset; neither full reset nor power-off is a
persistence mechanism.

<a id="joystick"></a>

### 7.3 Kempston joystick

Normal-mode `xx1F` returns active-high bits 0=right, 1=left, 2=down, 3=up, 4=fire; upper bits are
normally zero. In shadow mode the same port is the VG93 command/status register. Keep shadow mode
closed while polling a joystick.

<a id="mouse"></a>

### 7.4 Kempston mouse

| Port   | Result                                                       |
| ------ | ------------------------------------------------------------ |
| `FADF` | D7:4=4-bit wheel counter, D3=1, D2=middle, D1=right, D0=left |
| `FBDF` | 8-bit wrapping X counter                                     |
| `FFDF` | 8-bit wrapping Y counter                                     |

A button bit is 0 while pressed. Coordinate counters wrap from FFh to 00h. To obtain movement,
subtract the previous reading from the new reading using 8-bit arithmetic and interpret the result
as signed: FFh → 00h is +1. The counter is not a ready-made screen coordinate. The wheel counter
wraps from F to 0; use a signed 4-bit difference for wheel movement. The AVR reports the absent
mouse state as `FF` in all three registers; a non-wheel mouse can expose `F` in the wheel field.
Mouse resolution is a PS/2 device setting, 1/2/4/8 counts per millimetre for settings 0–3 where
supported, not a change to the video coordinate system.

### 7.5 Tape input

The AVR samples the tape input and sends its level to the FPGA; the CPU reads the stored level
through `FE.6`. It is distinct from TAP-file loading. TAP emulation is an ERS ROM service using RAM
buffers and BASIC entry interception; it is not an FPGA cassette waveform generator and is not
available solely because `FE.6` exists.

Sources: [`zkbdmus.v`](fpga/base_trdemu/trunk/z80/zkbdmus.v), [`zx.c`](avr/baseconf/trunk/src/zx.c),
[`ps2.c`](avr/baseconf/trunk/src/ps2.c), [`kbmap.c`](avr/baseconf/trunk/src/kbmap.c),
[`tape.c`](avr/baseconf/trunk/src/tape.c), [`atx.c`](avr/baseconf/trunk/src/atx.c),
[`tape.a80`](rom/page5/source/tape.a80).

## 8. Sound and printer interfaces

<a id="ay-sound"></a>

### 8.1 AY/YM

Write a register number to `FFFD`, write its value to `BFFD`, and read the selected register through
`FFFD`. The onboard AY/YM is a physical peripheral, not an FPGA AY synthesizer. It adds three tone
channels, noise, envelope, and I/O registers to the 48K beeper facility. AY/VG93 I/O receives
extended timing at 14 MHz. Exact audio clocks/wiring depend on the board; refer to its schematic and
the AY/YM datasheet under `docs/datasheets`.

ROM code includes handling for additional AY arrangements, but their presence in ROM does not
establish two onboard AY chips or automatic TurboSound support on every machine. NMI code mutes AY
output; the current return path has the AY-restore call commented out, so applications should not
rely on NMI preserving all sound-generator state.

<a id="beeper-covox"></a>

### 8.2 Beeper, tape output, and Covox

`FE.4` is the beeper bit and `FE.3` is the tape-output bit. The AVR's `MODE_TAPEOUT` selects which
is routed to the shared output. The current FPGA does **not XOR the two bits**, contrary to the
older English manual.

Writing an unsigned sample `0..255` to `xxFB` enables Covox PWM output. Its carrier is approximately
218.75 kHz with the 28 MHz implementation clock. There is no sample FIFO, sample-rate interrupt,
DMA, or automatic playback clock; software times sample writes.

The source's switch back from Covox to beeper is conditional on the stored beeper-state change
(`beep_bit != beep_bit_old`) at an `FE` write. It is not the older documented unconditional "any FE
write disables Covox" rule. Border writes to `F6` do not generate a beeper write strobe. Programs
mixing sampled sound and border effects should account for this implementation detail.

### 8.3 Printer through AY

The optional AY printer interface is described in
[`docs/revb/zxevo_ay_printer.pdf`](docs/revb/zxevo_ay_printer.pdf) and
[`docs/ayprn.txt`](docs/ayprn.txt). ERS provides `RST 08h` functions `40–43` for initialization,
characters, BASIC tokens, and screen output. These functions depend on the configured printer
driver/attachment and are not a native ATM Centronics port. See the API reference before using them
outside BASIC hooks.

Sources: [`sound.v`](fpga/base_trdemu/trunk/sound/sound.v),
[`zports.v`](fpga/base_trdemu/trunk/z80/zports.v),
[`ay_printer.a80`](rom/page5/source/ay_printer.a80),
[`rst8service.a80`](rom/page5/source/rst8service.a80).

## 9. SD and IDE storage

For files on supported HDD and SD partitions, use the ERS device manager and FAT services.
This section describes direct controller access. The C library's `bc_sd_*` and `bc_ide_*`
sector calls work at this layer too. They bypass the ERS volume state and caches, so the
application must coordinate access to the same medium.
[ERS storage services](#rst-storage) · [storage.h](lib/baseconf/include/baseconf/storage.h)

<a id="sd-interface"></a>

### 9.1 SD SPI, Z-controller-compatible interface

In normal mode, write `01h` to `0077h` to select the card and `03h` to deselect it.
Both values keep D0=1 for Z-controller compatibility. D1 drives the active-low CS_n signal.
In shadow mode, write the same values to `8057h`. With normal decoding, reads from `xx77`
always return zero. Read the card-present contact in `RTC[0Ch].3` and the write-permit contact
in `RTC[0Ch].2`. SPI commands during initialization establish whether the card responds.
[RTC extension controls](#rtc-controls)

Each access to `xx57` starts one SPI exchange, transmitting and receiving eight bits simultaneously.
A write sends the written byte; a read sends FFh. The value returned by a read comes from the
**previous completed exchange**.

For example, an OUT of 55h starts transmitting 55h. The next IN returns the byte received
**while sending** 55h and starts sending FFh. Another IN returns the byte received while
sending that FFh. The byte received during a write can be filler. Reading the start token
already starts the next exchange, so the following read returns the first data byte.

In shadow mode:

| Access               | Meaning                                               |
| -------------------- | ----------------------------------------------------- |
| Write `xx57`, A15=0  | Transmit byte                                         |
| Write `xx57`, A15=1  | Set CS_n using D1, as normal-mode `xx77`              |
| Read `xx57`, any A15 | Receive previous byte and start transfer sending `FF` |

After the driver has received the block-start token and accounted for the one-exchange response
delay, it can read a sector with two `INIR` instructions with C=`57h`, B=0. Shadow reads also allow
this; **shadow writes cannot use an unrestricted 256-byte `OTIR`**, because the high address byte
changes and its top bit would turn some writes into chip-select changes. Use four 128-byte runs with
B=`80h` before each `OTIR`; the Z80 predecrements B before the output, keeping A15=0 for every byte.
These snippets describe only data transfer, not SD initialization, token, CRC, or busy handling.

The baseline SPI hub uses a fixed clock (`28 MHz / 2 = 14 MHz`). The Z80 interface exposes no
programmable divider. See the repository's SD driver for the initialization sequence used here.

The Z80 and AVR share the SD interface. The AVR can request exclusive access with a lock; the FPGA
grants it when the Z80 deselects the card (CS_n=1). Z80 software must therefore deselect the card
between transactions. Holding CS_n=0 continuously prevents the AVR from acquiring access.

This arbitration prevents simultaneous SPI transfers. It does not coordinate filesystem caches or
changes made by two independent programs.

<a id="ide-interface"></a>

### 9.2 IDE task file and data-byte ordering

The always-visible interface is Nemo IDE with a sequential-byte extension.

| Port   | Read                                                                    | Write                                                    |
| ------ | ----------------------------------------------------------------------- | -------------------------------------------------------- |
| `xx10` | Low byte, or alternating low/high bytes                                 | Commit low byte after `11`, or sequential low/high bytes |
| `xx11` | Latched high byte from last low read                                    | Latch high byte; no device cycle yet                     |
| `xx30` | Error: reason for the last command failure                              | Features: parameter for ATA configuration commands       |
| `xx50` | Sector count                                                            | Sector count                                             |
| `xx70` | Sector number / LBA7:0                                                  | Same                                                     |
| `xx90` | Cylinder low / LBA15:8                                                  | Same                                                     |
| `xxB0` | Cylinder high / LBA23:16                                                | Same                                                     |
| `xxD0` | Device/head                                                             | Device/head, LBA/CHS, master/slave                       |
| `xxF0` | Status: device busy, data ready, and error flags                        | Command: ATA command code that starts an operation       |
| `xxC8` | Alternate status: status without acknowledging an ATA interrupt request | Device control: software reset and ATA interrupt control |

The canonical `10,30,...,F0` ports use ATA CS0 and address A7:A5. `C8` uses CS1 and register
address 6. The decoder also accepts `08,28,48,68,88,A8,E8` on CS0, with the corresponding A7:A5
register number; `C8` is the special CS1 exception. These are implementation aliases, not another
storage controller.

To read a 16-bit word in Nemo mode, read `xx10` and then `xx11`. The first read fetches a word from
the IDE device, returns its low byte to the CPU, and saves its high byte in the FPGA. The second
read returns that saved high byte without another device access.

To write a word, write the high byte to `xx11` and then the low byte to `xx10`. Writing `xx11` only
saves the byte in the FPGA. Writing `xx10` sends the complete word, assembled from both bytes, to
the device.

Sequential extension: consecutive accesses to `10` transfer low then high, without swapping bytes.
It accepts two `INIR` or two `OTIR` runs of 256 bytes for a 512-byte sector. Before starting a
transfer, access another IDE register, such as command/status. This resets the low/high alternation
so the first access to xx10 transfers the low byte again. Prevent other handlers or drivers from
accessing IDE and changing that state between the two bytes of a word.

Poll alternate status at `xxC8` to leave the ATA interrupt request pending. Wait for `BSY`
(D7) to clear, then check `ERR` (D0) and `DF` (D5). Start a data transfer only when `DRQ`
(D3) is set. Bound each wait with a timeout. This ROM's `DEVFIND` probes the onboard Nemo master.
ERS also contains an ATAPI/CD driver and optional external-device drivers. ATA Device/Head
supports selecting the slave device directly.

Sources: [`zports.v`](fpga/base_trdemu/trunk/z80/zports.v),
[`spihub.v`](fpga/base_trdemu/trunk/spihub/spihub.v),
[`z_sd_drv.a80`](rom/page5/source/fat/z_sd_drv.a80),
[`nemo_drv.a80`](rom/page5/source/fat/nemo_drv.a80),
[`dev_drv.a80`](rom/page5/source/fat/dev_drv.a80).

## 10. Floppy controller and hardware-assisted emulation

<a id="floppy-registers"></a>

### 10.1 Physical Beta Disk/VG93 interface

Only shadow mode exposes these ports:

| Port   | Meaning                                                                      |
| ------ | ---------------------------------------------------------------------------- |
| `xx1F` | VG93/WD1793 command (W), status (R)                                          |
| `xx3F` | Track                                                                        |
| `xx5F` | Sector                                                                       |
| `xx7F` | Data                                                                         |
| `xxFF` | System latch (W), controller/drive status (R), palette data (W when enabled) |

`xxFF` write: D1:0 select drive 0–3, D2 controls reset (0 holds reset), D3 is HRDY (the controller
head-ready line, driven directly by D3), D4 is side selection. In this source, readback is D7=INTRQ,
D6=DRQ, D5=1, D4=**inverted** internal side, D3=HRDY, D2=reset state, D1:0=drive. The old English
manual's statement that all lower six read bits are 1 is stale.

VG93 command/status interpretation depends on the command class. A full floppy driver must respect
DRQ, command completion, errors, and physical media timing; these registers are not a sector-level
BIOS. Palette writes through `xxFF` share the drive/reset/HRDY/side latch and should preserve its
intended state.

<a id="floppy-overlay"></a>

### 10.2 FE hardware-assisted port interception

Bits `13BD.D3:0` enable interception for drives A–D: bit 0 is A, bit 3 is D, and 1 enables
interception. All bits reset to zero.

On a floppy-port access, the FPGA checks four conditions: DOS is active, ROM is selected in the
current CPU window, the selected drive's bit in 13BD is 1, and palette writes are disabled. If all
hold, the FPGA replaces ROM in window 0 with physical RAM page **FE**. The next instruction is read
from RAM at the same CPU address where it would otherwise have been read from ROM.

This is a ROM-to-RAM overlay, not an NMI, and does not push a trap return address. The emulator must
contain suitable continuation code at the intercepted TR-DOS instruction locations. `BF.0` alone is
not enough to trigger it: the implementation tests DOS, ROM state, drive mask, and palette mode.

Physical VG93 register cycles are suppressed for masked drives. Access to the system latch `xxFF`
still takes place. A transient write inhibit before the first subsequent M1 prevents `INI`/`INIR`
from corrupting the newly overlaid FE page. The software emulator must still implement the semantics
of the original I/O instruction and controller operation.

For an emulator-only overlay, `OUT (BE),A` restores ROM **for the next instruction**; do not apply
the NMI delayed-`RETN` sequence blindly. If NMI occurs while emulation is active, physical FF takes
priority. The first `BE` exit leaves NMI; the emulator is left by its later `BE` exit.

### 10.3 ERS virtual storage and FAT

The FE ROM installs the emulator in physical FE from the packed `dos_fe` image. The ordinary/non-FE
ROM path is a different software arrangement. Match the ROM to the FPGA before enabling masked
drives; `RST 08h,4Dh` includes a limited read/write probe of `13BD` for this purpose.

ERS provides FAT12/16/32 reading, device/partition selection, TRD file mounting, TRD/SCL/FDI image
loading into a RAM disk, and TAP loading into tape buffers. Mounted TRD images can be read/written
through the selected block driver; this is not a general FAT create/delete/resize/write-file API.

Important source limits and ownership rules:

- FAT sectors are 512 bytes. MBR scanning handles the four primary entries and supported FAT
  partition types; no general GPT or extended-partition walker is provided by that scan.
- Directory traversal and search use bounded service tables (up to 2048 found entries and 16 stored
  directory levels in the ROM structures).
- Four drive descriptors are available for A–D; the mount descriptor table is 256 bytes, 64 bytes
  per drive. For large files, the cluster cache stores cluster locations at intervals.
- `IMAGE.MNT` supplies automatic mount paths; its parser reads a bounded 512-byte sector. Use its
  implemented format from `mounter.a80` / the ERS manual, not the `IMAGE.FNT` typo in old comments.
- A mounted image takes priority over a RAM disk assigned the same drive letter.
- The RAM disk uses approximately 800 KiB of data plus a separate track/sector table. Its format
  markers and reserved pages must survive until unmounted.
- SD hot-swap is not a safe open-file operation. Reinitialize the service/device state after a media
  change; the ERS manual recommends reset.

See [the storage RST API](#rst-storage) for the actual function numbers and descriptor formats.

Sources: [`vg93.v`](fpga/base_trdemu/trunk/vg93/vg93.v),
[`zdos.v`](fpga/base_trdemu/trunk/z80/zdos.v), [`zports.v`](fpga/base_trdemu/trunk/z80/zports.v),
[`dos_fe`](rom/page1/dos_fe/), [`mounter.a80`](rom/page5/source/fat/mounter.a80),
[`read_fat.a80`](rom/page5/source/fat/read_fat.a80).

## 11. RTC, NVRAM, firmware identification, and EEPROM

<a id="rtc-access"></a>

### 11.1 Access and data format

The AVR emulates Gluk/DS12877-style registers using a PCF8583 and software state. In normal mode set
`EFF7.7`, write an index to `DFF7`, then read/write data at `BFF7`. In shadow mode use `DEF7` /
`BEF7`, with no `EFF7.7` gate. Each **data** access waits for the AVR. RTC writes can be much slower
than cached time reads. Prevent interrupt handlers from changing the RTC index between selecting an
index and accessing its data, or the operation will use the wrong cell.

| Index                  | Meaning                                                                                                                                                         |
| ---------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `00`, `02`, `04`       | Seconds, minutes, hours                                                                                                                                         |
| `01`, `03`, `05`       | Alarm positions; alarm interrupt functionality is not emulated                                                                                                  |
| `06`, `07`, `08`, `09` | Weekday (1–7), day, month, two-digit year                                                                                                                       |
| `0A`                   | Reads baseline RTC A value 0; writes select a 16-byte AVR EEPROM page                                                                                           |
| `0B`                   | Baseline value `02`; bit 2 selects binary (1) versus BCD (0) time values                                                                                        |
| `0C`                   | Time-update flag, keyboard LEDs, SD presence, EEPROM-window selection; reading clears the update flag. [Bit values and write effects](#rtc-controls)            |
| `0D`                   | Valid/status and PS/2 modifier bits, below                                                                                                                      |
| `0E`–`EF`              | Battery-backed NVRAM; ERS owns some high cells                                                                                                                  |
| `F0`–`FF`              | With `0C.7=0`: AVR version, scan codes, or current mode ([type selection](#rtc-extensions)); with `0C.7=1`: 16 bytes of the selected [EEPROM page](#avr-eeprom) |

Values in the first column are **RTC register indices**, not Z80 port addresses. For example, to
read seconds in normal mode, write `00` to `DFF7` and then read `BFF7`; reading Z80 port `0000` does
not access the RTC.

The AVR stores time internally in binary. If register `0B.2` is clear, it converts Z80 reads/writes
to and from BCD, where each decimal digit occupies four bits: for example, decimal 23 is encoded as
23h. Do not assume a full MC146818: alarm interrupts, periodic interrupt configuration, and complete
12-hour-mode behavior are not implemented by this emulation. In particular the source's physical
hour loading is 24-hour.

<a id="rtc-controls"></a>

### 11.2 RTC extension controls

| `0C` bit | Read                                                                                  | Write                                                                                                                         |
| -------- | ------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------- |
| 0        | Num Lock LED / tape-output mode indication                                            | 1 clears the raw PS/2 log                                                                                                     |
| 1        | Caps LED state                                                                        | Desired Caps LED state                                                                                                        |
| 2        | SD write-permit contact, 1 means writable                                             | No corresponding control                                                                                                      |
| 3        | SD presence contact, 1 means inserted                                                 | No corresponding control                                                                                                      |
| 4        | 1 means the AVR updated the time since the previous `0C` read; reading clears the bit | Writing does not set or acknowledge this flag                                                                                 |
| 7        | Last written EEPROM-window state                                                      | 1 redirects `F0–FF` to EEPROM; 0 restores the selected extension (versions, PS/2, or modes). [EEPROM addressing](#avr-eeprom) |

Reading `0C` clears its update-ended flag. Writing `0C` changes several functions together; setting
Caps LED with a write of `02` also disables the EEPROM window. Do not confuse this index with FPGA
port `0CBD`.

`0D` reads D7=1 and active-high PS/2 state in bits 0=left Ctrl, 1=right Ctrl, 2=left Alt, 3=right
Alt, 4=left Shift, 5=right Shift, 6=F12.

<a id="rtc-extensions"></a>

### 11.3 Version records and raw keyboard/configuration windows

With the EEPROM window disabled, writing a type to any index `F0–FF` selects:

| Type | Reading `F0–FF`                                                       |
| ---- | --------------------------------------------------------------------- |
| 0    | 16-byte base firmware version record                                  |
| 1    | 16-byte bootloader version record                                     |
| 2    | Raw PS/2 log, consuming one byte per read; 0=empty, FF=overflow       |
| 3    | `F0` reads `modes_register`; other positions return FF in this source |

Version record layout:

```text
F0..FB: 12-byte ASCII name, zero-padded
FC..FD: little-endian packed date/release word
FE..FF: little-endian AVR firmware CRC

word bit15    = official release flag
word bits14:9 = year - 2000
word bits8:5  = month
word bits4:0  = day
```

The CRC belongs to the AVR configuration image, not the Z80 ROM. The string does not provide a
universal bit-by-bit FPGA capability query. ERS's own ROM version is a separate `RST 08h` result.

In extension type 3, `modes_register` includes bit 0=VGA, bit 1=tape-output routing, bit 2=Caps LED,
bits 5:4=raster code (§4). The manual's older description of only bit 4 as a 60 Hz toggle is
incomplete for this AVR. This is a **read** extension; writing the window selects a type, not new
mode flags.

The extension type is shared AVR state. Reads from `F0–FF` return the selected extension's data,
so restoring the previous type requires an agreement with the loader or tracking each selector
write. In C, use `bc_avr_window_select()` to keep track of that state. Select type 0 explicitly
before reading the version. [rtc.h](lib/baseconf/include/baseconf/rtc.h)

Reading the version step by step:

1. Make sure `0C.7=0` (§11.2): while the EEPROM window is on, `F0–FF` are EEPROM cells, not the
   extension window.
2. Write type `0` to any index `F0–FF`: in normal mode the index goes to `DFF7` and data to `BFF7`
   (with `EFF7.7=1`); in shadow mode use `DEF7`/`BEF7`, independent of `EFF7.7`.
3. Read the 16 cells `F0–FF`, selecting the index before each read. With type 2, each read consumes
   one byte from the PS/2 log.
4. Restore the known previous type, then restore the EEPROM-window state.

This example assumes shadow ports are open, the EEPROM window is off, the previous type is 3,
and `buf` points to 16 writable bytes.

```asm
        ld bc,0xDEF7
        ld a,0xF0
        out (c),a
        ld bc,0xBEF7
        xor a
        out (c),a              ; type 0: base firmware version
        ld hl,buf
        ld e,0xF0
loop    ld bc,0xDEF7
        ld a,e
        out (c),a              ; cell index
        ld bc,0xBEF7
        in a,(c)               ; cell data
        ld (hl),a
        inc hl
        inc e
        jr nz,loop             ; F0..FF
        ld bc,0xDEF7
        ld a,0xF0
        out (c),a
        ld bc,0xBEF7
        ld a,3
        out (c),a              ; restore the loader's choice
```

To change `0C.7`, copy only D1 (Caps Lock) from the read value and add the required D7.
Keep D0 clear to preserve the PS/2 log. Reading D0 reports Num Lock, while writing a 1 to it
clears the log. Reading `0C` itself clears the D4 time-update flag.

<a id="avr-eeprom"></a>

### 11.4 AVR EEPROM window

Write the EEPROM page number to index `0A`, enable `0C.7`, then access `F0–FF`. The byte address is:

```text
EEPROM_address = (page_number << 4) + (RTC_index & 0Fh)
```

ATmega128 has 4 KiB EEPROM; stay within its real address range and preserve firmware-owned data such
as the keyboard map. EEPROM writes are persistent and have different endurance/latency from RAM.
Clear `0C.7` before expecting version or raw-keyboard reads. The selected extension type and EEPROM
page are global AVR state; cooperate with other callers.

<a id="ers-settings"></a>

### 11.5 ERS persistent settings

ERS uses the following NVRAM cells for its settings. ROM software interprets these values:

| Cell       | ERS use                                                                                                                                                                                                                                                                                                      |
| ---------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `E7`       | D1:0 select autoboot: 0=off, 1=run `SD_BOOT.$C`, 2=run the file named in `AUTOBOOT.INI`. D7:5 are declared as a timeout in `global_vars.a80`; current `AUTORUNBOOT` does not use them                                                                                                                        |
| `E8`       | Resident/one-shot launch state, including bit 7                                                                                                                                                                                                                                                              |
| `E9`       | Reserved in this ROM setup layout                                                                                                                                                                                                                                                                            |
| `EA`       | D7:4 set the screensaver delay in minutes (0 disables it); D3:0 are the HDD detection-delay setting in seconds. The menu selects values 0–9; this is an ERS setting, not an FPGA timer                                                                                                                       |
| `EB`       | D7 enables onboard SD discovery; D6/D5 are IDE master/slave access settings (slave discovery is not implemented). D3:2 select real drive 0–3; D1:0 select virtual/RAM-disk drive letter 0–3                                                                                                                  |
| `EC`       | D7: 1=3.5 MHz, 0=7 MHz; D6 enables NeoGS SD discovery; D5 enables `IMAGE.MNT` automount; D4 shows the ERS clock; D3 enables key sounds; D2 enables resident-program detection. D1:0: 0=1024K, 1=48K, 2=128K                                                                                                  |
| `ED`       | D7 selects 14 MHz when set and the EC.7 clock when clear; D6 selects TAP loading when set and physical-tape loading when clear; D5 enables the BASIC AY-printer driver; D4=1 preserves the current font at reset; D3: 0=ATM, 1=CP866; D2 enables TAP autostart. D1:0: 0=ERS, 1=GLUK, 2=ProfROM, 3=custom ROM |
| `EE`, `EF` | CRC16 for ERS settings `E7–ED` in the current source, low/high                                                                                                                                                                                                                                               |

For one-bit enable settings in EB–ED, 1 enables the named function and 0 disables it; the ED bit 4
exception is stated explicitly. These settings are consumed by ERS services and reset code: writing
NVRAM is not an immediate hardware-switch command. Use the corresponding ports to change the current
video mode, clock, or memory map. Drive numbers 0–3 mean A–D; FAT volume numbers use a separate
numbering scheme.

Use `RST 08h,55h,02h` to update ERS settings and recalculate their CRC; direct writes to protected
settings can cause reset-time restoration to defaults. The default bytes for `ED..E7` are
`04 82 00 00 00 00 00`. The current CRC routine processes all seven bytes `ED..E7`; older
descriptions of a five-byte `E9..ED` protected range do not describe this implementation. `ED.4=1`
inhibits font reload despite a misleading positive-sense symbol name. The low two bits of `ED`
select 0=ERS, 1=GLUK, 2=ProfROM, 3=custom ROM.

Sources: [`rtc.c`](avr/baseconf/trunk/src/rtc.c), [`rtc.h`](avr/baseconf/trunk/src/rtc.h),
[`version.c`](avr/baseconf/trunk/src/version.c), [`main.h`](avr/baseconf/trunk/src/main.h),
[`call_cmos.a80`](rom/page5/source/call_cmos.a80), [`global_vars.a80`](rom/global_vars.a80),
[`menu_execute.a80`](rom/mainmenu/src/menu_execute.a80),
[`sdcardboot.a80`](rom/mainmenu/src/sdcardboot.a80).

<a id="uart-interface"></a>

## 12. RS-232 UART

The UART implements a polling subset of the Kondratyev/16550 register layout, backed by the AVR UART
and software FIFOs. It supports CTS/RTS, not the complete modem-control interface. There is no Z80
UART interrupt; all accesses wait for AVR service. The FPGA actually selects registers from A10:A8
for low byte `EF`; use the documented `F8EF–FFEF` address range. Wider aliases may also decode; the
documented interface uses the stated range.

DLAB is bit 7 of LCR (`FBEF`). Only `F8EF` and `F9EF` change function when it is switched. To set
the baud rate, set DLAB=1, write both divisor bytes, then restore DLAB=0 before sending or receiving
data. The other registers do not depend on DLAB.

| Port   | Read                                                                                    | Write                                                                                                                |
| ------ | --------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------- |
| `F8EF` | DLAB=0: remove the next byte from the receive queue; DLAB=1: DLL, low divisor byte      | DLAB=0: enqueue a transmit byte; DLAB=1: set DLL                                                                     |
| `F9EF` | DLAB=0: saved IER value; DLAB=1: DLM, high divisor byte                                 | DLAB=0: store IER (interrupts are not implemented); DLAB=1: set DLM                                                  |
| `FAEF` | ISR, stored interrupt-identification register; interrupt operation is not implemented   | FCR: D0 must be 1; D1=1 discards queued receive bytes, D2=1 discards queued transmit bytes                           |
| `FBEF` | Current LCR, the serial character format                                                | D7=DLAB; D1:0=0/1/2/3 select 5/6/7/8 data bits; D2=0 selects one stop bit, 1 selects two; D5:3 select parity (below) |
| `FCEF` | Stored MCR, modem-line control                                                          | D1 controls RTS (1 drives the AVR pin active-low); other modem features are unsupported                              |
| `FDEF` | LSR: data readiness, transmitter state, and errors (decoded below)                      | Not provided                                                                                                         |
| `FEEF` | MSR: D7=1 CD, D6=0 RI, D5=1 DSR, D4=CTS, D0=CTS changed; reading clears the change flag | Not provided                                                                                                         |
| `FFEF` | Last byte written to the scratch register                                               | Store one byte; no battery backup                                                                                    |

LCR parity values `001` and `011` in bits 5:3 select odd/even; other combinations produce no parity.
A full 16550 driver must not assume stick parity, break transmission, hardware FIFO trigger
interrupts, or programmable interrupt IDs. FIFO mode is always used. The AVR uses all 16 slots
in each queue. When the ring indices coincide, LSR flags distinguish a full queue from an
empty one. A write to a full TX queue drops the byte. For byte-at-a-time sending, wait for
`LSR.D5=1` before each write, as `bc_uart_send()` does. Wait for `LSR.D0=1` before reading
a byte. Bound both waits with a timeout.

In LSR, D1 reports RX overrun, D2 parity error, D3 framing error, and D7 an RX queue that is
at least half full. Resetting the RX FIFO clears D1 and discards queued bytes. The firmware
sets D5 and D6 together when it hands the final queued TX byte to the AVR UART register.
A character may still be on the wire at that point. Reading MSR clears the CTS-change flag.

Divisor modes:

```text
DLM.bit7=0, divisor != 0:
    requested_baud = 115200 / ((DLM << 8) | DLL)

DLM.bit7=1:
    UBRR = ((DLM & 7Fh) << 8) | DLL
    actual_baud = 11059200 / (16 * (UBRR + 1))
```

The standard mode is converted to an AVR divisor, so clock quantization still applies. Both divisor
bytes zero select the firmware's nominal "256000 baud" special case; the actual rate follows its
integer UBRR calculation and AVR clock. For MIDI's nominal 31250 baud, the documented native request
is DLL=`15h`, DLM=`80h`, again subject to quantization.

To initialize, set LCR.DLAB, write DLL/DLM, clear DLAB with the desired framing, reset FIFOs with
FCR=`07h`, set RTS as required, and poll LSR for transfers. Preserve the register index's high
address byte when using `IN`/`OUT`.

Source: [`rs232.c`](avr/baseconf/trunk/src/rs232.c),
[`zports.v`](fpga/base_trdemu/trunk/z80/zports.v).

## 13. Reset, boot, and ERS-owned memory

### 13.1 Boot sequence and image responsibilities

The AVR bootloader loads the working AVR firmware; that firmware contains the FPGA configuration. A
full startup configures the FPGA, then releases the Z80. The disabled pager initially exposes the
last Flash bank in every CPU window. The page-0 service starts at `0000`, creates both memory maps,
initializes the RST/NMI service, applies settings, sets palette/font, and selects the target ROM or
ERS main menu.

Do not equate the two update files:

- `zxevo_fw.bin`: AVR program plus FPGA image, installed through the AVR bootloader workflow.
- `rom/zxevo_fe.rom`: 512 KiB Z80 Flash contents assembled from BASIC, DOS, ERS, CP/M, GLUK,
  ProfROM, and the custom region.

The FPGA FE port interception needs a corresponding FE ROM installation. Changing only one layer can
leave an apparently bootable machine whose disk emulation or ROM services do not work.

### 13.2 ROM layout

`rom/build_full.sh` assembles `zxevo_fe.rom` as 32 consecutive 16 KiB Flash banks. The file offset
identifies a bank directly: `bank = offset / 4000h`. The plain ROM selector value is inverted:
`selector = 1Fh - bank`. The selector is the page number before the ROM type/control bits are added
for port `xFF7`. All values in the map are hexadecimal.

| File offset   | Flash bank | ROM selector | Contents                                                                                  |
| ------------- | ---------- | ------------ | ----------------------------------------------------------------------------------------- |
| `00000–03FFF` | `00`       | `1F`         | Custom ROM area, block 0; filled with `FFh` in the supplied image (`ff_64k.rom`)          |
| `04000–07FFF` | `01`       | `1E`         | Custom ROM area, block 1; filled with `FFh` in the supplied image                         |
| `08000–0BFFF` | `02`       | `1D`         | Custom ROM area, block 2; filled with `FFh` in the supplied image                         |
| `0C000–0FFFF` | `03`       | `1C`         | Custom ROM area, block 3; filled with `FFh` in the supplied image                         |
| `10000–13FFF` | `04`       | `1B`         | ATM CP/M BIOS (`rom/atm_cpm/rbios.rom`)                                                   |
| `14000–17FFF` | `05`       | `1A`         | Standard BASIC 48 from the BASIC 128 set (`rom/page3/basic48_128_std.rom`)                |
| `18000–1BFFF` | `06`       | `19`         | Standard BASIC 128 (`rom/page2/128_std.rom`)                                              |
| `1C000–1FFFF` | `07`       | `18`         | Original BASIC 48 (`rom/page3/basic48_orig.rom`)                                          |
| `20000–23FFF` | `08`       | `17`         | ROM image `rom/page3/2006.rom`                                                            |
| `24000–27FFF` | `09`       | `16`         | TR-DOS 6.10 (`rom/trdos_v6/dosatm3.rom`)                                                  |
| `28000–2BFFF` | `0A`       | `15`         | BASIC 128 (`rom/page2/basic128.rom`)                                                      |
| `2C000–2FFFF` | `0B`       | `14`         | GLUK startup ROM (`rom/page0/glukpen.rom`)                                                |
| `30000–33FFF` | `0C`       | `13`         | EVO ProfROM image, block `00000–03FFF` in `rom/profrom/evoprofrom.rom`                    |
| `34000–37FFF` | `0D`       | `12`         | EVO ProfROM image, block `04000–07FFF`                                                    |
| `38000–3BFFF` | `0E`       | `11`         | EVO ProfROM image, block `08000–0BFFF`                                                    |
| `3C000–3FFFF` | `0F`       | `10`         | EVO ProfROM image, block `0C000–0FFFF`                                                    |
| `40000–43FFF` | `10`       | `0F`         | EVO ProfROM image, block `10000–13FFF`                                                    |
| `44000–47FFF` | `11`       | `0E`         | EVO ProfROM image, block `14000–17FFF`                                                    |
| `48000–4BFFF` | `12`       | `0D`         | EVO ProfROM image, block `18000–1BFFF`                                                    |
| `4C000–4FFFF` | `13`       | `0C`         | EVO ProfROM image, block `1C000–1FFFF`                                                    |
| `50000–53FFF` | `14`       | `0B`         | Empty; symbol `ROM_ADD_BAS48`                                                             |
| `54000–57FFF` | `15`       | `0A`         | Empty; symbol `ROM_ADD_DOS`                                                               |
| `58000–5BFFF` | `16`       | `09`         | `ROM_RST83`: RST 8 dispatcher and service routines                                        |
| `5C000–5FFFF` | `17`       | `08`         | `ROM_RST82`: windows, input, tape, printer, CMOS, and other services                      |
| `60000–63FFF` | `18`       | `07`         | `ROM_RST81`: service installation, fonts, menu descriptors, and handlers                  |
| `64000–67FFF` | `19`       | `06`         | `ROM_RST80`: addon and loader/font utilities                                              |
| `68000–6BFFF` | `1A`       | `05`         | `ROM_MAINMENU`: unpacker and packed ERS main menu                                         |
| `6C000–6FFFF` | `1B`       | `04`         | Empty; `ROM_EMPTY`                                                                        |
| `70000–73FFF` | `1C`       | `03`         | BASIC 48 from Spectrum 128 ROM 1, patched for Evolution ERS (`rom/page3/basic48_128.rom`) |
| `74000–77FFF` | `1D`       | `02`         | NEO-DOS (`rom/page1/neo-dos.rom`)                                                         |
| `78000–7BFFF` | `1E`       | `01`         | BASIC 128 (`rom/page2/basic128.rom`)                                                      |
| `7C000–7FFFF` | `1F`       | `00`         | Reset service `ROM_ERS` (`rom/page0/services_fe.rom`)                                     |

The eight EVO ProfROM rows above identify exact 16 KiB slices of the 128 KiB binary, in file order.
The build and page-selection source do not assign separate names to those slices. The ERS service
image is also documented in [`rom/page5/README.md`](rom/page5/README.md); that file describes the
five service pages and their internal entry points.

Here, “patched for Evolution” means that address `0008h` contains `NOP; JP RST8_CMP`, routing
supported `RST 08h` service calls to ERS. BASIC's AY-printer operations also use ERS functions
`40h–43h`. The source is based on Spectrum 128 ROM 1, so it includes its 128K-specific keypad
scanning and token routines such as `PLAY` and `SPECTRUM`. The `RST8_CMP` entry forwards command
bytes in the range `40h–6Fh`; the documented public function table is `40h–57h`.

Within the ERS region, logical ROM constants run backwards from the end:

| ERS constant   | Value | Purpose                                                          |
| -------------- | ----- | ---------------------------------------------------------------- |
| `ROM_ERS`      | 0     | Reset service, `page0/services_fe`                               |
| `ROM_BAS128`   | 1     | BASIC 128                                                        |
| `ROM_DOS`      | 2     | NEO-DOS in the current FE build                                  |
| `ROM_BAS48`    | 3     | Spectrum 128 BASIC 48 with Evolution ERS patches                 |
| `ROM_EMPTY`    | 4     | Empty                                                            |
| `ROM_MAINMENU` | 5     | Packed ERS main menu                                             |
| `ROM_RST80`    | 6     | Addon2 and loader/font utilities                                 |
| `ROM_RST81`    | 7     | Addon1, installation, fonts, menu descriptors                    |
| `ROM_RST82`    | 8     | Addons, windows, input, tape, printer, CMOS                      |
| `ROM_RST83`    | 9     | RST dispatcher, FAT/device/mount/math services                   |
| 10, 11         | —     | Empty in the current full FE concatenation; historic addon slots |

The packed FE disk emulator is included by the service sources; the FE full image does not put a
separate emulator ROM in logical slot 10. Other useful logical constants are `18h`=original BASIC
48, `19h`=standard BASIC 128, `1Ah`=48 BASIC from the 128 set, `1Bh`=ATM CP/M. Their corresponding
physical bank is `1Fh-constant`.

### 13.3 ERS RAM reservations

These values are **inverted port values / ERS constants**, not physical page numbers. Convert with
`physical=FFh-value`:

| Port value / constant range | Physical RAM page(s) | Ownership                                           |
| --------------------------- | -------------------- | --------------------------------------------------- |
| `00` (`RAM_NMI`)            | `FF`                 | NMI/RST code, registers, stacks, service variables  |
| `01` (`RAM_EVODOS`)         | `FE`                 | DOS/emulator workspace                              |
| `02`                        | `FD`                 | FAT variables/buffers                               |
| `03`                        | `FC`                 | File-search/sort tables                             |
| `04`                        | `FB`                 | ProfROM variables                                   |
| `05`                        | `FA`                 | Mount descriptors/cache                             |
| `06`, `07`                  | `F9`, `F8`           | Debugger-related allocations                        |
| `08`                        | `F7`                 | Saved screen/font                                   |
| `09`                        | `F6`                 | Temporary workspace                                 |
| `0A`                        | `F5`                 | RAM-disk track/sector descriptors                   |
| `0B–3C`                     | `F4–C3`              | 50 pages / 800 KiB RAM-disk data                    |
| `3D` onward                 | `C2` downward        | TAP buffer; overlaps Flash-buffer allocation by use |
| `3D–5C`                     | `C2–A3`              | 32-page / 512 KiB ROM flashing buffer               |

These are allocations of the service software, not all permanently enforced by hardware. An
application taking arbitrary extended RAM must leave ERS-owned pages intact or stop using those
services and install its own handlers. Overwriting the NMI/RST page while still relying on Print
Screen/RST calls is unsafe. The flasher and TAP buffers intentionally use the same physical pages
and cannot hold independent contents at the same time.

### 13.4 Reset-time ROM choice and resident hooks

`ED.1:0` selects ERS, GLUK, EVO ProfROM, or a custom 64 KiB ROM set. Holding `0` at reset forces the
ERS path and clears resident/autoboot activation. Source also handles BASIC/DOS/menu choices from
the reset-time keyboard and can enter CMOS setup when multiple keys are held. These are
reset-service policies, not additional hardware reset vectors.

GLUK/custom handoff expects the service entry convention around offset `3D2D` and `NOP; RET` at
`3D2F`. ProfROM uses a short page-switching routine copied to RAM so that changing the ROM does not
replace the instruction being executed. A replacement ROM must match the chosen setup's page and
entry assumptions or provide its own reset service. The Honey Commander resident hook uses a
reserved high-memory page, a jump opcode, a checksum, and trailing signature; `CMP_REZIDENT` in page
0 implements its launch contract. Applications use this Hobeta/SPG-specific mechanism; there is no
general autostart API.

Sources: [`services.a80`](rom/page0/source/services.a80),
[`addon1.a80`](rom/page5/source/addon1.a80), [`global_vars.a80`](rom/global_vars.a80),
[`build_full.sh`](rom/build_full.sh), [`avrboot`](avrboot/trunk/),
[`firmware update manual`](docs/zxevo_firmware_update_eng.pdf).

## 14. AVR/FPGA interface and firmware development

### 14.1 Responsibilities

The FPGA implements Z80 buses, RAM arbitration, paging, video, clock generation, input latches, SD
SPI, and the logic that holds the CPU in WAIT until the AVR responds. The AVR runs PS/2 protocols,
keyboard translation, mechanical input scanning, tape sampling, RTC/NVRAM, UART, ATX/reset policy,
and FPGA loading. Z80 applications normally access the AVR through RTC/UART ports; the internal AVR
SPI register map below is **not a Z80 I/O address space**.

<a id="avr-spi"></a>

### 14.2 Internal SPI protocol

With `spics_n=1`, transmit the register number and read status. Pull `spics_n` low to transfer data;
its rising edge commits many operations. The rising edge also resets the register selection to zero,
so select it again after a new transaction. Status is latched on that edge: bit 7 is Z80
read/not-write, bit 0 indicates a waiting RTC request, bit 1 a waiting UART request.

| Internal SPI register  | Function                                                                                                                                       |
| ---------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------- |
| `00`                   | Default/ignored register                                                                                                                       |
| `10`                   | Shift 40 keyboard-state bits                                                                                                                   |
| `11`                   | Apply the 40 bits previously shifted through `10` as the new keyboard matrix, on the CS 0→1 edge                                               |
| `20`, `21`, `22`, `23` | Mouse X, Y, buttons, Kempston joystick                                                                                                         |
| `30`                   | Z80 reset request on CS edge                                                                                                                   |
| `40`                   | For a Z80 read, the AVR supplies the reply; for a Z80 write, the AVR receives the written byte. CS 0→1 completes the request and releases WAIT |
| `41`                   | Read the RTC index selected by Z80                                                                                                             |
| `42`                   | Read UART register index A10:A8 latched from Z80                                                                                               |
| `50`                   | Configuration: bit0 VGA, bit1 falling-edge NMI request, bit2 tape input, bit3 tape-output selection, bits5:4 raster                            |
| `60`                   | SD transfer data                                                                                                                               |
| `61`                   | SD arbitration: bit7 lock request/status, bit0 AVR CS_n                                                                                        |

To claim SD for the AVR: request lock with CS_n high, wait until lock is granted when the Z80
releases the card, then transfer while holding lock. To release: set CS_n high while locked, then
clear lock. The first byte read after acquiring the interface is not defined as valid received data.

For a WAIT request, determine read/write and source from status, read the source's address register,
exchange the data through `40`, and finish the transaction. Until the AVR completes it, the Z80
remains waiting. Adding a new AVR-serviced device requires matching FPGA request/status routing and
an AVR handler; a new C switch branch alone does not create a Z80 port.

### 14.3 Build entry points

These are source entry points for an intentional firmware build, not commands that were executed to
produce this documentation:

| Layer                  | Entry point / tools                                                                         |
| ---------------------- | ------------------------------------------------------------------------------------------- |
| FPGA FE                | `fpga/base_trdemu/trunk/quartus/Makefile`, `top.qsf`; Quartus project and Verilog modules   |
| AVR                    | `avr/baseconf/trunk/build/Makefile`; AVR compiler, packed FPGA include/configuration inputs |
| Combined configuration | `cfgs/standalone_base_trdemu/trunk`; packaging scripts combine selected source trees        |
| Reset ROM              | `rom/page0/source/build.sh`; ASL + `p2bin`, normal and `DOS_FE` variants                    |
| RST ROM                | `rom/page5/source/build.sh`; ASL, `p2bin`, MHMT-packed assets                               |
| Complete Z80 ROM       | `rom/build_full.sh`; writes `rom/zxevo_fe.rom`                                              |
| AVR bootloader         | `avrboot/trunk/avr/boot_evo.asm` and associated build scripts                               |

Many assembly sources are CP866-encoded. Decode comments deliberately when reading them, and
preserve encoding when changing source; do not infer unreadable comments as missing documentation.
Include files (`ports_evo.a80`, `define.a80`, `global_vars.a80`) are shared between pages. The
generated `.lst` files are useful address maps but can be stale relative to `.a80`; dispatch/source
logic takes precedence over an old listing or prebuilt `.rom`.

Configuration packaging is intentionally separate from source-tree layout; see
[`cfgs/README`](cfgs/README). A bundled binary does not necessarily prove that the nearest source
directory was used to build it.

Sources: [`slavespi.v`](fpga/base_trdemu/trunk/slave/slavespi.v),
[`SPI format notes`](fpga/base_trdemu/trunk/slave/spi_fmt.txt),
[`zx.h`](avr/baseconf/trunk/src/zx.h), [`zx.c`](avr/baseconf/trunk/src/zx.c),
[`main.c`](avr/baseconf/trunk/src/main.c).

## 15. Documented extensions and source discrepancies

<a id="enhanced-interrupts"></a>

### 15.1 Enhanced interrupt controller in the `newirq` tree

Ports `14BD`–`16BD` belong to the separate [`fpga/base_trdemu/newirq`](fpga/base_trdemu/newirq/)
tree: `z80/zports.v` decodes these addresses, and `z80/zint.v` contains the controller logic. The
main [`fpga/base_trdemu/trunk`](fpga/base_trdemu/trunk/) tree has no such registers; its `zint.v`
generates the frame pulse.

The `newirq` tree has a separate Quartus project and a `top.rbf` build target in
[`quartus/Makefile`](fpga/base_trdemu/newirq/quartus/Makefile). The standard standalone
configuration builder points to `fpga/base_trdemu/trunk/quartus`; the firmware packaging scripts do
not select `newirq`.

The `newirq` implementation is incomplete. For example, request autoclear is marked TODO in `zint.v`
and is disabled. The table below records the interface described by the hardware manual; check the
source for the FPGA image being built before relying on a specific feature.

| Port   | Documented enhanced-controller meaning                                                                                                                                                                                                                                |
| ------ | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `14BD` | D7=1 enables the enhanced controller, 0 selects the traditional frame INT. D2=1 lets ZXBUS devices supply an interrupt vector; D1=1 enables internal-source vectors; D0=1 automatically clears an acknowledged internal request (only with D1=1). All bits reset to 0 |
| `15BD` | Internal interrupt enable; bit0 VBLANK; bit7 is write set/reset control, reads zero                                                                                                                                                                                   |
| `16BD` | Internal pending status; bit0 VBLANK; bit7 is write set/reset control, reads zero                                                                                                                                                                                     |

VBLANK is a request at the start of vertical blanking, after the visible part of the frame. An
enable bit in `15BD` allows a pending request to assert INT; the request itself is stored separately
in `16BD`.

With the enhanced controller, sources latch pending state and enabled pending sources hold INT.
Internal interrupts have priority over external interrupts. Internal vectors by bit/priority,
highest first, are:

| Bit | Vector | IM 0 instruction | Documented source |
| --- | ------ | ---------------- | ----------------- |
| 6   | CF     | RST 08h          | Reserved          |
| 5   | D7     | RST 10h          | Reserved          |
| 4   | DF     | RST 18h          | Reserved          |
| 3   | E7     | RST 20h          | Reserved          |
| 2   | EF     | RST 28h          | Reserved          |
| 1   | F7     | RST 30h          | Reserved          |
| 0   | FF     | RST 38h          | VBLANK            |

These are **interrupt vector bytes**, not ERS function numbers. Unassigned bits do not imply
implemented devices. Autoclear requires internal vectoring; the manual advises against relying on
autoclear with IM 1 because source information can be lost. With internal pending interrupts,
external interrupt acknowledge is blocked. External vectoring must be enabled only with suitable
expansion devices; the documented default external vector is `C7` (`RST 00h`). The manual calls bit
7 a set/reset control but does not unambiguously state which value sets bits and which clears them.
Consult the enhanced-controller source for the installed firmware before writing its enable or
status registers.

### 15.2 Differences that affect real programs

| Topic                | Older/distributed document or header                  | Selected source                                                                 |
| -------------------- | ----------------------------------------------------- | ------------------------------------------------------------------------------- |
| `12BD`               | Listed RW in the Russian manual                       | Readback only; protection writes use `xBF7`                                     |
| `BF` read            | Some tables leave upper implemented fields undefined  | Returns bits 5:0, including RGB444 enable                                       |
| Font/palette read    | Can look like ordinary indexed reads                  | `0DBD` reads the colour currently displayed; `0EBD` reads the current glyph row |
| ULAplus              | Generally described for ZX/ZX-multicolour             | Palette-selection gate additionally requires 128K raster code 3                 |
| Audio routing        | English manual says beeper XOR tape-out               | `sound.v` selects one of them                                                   |
| Covox exit           | Any FE write                                          | Source tests a stored beeper-state change                                       |
| Floppy `FF` read     | English manual says lower bits all 1                  | Source reads back drive/reset/HRDY/inverted-side fields                         |
| Scroll Lock          | Shift selects a special 60 Hz toggle                  | AVR cycles TV/VGA across four raster codes                                      |
| UART LSR.D7          | Unused                                                | Half-full receive-FIFO indication                                               |
| RTC mode flags       | Only 50/60 Hz bit described                           | AVR carries a two-bit raster selection                                          |
| ERS `4E/4F`          | Reserved in PDF; `_BIOS` alias at 4F                  | File browser / NMI browser block in dispatch table                              |
| Mounter subfunctions | PDF and C header disagree                             | Follow `TAB_MOUNTER`, documented in §17.5                                       |
| `SET_MODE`           | Conflicting constants and video-update flag           | Several implementation differences; see API reference                           |
| Math input registers | PDF says BC for divide16, two 32-bit multiply factors | Actual wrappers use HL/DE for divide16 and BCDE×HL for multiply32               |
| Custom font service  | PDF suggests raw font data                            | Current code always invokes the packed-data decompressor                        |

This guide records source behavior and visible limitations; it does not claim that every discrepancy
has been measured on hardware. No firmware build, simulation, automated checks, or hardware tests
were run for this documentation.

## 16. Source map

Prefer the linked implementation when updating a bitfield or call contract. Use the manuals for
hardware background and user-visible procedures, with their version differences kept explicit.

| Area                     | Primary implementation                                                                                 | Supporting documentation                                                                                                    |
| ------------------------ | ------------------------------------------------------------------------------------------------------ | --------------------------------------------------------------------------------------------------------------------------- |
| Complete port decoding   | [`zports.v`](fpga/base_trdemu/trunk/z80/zports.v), [`ports_evo.a80`](rom/ports_evo.a80)                | [Russian hardware reference](docs/zxevo_base_configuration.pdf), [English reference](docs/zxevo_base_configuration_eng.pdf) |
| ATM heritage             | [`atm_pager.v`](fpga/base_trdemu/trunk/mem/atm_pager.v), [`zdos.v`](fpga/base_trdemu/trunk/z80/zdos.v) | [ATM Turbo 2+ architecture](docs/atm2_arch.pdf)                                                                             |
| Video                    | [`video`](fpga/base_trdemu/trunk/video/)                                                               | Hardware reference §7; [`fontread.src`](docs/fontread.src)                                                                  |
| CPU and interrupts       | [`z80`](fpga/base_trdemu/trunk/z80/)                                                                   | Hardware reference §§6,8,9; [`evonmi.txt`](docs/evonmi.txt)                                                                 |
| AVR input, time, UART    | [`AVR sources`](avr/baseconf/trunk/src/)                                                               | Hardware reference §10; historical keyboard-map notes                                                                       |
| SD/IDE                   | [`spihub`](fpga/base_trdemu/trunk/spihub/), [`ROM drivers`](rom/page5/source/fat/)                     | [`ZC.pdf`](docs/ZC.pdf), [`nemo-divide.txt`](docs/nemo-divide.txt)                                                          |
| ERS API                  | [`define.a80`](rom/define.a80), [`rst8service.a80`](rom/page5/source/rst8service.a80)                  | [ERS manual](<docs/Описание сервис-прошивки ''EVO Reset Service''.pdf>)                                                     |
| Startup and reservations | [`services.a80`](rom/page0/source/services.a80), [`global_vars.a80`](rom/global_vars.a80)              | ERS reset/menu/settings sections                                                                                            |
| Physical board           | Revision-specific schematics under [`docs`](docs/)                                                     | [Revision C user manual](docs/revC/zxevo_user_manual_eng.pdf)                                                               |
| Firmware update          | [`avrboot`](avrboot/trunk/), [`cfgs`](cfgs/)                                                           | [English update manual](docs/zxevo_firmware_update_eng.pdf)                                                                 |

The historical `GLUK661.txt`, `DOS6_11E.txt`, and `PC21HELP.txt` describe their respective
ROM/software versions, not one universal BASECONF hardware API. Likewise the vendor datasheets
describe chips in full; only the subset wired and exposed by this FPGA/AVR combination is a
motherboard programming interface.

## 17. ROM and RST API

### 17.1 Calling convention and prerequisites

ERS calls start with `RST 08h`, followed by a function selector:

```asm
        rst 0x08
        db 0x4D            ; VERSION function selector

        ld h,0x09          ; RTC year index
        rst 0x08
        db 0x55,0x01       ; CMOS_RW function, READ_CMOS subfunction; result A=L
```

The call consists of `RST 08h` and the following function-selector byte. Grouped functions take
another byte for the subfunction: `RST 08h → function → subfunction → next instruction`. ERS reads
these bytes from the program code and returns execution to the instruction after the last selector
byte.

In the first example, `4Dh` selects `VERSION`, so the next byte is an opcode. In the second, `55h`
selects the `CMOS_RW` group and `01h` selects its `READ_CMOS` subfunction; H supplies the RTC index.
The read result is in A and L.

For MATH, COM_DEV, COM_FAT, MOUNTER, and CMOS_RW, the next byte selects a subfunction. A driver call
through COM_DEV can read one more byte. `POS_FILES` takes its additional operation from register A.
The number of bytes after RST must match the call exactly, or the program will resume at the wrong
instruction.

The public ERS API uses one vector, `RST 08h`, followed by inline function numbers. ERS also uses
private `RST 20h` and `RST 30h` cross-page calls; see §17.7.

The ROM startup code must initialize ERS first. The page mapped at `0000` must also contain
compatible entry and return routines. These short routines that transfer control into and out of the
service are called stubs.

In the full ERS image, `ROM_BAS48` is BASIC 48 from Spectrum 128 ROM 1, with Evolution changes. Its
`0008h` vector contains `NOP; JP RST8_CMP`. That routine examines the byte after `RST 08h` and
forwards command bytes `40h–6Fh` to ERS; the documented public functions occupy `40h–57h`. BASIC's
AY-printer calls use ERS functions `40h–43h`. The ROM also contains the 128K keypad and token
routines described in §13.2. `ROM_BAS128` contains the standard BASIC 128: address `0008h` lies
inside the reset-delay instruction, and there is no `RST 08h` handler there. The standard ROM slots
`ROM_BAS48_STD` (`18h`), `ROM_BAS128_STD` (`19h`), and `ROM_BAS48_128` (`1Ah`) also lack the ERS
dispatcher. The original 48K ROM uses `RST 08h` for BASIC errors.

`RST 08h` sends the CPU to address `0008h` in the page mapped into window 0. At that address,
`ROM_RST83` contains `JP $`, a jump to itself that loops the CPU there. A call must start from a
page with an entry stub at `0008h`: for example, `services.rom` has `NOP; JP DOS2RST8`, and
`ROM_BAS48` has `NOP; JP RST8_CMP`. The stub prepares the call state and forwards it to the ERS
dispatcher in `ROM_RST83`.

The entry routines save the CPU registers, memory map, and interrupt state, map physical RAM FF, and
transfer control to the service ROM. Saved registers are stored in cells named `RREG_*`; functions
read arguments from those cells and write their results back there. Code that performs this
conversion between the application call and an internal routine is called a wrapper.

ERS temporarily changes memory windows and normally runs at 7 MHz. It restores the saved state on
return, except for changes requested by the function. Some routines access a buffer through the
caller's saved memory map. Others, such as font loading, read an address directly in their own
current map; those restrictions are noted separately.

Place writable buffers in accessible application RAM, outside ERS-owned pages. The code containing
RST and its function bytes must also remain readable. Do not start a second ERS call before the
first has finished: both calls would use the same register-save cells, stacks, and
file/directory/volume state. This is what non-reentrant means. Interrupt or nested-NMI handlers must
not call ERS on top of an unfinished call.

The wrapper generally preserves registers that are not explicitly returned. Flags, buffer contents,
file position, and persistent service state follow the function-specific effects below. A "no
defined result" entry does not mean there are no side effects. Do not assume a uniform carry/error
convention.

In C, `bc_ers_set_profile()` tells the library which ROM context the application has prepared.
The `bc_ers_*` wrappers use the same `RST 08h` entry and memory requirements. For storage calls,
check both `bc_status_t` and the firmware result field. For example, after `BC_OK` from
`bc_ers_device_find()`, `found` tells you whether ERS found a partition.
[rom.h](lib/baseconf/include/baseconf/rom.h) · [ers.h](lib/baseconf/include/baseconf/ers.h)

The dispatcher masks the function with `3Fh`, then indexes a **24-entry** table covering `40h–57h`;
it has no general bounds check. Only the assigned functions are valid. Masking does not make the
entire `40h–7Fh` range safe. Subfunction tables likewise must not receive arbitrary unknown
selectors.

Sources: [`rst8.a80`](rom/page3/source/rst8.a80), [`services.a80`](rom/page0/source/services.a80),
[`rst8service.a80`](rom/page5/source/rst8service.a80),
[`nmi_service.a80`](rom/page5/source/nmi_service.a80).

### 17.2 Complete top-level RST 08h table

All selector values in this chapter are hexadecimal. `CF` below means the Z80 carry flag, not byte
`CFh`.

| Selector | Symbol / implementation   | Arguments / result / purpose                                                                                                                                        |
| -------- | ------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `40`     | `AY_PRN_INIT`             | Initialize optional AY printer; BASIC hook, no general return value [Details](#rst-output).                                                                         |
| `41`     | `AY_PRN_A_`               | A=character `20–7F`; optional AY printer output [Details](#rst-output).                                                                                             |
| `42`     | `AY_PRN_TOKEN`            | A=BASIC token; printer expands token text [Details](#rst-output).                                                                                                   |
| `43`     | `AY_PRN_SCR`              | Print screen region using BASIC/driver context [Details](#rst-output).                                                                                              |
| `44`     | `TAPE_INIT`               | Initialize ROM tape-emulation state; BASIC hook [Details](#rst-output).                                                                                             |
| `45`     | `TAPE_EMUL`               | IX=destination, DE=length, BASIC loader state; see §17.3 [Details](#rst-output).                                                                                    |
| `46`     | `EXT_ZX_WINDOW` / `_WINW` | IX=20-byte window descriptor; draw/set window context [Details](#rst-output).                                                                                       |
| `47`     | `PRINT_MESSAGE`           | HL=zero-terminated string with control bytes [Details](#rst-output).                                                                                                |
| `48`     | `PRINT_A`                 | A=character; print in current output/window context [Details](#rst-output).                                                                                         |
| `49`     | `SCRUP`                   | Scroll current window up; A=0 excludes attributes, A=1 includes them [Details](#rst-output).                                                                        |
| `4A`     | `SCRDN`                   | Scroll current window down; same A convention [Details](#rst-output).                                                                                               |
| `4B`     | `SET_MODE`                | A=mode request, A=0 queries stored request; source caveats in §17.4 [Details](#rst-modes-math).                                                                     |
| `4C`     | `MATH`                    | Inline subfunction `00–06`; arithmetic/string conversions [Details](#rst-modes-math).                                                                               |
| `4D`     | `VERSION_`                | BCDE=packed BCD-style ROM version; A=emulation/configuration flags                                                                                                  |
| `4E`     | `EXT_FILE_BROWSER_`       | Opens the file-browsing interface. One extra byte after the selector is required (its value is ignored); no selected-file return contract is implemented. See below |
| `4F`     | `NMI_FB_BLOCK_`           | A=0 unblocks NMI browser; A≠0 blocks it                                                                                                                             |
| `50`     | `COM_DEV`                 | Inline device-manager subfunction `00–09` [Details](#rst-storage).                                                                                                  |
| `51`     | `COM_FAT`                 | Inline FAT subfunction `00–0C` [Details](#rst-storage).                                                                                                             |
| `52`     | `SORT_FINDFILES`          | Sort current internal search results; no buffer argument [Details](#rst-storage).                                                                                   |
| `53`     | `MOUNTER`                 | Inline image/drive subfunction `00–0B` [Details](#rst-storage).                                                                                                     |
| `54`     | `INST_FATBOOT_`           | Install FAT micro-loader; HL=destination for current FAT variable copy [Details](#rst-storage).                                                                     |
| `55`     | `CMOS_RW`                 | Inline subfunction `00–03`; read/write/check/reset CMOS settings [Details](#rst-cmos).                                                                              |
| `56`     | `SETUP_PAL`               | A=0 default Spectrum palette; A≠0, HL=16-byte palette in ATM write format [Details](#rst-output).                                                                   |
| `57`     | `SETUP_FONT`              | A=0 built-in ATM font, 1=CP866, 2=custom packed font at HL [Details](#rst-output).                                                                                  |

For `4Dh`, BCDE=`00006101h` means version 0.61.01. A.bit0 is 1 for FE hardware-assisted disk
emulation and 0 for the older software arrangement. A.bit7 flags an FPGA/ROM mismatch detected by a
`13BD` write/read probe. That probe temporarily writes a test drive mask and restores it; it is not
a complete FPGA capability test or firmware version match.

The old ERS PDF calls `4E/4F` reserved. `rom/c/ers.h` still defines `_BIOS=4F` and lacks the current
browser calls. The ROM's `CODE_TABL` determines which function actually executes.
`EXT_FILE_BROWSER` reads the extra byte, sets the external-browser and NMI-browser-block flags,
then enters the menu-opening code. The firmware provides no return-to-caller or selected-file
contract for this entry.

<a id="rst-output"></a>

### 17.3 BASIC hooks, windows, strings, palette, and fonts

**Printer/tape hooks (`40–45`).** Patched BASIC calls these functions in place of its original
printing and tape-loading routines. Such a replacement is called a hook. The functions require
BASIC's prepared variables/state and ERS settings; they are not standalone drivers for an arbitrary
application. `41` with the printer disabled reproduces the fallback flags from
`LD A,B; CP 3; SBC A,A`. In BASIC COPY, the disabled-driver path of `43` returns B=`B0h`.
When the AY printer is enabled, a direct `43` call prints the ZX screen at `4000h` and returns
to the application, preserving the BASIC printer buffer.

The tape emulator's successful-load path advances IX by the copied block, sets DE=0 and load-status
flags; the disabled path reproduces the original loader's fallback behavior (including C=A and flags
from `CP A`). The exact flag/alternate-register convention follows the BASIC loader call site in
[`tape.a80`](rom/page5/source/tape.a80). Supplying IX/DE alone does not select a TAP file or
initialize its current block; load/mount TAP through ERS first.

**Window descriptor (`46`).** Offsets are hexadecimal; all words are little-endian.
Coordinates/sizes are in character cells for the descriptor.

| Offset     | Size   | Meaning                                                                                                                                                                             |
| ---------- | ------ | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `00`, `01` | 1 each | X, Y                                                                                                                                                                                |
| `02`, `03` | 1 each | Height, width                                                                                                                                                                       |
| `04`, `05` | 1 each | Window colour, cursor colour                                                                                                                                                        |
| `06`       | 1      | D7=1 draws the lower header, D6=1 removes the frame, D5=1 draws the upper header; D4=0 prints text from `0C`, D4=1 skips it; D3=1 selects a file list; D3=0 selects a menu          |
| `07`       | 1      | Cursor/descriptor control; with bit7=1, the word at `08` holds the address of the current-item variable; with bit6=1, the word at `0A` holds the address of the item-count variable |
| `08`       | 2      | With byte `07` D7=0: current item number itself; with D7=1: address of a two-byte variable holding that number                                                                      |
| `0A`       | 2      | With byte `07` D6=0: item count itself; with D6=1: address of a two-byte variable holding that count                                                                                |
| `0C`       | 2      | Text address                                                                                                                                                                        |
| `0E`       | 2      | Item-handler address table                                                                                                                                                          |
| `10`       | 2      | Mouse active-zone list address                                                                                                                                                      |
| `12`       | 2      | Hotkey list address                                                                                                                                                                 |

A mouse zone is a rectangular region associated with a hotkey. Its record contains six bytes:
reserved, X, Y, height, width, key code. Drawing a window does not provide public access to every
internal menu-selection routine. Set the video mode and draw the window first. Pass its descriptor
in IX for each 47 call using control code 01 or 03 and each 49 or 4A call. Keep the descriptor in
RAM at 4000–7FFFh so it stays visible when the firmware remaps other memory windows. Selector 48
uses the saved cursor position.

**String controls (`47`).** A zero ends the string; ordinary displayable bytes are characters. The
Spectrum text renderer uses a 6×8 font; text mode uses hardware character cells. Documented controls
are:

| Byte sequence | Effect                                                                                           |
| ------------- | ------------------------------------------------------------------------------------------------ |
| `01`          | Move the print position to the start of the current window text area; does not clear the window  |
| `03`          | Centre following printable text in a Spectrum window; ignored in text mode                       |
| `09,n`        | Advance by n character positions                                                                 |
| `0D`          | New line within the window                                                                       |
| `16,y,x`      | Set print position; **Y precedes X**; Spectrum uses pixel coordinates, text mode character cells |
| `17,c`        | Set colour; FF means leave destination colour unchanged                                          |

Do not substitute BASIC's entire PRINT control-code set for this renderer's implemented controls.
The internal code also uses special message forms not documented as a general external string
format.

**Palette (`56`).** HL points to 16 bytes in the inverted ATM `grbG00RB` format from §5.4, not RGB
triplets. A=0 loads the built-in table. The routine temporarily enables palette writes and waits for
frame timing. This service does not accept a full RGB444 or 64-entry ULAplus table and does not
guarantee that a custom RGB444 state is preserved by the classic palette save/restore.

**Font (`57`).** Built-ins are packed assets. In the current source, A=2 also passes HL directly to
`UNPACK`, then copies 2048 decompressed bytes to font RAM. Therefore a custom input must use the
same packed format (the build uses MHMT `-mlz`), with its source accessible in the routine's actual
mapped context. The PDF describes HL in `4000–FFFF` but does not explain this decompression. That
branch does not restore the application's memory map to read the source. An address that was
readable before the call may therefore refer to different data inside the routine. For raw,
arbitrary fonts, use the controlled `BF.2` upload sequence in §5.6.

Sources: [`window.a80`](rom/page5/source/window.a80),
[`txtmode_proc.a80`](rom/page5/source/txtmode_proc.a80),
[`addons.a80`](rom/page5/source/addons.a80), [`addon1.a80`](rom/page5/source/addon1.a80),
[`define.a80`](rom/define.a80).

<a id="rst-modes-math"></a>

### 17.4 Mode and mathematics services

**`4B SET_MODE` needs particular care.** The distributed PDF/C header advertise video `mode|08h`,
speed fields `10/20/30`, and memory fields `40/80/C0` for 48K/128K/1024K. The current `ISET_MODE`
code actually does the following:

| Request field | Current code behavior                                                     |
| ------------- | ------------------------------------------------------------------------- |
| A=0           | Return saved `TEK_MODES` in A, not a fresh read of all hardware registers |
| A2:0≠0        | Replace saved `xx77` mode bits with A2:0                                  |
| A2:0=0        | Leave video unchanged; bit3 is not tested as a video-update flag          |
| A5:4=1/2/3    | Set saved clock state to 3.5/7/14 MHz respectively                        |
| A7:6=1 (`40`) | Set saved memory state to **1024K**                                       |
| A7:6=2 (`80`) | Set saved memory state to **48K**                                         |
| A7:6=3 (`C0`) | Set saved memory state to **128K**                                        |

Nonzero requests are stored as `TEK_MODES`; this is not a merge of all current hardware mode fields.
When changing memory mode, the routine returns without updating the saved A value that will be
restored to the application. Because zero low bits mean "unchanged", the advertised mode-0/`08h`
request cannot select ATM 320×200 in this implementation. The `define.a80` video constants also
disagree with the numeric ATM mode codes in some headers. Use explicit low-level ports for precise
mode changes, or deliberately fix the ROM implementation before relying on the advertised API. The
settings are temporary and are not a CMOS write.

**`4C MATH`: arguments and results as seen by the application:**

| Inline subfunction | Operation                          | Inputs                               | Outputs                                                 |
| ------------------ | ---------------------------------- | ------------------------------------ | ------------------------------------------------------- |
| `00`               | Unsigned 16-bit division           | **HL** dividend, DE divisor          | DE quotient                                             |
| `01`               | Unsigned 16×16 multiply            | BC, DE factors                       | BCDE product                                            |
| `02`               | Unsigned 32-bit division           | BCDE dividend, BC′DE′ divisor        | BCDE quotient, BC′DE′ remainder                         |
| `03`               | Unsigned 32×16 multiply            | BCDE multiplicand, **HL** multiplier | BC′DE′BCDE product stored across the listed eight bytes |
| `04`               | Unsigned 32-bit decimal conversion | BCDE number, HL destination          | Zero-terminated decimal string                          |
| `05`               | Unsigned 16-bit decimal conversion | DE number, HL destination            | Same                                                    |
| `06`               | Unsigned 8-bit decimal conversion  | E number, HL destination             | Same                                                    |

In concatenated register names the leftmost byte is most significant. Allocate **11 bytes for every
decimal conversion**: the common copy path copies that many bytes even for short values. The string
ends at its first zero; bytes beyond that terminator are not a useful result. Division by zero has
no documented error return. Reject a zero divisor in the application before calling.

The assembly comments around multiplication retain older register descriptions; the saved-register
loads/stores in `.X*` wrappers establish the table above. The PDF's BC dividend for divide16 and two
full 32-bit multiply factors do not match those wrappers.

Source: [`math.a80`](rom/page5/source/math.a80), [`addons.a80`](rom/page5/source/addons.a80).

<a id="rst-storage"></a>

### 17.5 Device, FAT, and image services

These functions use one shared state: selected volume, directory position, current file, cluster
cache, and ERS buffers. Opening another file replaces the previous file's state; there are no
independent open-file handles. Discover devices and select a volume before using FAT and image
operations.

#### Device manager: `50`, inline subfunction

| Subfunction | Name             | Contract                                                                                                                                            |
| ----------- | ---------------- | --------------------------------------------------------------------------------------------------------------------------------------------------- |
| `00`        | `DEVFIND`        | Discover/initialize devices and volumes; D=current volume, E=count, A=volume type (0 FAT12, 1 FAT16, 2 FAT32); CF=1 when none                       |
| `01`        | `SET_VOL`        | A=zero-based volume; CF=1 if invalid; changes current FAT context                                                                                   |
| `02`        | `KOL_VOL`        | Return current volume/count/type as for discovery                                                                                                   |
| `03`        | `GET_FNDVOLUME`  | HL=destination for 256-byte volume table                                                                                                            |
| `04`        | `TO_DRV`         | Consume a third inline byte `00–04` to invoke selected block-device driver                                                                          |
| `05`        | `SET_DEVICE`     | Internal device/head/LBA setup changes B inside the service but does not save that result for return to the application; use the driver call        |
| `06`        | `CONTROL_SD`     | A returns availability bits: bit0 onboard SD, bit1 NeoGS SD; media changes also update service state                                                |
| `07`        | `COMHDDNEX`      | Consume third byte to call Nemo driver directly; HL buffer, BCDE starting LBA, A sector count, A′ master/slave bit; see caveat below                    |
| `08`        | `FREINIT_VOL`    | Reinitialize volume state; this routine lacks complete argument transfer from the application's saved registers; use normal discovery and selection |
| `09`        | `SET_SECTOR_NUM` | HLBCDE=48-bit sector number, most-significant pair HL; stores internal `NUMSEC_LBA48`                                                               |

Driver selectors are `00 INIT`, `01 STATUS`, `02 READ`, `03 WRITE`, `04 READID`. For `07/02` and
`07/03`, inputs are HL=buffer, A=number of 512-byte sectors, BCDE=32-bit LBA, and bit 0 of A′ is
the drive (0 master, 1 slave). This is **not** the volume-table device ID (`04/05` for Nemo in the
current build). Select the Nemo volume with `SET_VOL` first: the bundled READ/WRITE driver takes
ATA device/head from that selected volume, so A′ alone does not switch master/slave. A count of zero
means 256 sectors and requires a 131072-byte buffer; use a nonzero count for ordinary buffers.
WRITE modifies data on the selected device.

This contract follows `rom/page5/source/fat/nemo_drv.a80` and
`rom/page5/source/fat/dev_drv.a80`. On return, A is the driver status: 0=success, 1=timeout,
6=command error, 7=CD/DVD detected. The
external call returns A/HL/DE/BC but does not always update the saved F register, so CF cannot be
used as the success indicator. In the bundled driver BCDE preserves the starting LBA, while HL
points after the bytes transferred (after the last byte actually transferred on a partial error).
The C wrapper returns `BC_OK` only to indicate that ERS returned; inspect the driver status in A.
SD and optional device drivers have their own init/status results. `09` storing a 48-bit number does
not make every driver's transfer path a fully supported 48-bit ATA API.

The volume table consists of 8-byte records: +0 device ID, +1 master/slave or zero for SD, +2 FAT
type 0/1/2, +3..6 little-endian starting sector, +7 reserved. The 256-byte copy includes service
metadata after the populated records. The returned count gives the number of volume records; the
table has 32 possible positions. Volume numbers are separate from floppy letters A–D. Older text
conflicts on a limit of 16 or 26 volumes, so use the count returned by discovery and stay within the
table's size.

Current device IDs from `nums_devices.a80`:

| ID      | Device                                                   |
| ------- | -------------------------------------------------------- |
| `00–03` | FDD A–D identifiers; not ordinary FAT discovery drivers  |
| `04/05` | Nemo master/slave                                        |
| `06/07` | SMUC master/slave                                        |
| `08/09` | DivIDE master/slave                                      |
| `0A/0B` | Profi master/slave                                       |
| `0C/0D` | ATM Turbo 2+ master/slave                                |
| `0E`    | NeoGS SD                                                 |
| `0F`    | Onboard/Z-controller SD                                  |
| `10`    | SL811 USB Flash identifier; `USB_ENABLE=0` in this build |

An identifier or header declaration does not establish a fitted interface or an enabled driver. The
current discovery routine actively tries Nemo master, optional NeoGS SD, and onboard SD; USB is
compiled out.

#### FAT driver: `51`, inline subfunction

| Subfunction | Name            | External arguments/results                                                                                                                                                             |
| ----------- | --------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `00`        | `INIT_FATVARS`  | Internal volume initialization; application code should call the device manager                                                                                                        |
| `01`        | `READ_DIR`      | Input: BC=zero-based physical 32-byte record number in the current directory, HL=32-byte buffer. Copies the entry and makes it current for subsequent calls                            |
| `02`        | `ENTER_DIR`     | Enter current directory entry, or parent for `..`; depends on current entry                                                                                                            |
| `03`        | `GET_PATH`      | HL=destination for current path; allocate a 256-byte buffer                                                                                                                            |
| `04`        | `GET_LONGNAME`  | HL=256-byte name buffer; long name or 8.3 fallback, zero-terminated                                                                                                                    |
| `05`        | `FIND_NAME`     | Input: HL=short filename string (`filename.ext`, not padded). Output: BC=found record number, CF=1 if not found; see string-termination rules below                                    |
| `06`        | `FIND_FILEITEM` | Collect matching entries in the current directory. Output: BC=entry count, A=`RAM_SORTFILES`, **inverted port value 03** (physical page FC); results reside in the internal ERS buffer |
| `07`        | `SET_MASK_EXT`  | A=0 clears filter; A≠0, HL=consecutive 3-byte extensions ending with zero; up to 16                                                                                                    |
| `08`        | `OPEN_FILE`     | Open current entry for sequential reading; initializes shared file state                                                                                                               |
| `09`        | `READ_FILE`     | A=512-byte sector count, HL=destination; CF=1 on end of file or error; returned HL advances as data is read                                                                            |
| `0A`        | `INIT_TEKDIR`   | Internal current-directory cluster table creation; not a general application call                                                                                                      |
| `0B`        | `POS_FILES`     | A=position operation below, BC=position argument/result                                                                                                                                |
| `0C`        | `ROOTDIR`       | Reset current volume to its root directory                                                                                                                                             |

`FIND_NAME` terminates at a byte below `21h`, including a space; it is a short name search, not a
general long-name search with spaces. `SET_MASK_EXT` uses fixed 3-byte groups: for example
`"TRDSCLFDI",0`, with padding for shorter extensions. Directories are retained by the search; the
`.` entry is skipped.

For `POS_FILES`, A=`00` saves current position, `01` restores it, `02` resets to the first legal
entry, `03` moves B files backward, `04` B forward, `05` counts legal entries, `06` selects legal
ordinal BC, `07` returns it. Legal entries exclude deleted entries and long-name continuation
records. Physical FAT record positions and legal ordinals are not interchangeable.

The usual read sequence is discovery → select volume → find/select entry → open → read sectors.
Reads are sector-granular; use the directory entry's file size to interpret the last sector. The
interface does not return a POSIX-style byte count and is not a FAT file-writing API.

`52 SORT_FINDFILES` sorts the **existing internal** results, placing directories before files
according to the ROM's character-order table. It does not accept an arbitrary application array. The
service's found-entry and sort buffers are in physical FC, addressed by inverted port value 03.

#### Mounter: `53`, inline subfunction

This is the **current `TAB_MOUNTER` order**, not the old PDF or C header order:

| Subfunction | Name            | Arguments / results                                                                                        |
| ----------- | --------------- | ---------------------------------------------------------------------------------------------------------- |
| `00`        | `OPEN_MOUNT`    | A=drive 0–3; mount file selected by current FAT entry; TRD is supported for direct access to an image file |
| `01`        | `RDWR_MOUNT`    | A1:0=drive, A7=write (0=read), D=logical track, E=sector **1-based**, HL=256-byte buffer                   |
| `02`        | `FIND_MOUNTED`  | Parse `IMAGE.MNT` and mount configured images                                                              |
| `03`        | `GET_MOUNTED`   | HL=destination for four 64-byte descriptors (256 bytes)                                                    |
| `04`        | `CLOSEMOUNT`    | A=drive 0–3; dismount image                                                                                |
| `05`        | `LOADIMAGE`     | Load selected TRD/SCL/FDI into RAM disk, or TAP into tape memory                                           |
| `06`        | `REST_NAMELOAD` | HL=destination for 32-byte source-file descriptor of RAM-disk image                                        |
| `07`        | `CMP_DRIVE`     | A=drive; CF=1 if no mounted image, CF=0 if present; also replaces A                                        |
| `08`        | `GET_VIRT_BITS` | A bits0–3=mounted A–D, bits4–7=RAM-disk assignment A–D; refreshes internal drive state                     |
| `09`        | `SET_VIRTREAL`  | A=drive 0–3; set real and virtual drive assignment together                                                |
| `0A`        | `SET_REAL`      | A=real drive 0–3; update persistent assignment                                                             |
| `0B`        | `SET_VIRT`      | A=virtual/RAM-disk drive 0–3; update persistent assignment                                                 |

The old PDF inserts a nonexistent operation at 06 and shifts subsequent functions, while `ers.h`
calls 09 `MOUNT_RAMDISK`. In this source the dedicated RAM-disk mount helper is disabled/commented
out; use the actual table above. Some operations do not copy an error result into `RREG_*`. Check
the relevant state with the provided query operations; CF alone may not indicate success.

Mounted-image descriptor (64-byte record; offsets hexadecimal):

| Offset  | Size | Meaning                                                                                                        |
| ------- | ---- | -------------------------------------------------------------------------------------------------------------- |
| `00–1F` | 32   | FAT directory entry, including filename, attributes, cluster and file size                                     |
| `20`    | 1    | Internal flags byte; no stable application bit layout is defined                                               |
| `21`    | 1    | Internal file-type number derived from the extension; not extension text or a device ID                        |
| `22`    | 1    | Number of clusters between cached FAT-chain positions, used to locate image sectors quickly; not a sector size |
| `23`    | 1    | Filesystem of the volume holding the image: 0=FAT12, 1=FAT16, 2=FAT32                                          |
| `24`    | 1    | Zero-based index in the device manager volume table; not a floppy-drive letter                                 |
| `25`    | 2    | Mounted-image device-driver address                                                                            |
| `27`    | 2    | Saved selected-volume driver address                                                                           |
| `29`    | 1    | Drive assigned to the image: 0=A, 1=B, 2=C, 3=D                                                                |
| `2A–3F` | 22   | Remaining internal/reserved storage                                                                            |

Sizes are in bytes; two-byte addresses are little-endian. `GET_MOUNTED` returns a snapshot of
internal descriptors. Editing the copy does not mount an image or change a drive; use mounter calls
for those operations.

A driver address points to code in this particular ROM build. Do not retain it as a permanent
identifier or call it after changing ROM versions. Additional per-drive path tables and caches are
not included in the returned 256 bytes.

`54 INST_FATBOOT` unpacks the embedded micro-loader into its implementation-defined RAM location and
copies current FAT variables to HL. It is used by the ROM's Hobeta/SPG boot flow. Integrations must
follow `UNP_MICRO_BOOT` and the loader format. Internal absolute addresses may change between
versions.

Sources: [`dev_drv.a80`](rom/page5/source/fat/dev_drv.a80),
[`nemo_drv.a80`](rom/page5/source/fat/nemo_drv.a80),
[`read_fat.a80`](rom/page5/source/fat/read_fat.a80),
[`mounter.a80`](rom/page5/source/fat/mounter.a80),
[`struct_mount.a80`](rom/page5/source/struct_mount.a80),
[`addon2.a80`](rom/page5/source/addon2.a80), [`nums_devices.a80`](rom/nums_devices.a80).

<a id="rst-cmos"></a>

### 17.6 CMOS service: `55`

| Inline subfunction | Name         | Contract                                                                           |
| ------------------ | ------------ | ---------------------------------------------------------------------------------- |
| `00`               | `INIT_CMOS`  | Check ERS settings CRC; install defaults if invalid                                |
| `01`               | `READ_CMOS`  | Input: H=RTC index. Output: A and L contain the byte read; Z=1 if it is zero, CF=0 |
| `02`               | `WRITE_CMOS` | H=index, L=value; write and recompute ERS CRC                                      |
| `03`               | `CLEAR_CMOS` | Force default ERS settings                                                         |

`CLEAR_CMOS` resets ERS's setup cells, not every NVRAM/AVR EEPROM byte. The source CRC input is
seven bytes `ED..E7`; stored CRC is `EE/EF`. Raw RTC access remains available for other
application-owned cells and is faster than the complete service wrapper. Calls on extension cells
still have the global selection/EEPROM side effects described in §11.

Source: [`call_cmos.a80`](rom/page5/source/call_cmos.a80).

### 17.7 Other RST vectors and cross-page calls

An RST instruction calls a fixed CPU address. The routine at that address depends on the ROM/RAM
page mapped into window 0, so the same RST instruction can perform different actions under different
memory maps.

| Vector / entry         | Meaning in the relevant service context                                                        |
| ---------------------- | ---------------------------------------------------------------------------------------------- |
| `0000`                 | Reset in `ROM_ERS`; in the installed RAM service image, a `JP CALL2PAGE` stub                  |
| `0008`                 | Public service route in supported ROM stubs; ordinary BASIC error route in an unpatched ROM    |
| `0010`, `0018`, `0028` | No new universal Evolution application service; several private ROM pages contain placeholders |
| `0020` (`RST 20h`)     | Private `CALL2RAMPAGE` in service pages that install this vector                               |
| `0030` (`RST 30h`)     | Private `CALL2PAGE` in service pages that install this vector                                  |
| `0038`                 | IM 1/frame-interrupt entry; private service pages may just execute `EI; RET`                   |
| `0066`                 | NMI entry (not a RST instruction); hardware NOP and FF-page overlay                            |

Private cross-page convention:

```asm
        rst 0x30           ; ROMCALL, only in the installed service context
        dw target_address
        db rom_port_value

        rst 0x20           ; RAMCALL, same contextual restriction
        dw target_address
        db inverted_ram_page
```

`CALL2PAGE` saves the current page number on ERS's internal page stack, maps the target page into
window 0, and calls the supplied address. When the called routine executes RET, it restores the
previous page. The RAM variant switches pages through x7F7.

These routines modify their own code and use ERS's internal page stack. They work only with the
service environment installed. An application that replaces the handlers or reuses ERS workspace
must provide its own cross-page calls.

Fixed private entries in `ROM_RST83` include `0014` (page-switch stub), `001C` (DOS entry), `0024`
(page-0 entry path), `002C` (dispatcher continuation), and `0034` (NMI continuation).
`ROM_RST81:0040` installs the RST/NMI environment; `ROM_MAINMENU:0101` unpacks the main menu into
RAM. Treat them as firmware integration points requiring the matching ROM build, not stable public
calls.

The header's `_BIOS` name and jump constants beginning at `0100` for keyboard, device, or allocation
helpers do not create an additional public RST BIOS in the current `CODE_TABL`. For ATM CP/M, its
banked BIOS is a separate software environment under [`rom/atm_cpm/source`](rom/atm_cpm/source/);
CP/M conventions must not be confused with ERS selectors or the enhanced controller's IM 0 restart
opcodes.

Sources: [`macros.a80`](rom/macros.a80), [`nmi_service.a80`](rom/page5/source/nmi_service.a80),
[`rst8service.a80`](rom/page5/source/rst8service.a80), [`define.a80`](rom/define.a80),
[`ers.h`](rom/c/ers.h).
