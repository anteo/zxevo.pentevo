# BaseConf

`lib/baseconf` is a C and Z80 assembly library for ZX Evolution applications running the BaseConf
FPGA profile. It brings together BaseConf memory-map and port access with video modes, graphics,
text, input, RTC/AVR, audio, storage, and selected ERS and interrupt interfaces. PT3 playback is
available through a wrapper around z88dk's Vortex Tracker II player; the module must remain
resident, and the application calls the player at a steady rate.

Public headers are in `lib/baseconf/include/`. The library uses the usual z88dk linking model: only
archive members needed by an application are linked. It does not replace the application's CRT or
system environment, but provides a target configuration and a local CRT hook for the BaseConf
console.

## Requirements and build

You need z88dk with the SDCC compiler and classic library, plus `zcc` and `z88dk-z80asm`. From the
repository root, build the archive with:

```sh
make -C lib/baseconf
```

The output is `lib/baseconf/build/default/baseconf.lib`. If the tools are installed in non-default
locations, pass `ZCC` and `Z80ASM`; the project build uses `Z88DK_ROOT`, `Z88DK_SRC`, and `ZCCCFG`
to locate z88dk resources.

## Using the library in an application

Include the library's shared makefile from the application's Makefile. It sets the absolute paths to
the target configuration, headers, and archive, along with the matching ABI flags:

```make
ZCC ?= zcc
Z80ASM ?= z88dk-z80asm
include ../lib/baseconf/baseconf.mk

APP_FLAGS := +$(BASECONF_CFG) $(BASECONF_ABI_FLAGS) \
             -I$(BASECONF_INCLUDE) -L$(BASECONF_BUILD)
```

Use these flags when building the application and link `baseconf` (for example, with `-lbaseconf`).
Adjust the include path relative to the application directory. The BaseConf configuration relies on
z88dk's standard configuration directory (`ZCCCFG`) for the standard headers and libraries. When
invoking `zcc` directly, set `BASECONF_ROOT` to the absolute `lib/baseconf` path and select
`+<absolute-path>/config/baseconf.cfg`.

The main library build options are:

- `BASECONF_VIDEO_TEXT_MODES` and `BASECONF_VIDEO_PRIMITIVE_MODES` accept `ALL`, `GRAPHICS`, `NONE`,
  or a list of modes;
- `BASECONF_CONSOLE_NO_DEFAULT_FONT=1` omits font8 when the application uses a different font.

Rebuild the archive and relink the application after changing its mode or font settings.

## Application size

`baseconf.lib` is an archive, not one object copied wholesale into the application. The linker pulls
members referenced by the application and their dependencies. Unused APIs and mode-specific
implementations are generally left out; one member may group several related functions, and a
function may pull shared helper members. The final size depends on the application's call graph and
link layout, so use its linker map for the exact result.

- **`CRT_ENABLE_STDIO`** is an application compile-time setting, for example
  `-pragma-define:CRT_ENABLE_STDIO=0`. With `1`, z88dk standard stdio remains available, including
  familiar `printf` and stream support; this is convenient but can add substantial code. With `0`,
  standard stdio is disabled, so do not call ordinary `printf`/`scanf`; the BaseConf CRT's generic
  console hook is disabled as well. Direct BaseConf console functions remain available. For compact
  formatted output use `bc_printf()`, which does not depend on CRT stdio. For direct output use
  `bc_console_putc()`, `bc_console_gotoxy()`, `bc_console_clear()`, or `bc_text_putc()`,
  `bc_text_puts()`, `bc_text_gotoxy()`, and `bc_text_clear()` in hardware text modes.
- **`BASECONF_VIDEO_TEXT_MODES`** selects which mode-specific text and console implementations are
  built into the library. It accepts `ALL`, `GRAPHICS`, `NONE`, or a list of modes such as `ZX_256`,
  `ATM_640`, and `ATM_TEXT_80`. Keep only the modes where the application outputs text.
- **`BASECONF_VIDEO_PRIMITIVE_MODES`** similarly limits mode-specific graphics primitive
  implementations. Here `GRAPHICS` means the five graphics modes; 80×25 text modes do not add a
  graphics primitive backend. List only the modes the application needs.
- **`BASECONF_CONSOLE_NO_DEFAULT_FONT=1`** removes the console's automatic reference to the built-in
  font8. An application can still explicitly use font8, but the linker will not pull it in just for
  the default selection. In graphics modes, select another font with `bc_console_set_font()` before
  printing.
- **Graphics primitives:** when the application needs only a few lines, circles, ellipses, or
  horizontal/vertical spans, use the direct `bc_draw_*` entry points. SET/CLEAR/XOR shortcuts
  and explicit-operation functions are separate entry points; call only those needed to avoid
  pulling unrelated archive members. Several ASM functions may share one member, which is linked as
  a unit. The portable z88dk classic interface (`plot`, `draw`, `circle`, etc.) is convenient, but
  its wrappers and supporting routines can also increase the image size.

Example: build the library with only the required modes:

```sh
make -C lib/baseconf \
  BASECONF_VIDEO_TEXT_MODES="ZX_256 ATM_TEXT_80" \
  BASECONF_VIDEO_PRIMITIVE_MODES="ZX_256" \
  BASECONF_CONSOLE_NO_DEFAULT_FONT=1
```

Mode and font options change the contents of `baseconf.lib`; set `CRT_ENABLE_STDIO` separately when
compiling the application. Link an archive built with the modes the application needs. These options
mainly reduce linked code and constant data; they do not replace RAM planning for DATA/BSS and
stack.

To inspect a linked application's memory map, use:

```sh
python3 lib/baseconf/tools/memory_map.py path/to/application.map
python3 lib/baseconf/tools/memory_map.py path/to/application.map --sort size --name-width 36
```

The report shows address ranges and sizes for memory areas, modules, and functions, along with
source files and lines. Range end addresses are exclusive; function boundaries are inferred from the
next function symbol in the linker map. Names are truncated to the name-column width (48 characters
by default; set it with `--name-width`). `--sort size` sorts areas, modules, and functions by
descending size; address order is the default. Aliases may share a range, and anonymous bytes are
included in the nearest range.

## Initialization and setup

Call `bc_init()` after application startup and before using the library's high-level features. The
default ERS profile is the service ROM. Initialization prepares library state but does not configure
maps or memory placement for the application.

The application is responsible for memory maps and screen surfaces. Alongside initialization, an
application can choose an ERS profile with `bc_ers_set_profile()`, override the pinned-window
mask with `bc_set_pinned_windows()`, or report map preparation through `bc_set_maps_prepared()`. By
default, the library derives the pinned mask from code, DATA/BSS, and the current stack.
Page-manager control is available through `bc_set_page_manager_enabled()` and uses the prepared-maps
flag. Drawing pages must be mapped as writable RAM in windows W1–W3. Normal drawing operations do
not remap windows or change memory protection.

```c
if (bc_init() != BC_OK) return 1;
```

If the application needs the page manager, configure both maps and pin every window
that must remain accessible. Then report map readiness and enable the page manager.

```c
bc_set_maps_prepared(1);
if (bc_set_page_manager_enabled(1) != BC_OK) return 1;
```

## Implemented interfaces

The examples below are independent snippets for an application that includes
`<baseconf.h>` and has called `bc_init()`. Memory and build requirements are listed
alongside the examples.

### Memory and banking

Use `bc_get_active_map()` to query the active map, `bc_configure_window()` to configure a
window in a selected map, and `bc_window_acquire()` to borrow a window temporarily.
Pair each successful acquisition with `bc_window_release()`. The library maps RAM
and Flash banks and controls window write protection. [memory.h](include/baseconf/memory.h)

The z88dk near heap (`malloc`, `calloc`, `realloc`, `free`) is not initialized by default, so
applications that do not allocate memory avoid the extra startup code. To enable a heap between the
end of BSS and the current stack, set a positive stack reserve:

```c
#pragma output CRT_STACK_SIZE = 512
```

In this mode, `malloc` uses the contiguous free range from the end of BSS to `SP - CRT_STACK_SIZE`.
The value `512` reserves space for the stack; it does not guard against stack overflow, so choose a
value for the application. This only works with RAM that remains continuously accessible at those
addresses. It does not allocate physical banks or switch BaseConf windows.

For banked pages, use `bc_ram_page_pool_t`. BaseConf has a fixed set of 64 RAM pages, each 16 KiB,
so pool membership is configured with one 8-byte mask: a set bit includes a page, and a clear bit
excludes it. For example, to exclude pages 2 and 5, set the first mask byte to `0xDB` and the
remaining bytes to `0xFF`. The pool does not discover pages used by the application or system, clear
page contents, or map pages into windows; construct the mask from the application's memory layout.
Allocation state is stored inside `bc_ram_page_pool_t`. Map an allocated page temporarily with
`bc_window_acquire()` and `bc_window_release()`, then return it to the pool with
`bc_ram_page_free()`.

Use `bc_ram_heap_t` to allocate objects from that pool. It takes pages from the pool as needed and
uses classic z88dk named heaps within each page. Allocation returns a `bc_ram_heap_ptr_t` handle
(`page + offset`), not a normal C pointer. Call `bc_ram_heap_map()` before reading or writing the
object and `bc_ram_heap_unmap()` when finished. The pointer is valid only while its window is
mapped, and only one lease can be active at a time. Keep the heap object, its `bc_ram_heap_page_t`
storage, the mask, and the pool object in pinned memory outside the window being switched.
`bc_ram_heap_destroy()` returns all backing pages to the pool and invalidates the heap's handles.

```c
static const bc_ram_page_mask_t page_mask = {
    0xDB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};
static bc_ram_page_pool_t pool;

bc_ram_heap_t heap;
bc_ram_heap_page_t heap_pages[4];
bc_ram_heap_ptr_t item;
void *address;

/* Pages 2 and 5 are excluded; verify the other pages against the app layout. */
if (bc_ram_page_pool_init(&pool, page_mask) != BC_OK ||
    bc_ram_heap_init(&heap, &pool, heap_pages, 4, BC_WINDOW_2) != BC_OK ||
    bc_ram_heap_alloc(&heap, 128, &item) != BC_OK) {
    return 1;
}

if (bc_ram_heap_map(&heap, item, &address) != BC_OK) return 1;
((uint8_t *)address)[0] = 42;
if (bc_ram_heap_unmap(&heap) != BC_OK) return 1;

if (bc_ram_heap_free(&heap, item) != BC_OK) return 1;
if (bc_ram_heap_destroy(&heap) != BC_OK) return 1;
```

Flash programming is not implemented.

### Video modes and graphics

The library supports ZX 256, ZX Multicolor, Pentagon 16-color, ATM 320 and ATM 640,
plus ATM and Evo 80×25 hardware text modes. Build options select the output modes
and graphics primitives available to the application. Draw lines, circles and
ellipses with SET, CLEAR or XOR. Use `bc_read_pixel_index()` to inspect a pixel
on the selected draw surface.

Before changing video modes, prepare both maps and enable the page manager as
described above. `bc_set_video_mode()` maps screen pages into unpinned windows
as needed. This example clears and draws on the currently displayed screen in ZX 256 mode.

```c
if (bc_set_video_mode(BC_VIDEO_ZX_256) != BC_OK) return 1;
bc_set_ink(7);
bc_set_paper(0);
if (bc_console_clear() != BC_OK) return 1;
if (bc_graphics_begin() != BC_OK) return 1;
bc_draw_line(16, 16, 96, 64);
bc_draw_circle(96, 64, 16);
if (bc_graphics_end() != BC_OK) return 1;
```

For double buffering, map both screens into writable RAM first.
`bc_select_draw_screen()` picks the draw target, and `bc_show_screen()` selects
the displayed screen. In ZX mode, primitives update the bitmap and preserve the
attributes. [video.h](include/baseconf/video.h) · [graphics.h](include/baseconf/graphics.h)

In z88dk classic, use `#include <games.h>` for software sprites with `putsprite()`,
background save and restore with `bksave()` and `bkrestore()`, and joystick input
with `joystick()`. The ZX implementation of `putsprite()` uses the standard bitmap
at `0x4000`. Map the intended screen page into W1 and keep the routine in writable RAM.
[z88dk games.h](https://github.com/z88dk/z88dk/blob/master/include/games.h)

### Text, fonts and palettes

The console provides cursor-based text in graphics modes and in 80×25 hardware
text modes. In graphics modes, use the built-in 8×8 or 5×7 font or pass a custom
descriptor to `bc_console_set_font()`. Use `bc_text_puts()` for hardware text.
`bc_printf()` formats strings and 16-bit integers without stdio. This example
uses an archive built with Evo 80×25 text support.

```c
if (bc_set_video_mode(BC_VIDEO_EVO_TEXT_80) != BC_OK) return 1;
bc_set_ink(7);
bc_set_paper(0);
if (bc_text_clear() != BC_OK) return 1;
if (bc_text_gotoxy(2, 1) != BC_OK) return 1;
if (bc_printf("Screen: %u x %u", bc_video_width(), bc_video_height()) < 0) return 1;
```

Upload a 2 KiB hardware font with `bc_upload_font_to()` or
`bc_upload_font_preserving()`. In ZX modes, `bc_set_ulaplus_palette()` addresses
ULAplus entries by index. This example sets color 3 in the active ZX mode.

```c
if (bc_set_ulaplus_palette(3, 0x9e) != BC_OK) return 1;
bc_set_ulaplus_enabled(1);
```

ATM RGB222/RGB444 writes target the color currently under the raster beam, so the
application controls write timing.
[console.h](include/baseconf/console.h) · [text.h](include/baseconf/text.h) ·
[printf.h](include/baseconf/printf.h) · [font.h](include/baseconf/font.h) ·
[palette.h](include/baseconf/palette.h)

### Mouse and keyboard

For ZX keyboard input, use `#include <input.h>` from z88dk classic. `in_Inkey()`
polls for an ASCII character, and `in_LookupKey()` and `in_KeyPressed()` let you
check a specific key or key combination.
[z88dk input.h](https://github.com/z88dk/z88dk/blob/master/include/input.h)

Kempston mouse snapshots provide counters and button bits. `bc_mouse_delta()`
handles counter wraparound. Keep the previous snapshot between polls and poll
often enough that motion fits in the signed delta.

```c
bc_mouse_snapshot_t previous, current;
if (bc_mouse_read(&previous) != BC_OK) return 1;
/* On the next input tick: */
if (bc_mouse_read(&current) != BC_OK) return 1;
bc_mouse_delta_t motion = bc_mouse_delta(&previous, &current);
previous = current;
```

The PS/2 API reads physical modifiers and raw scancodes from the AVR log.
Establish the current window with `bc_avr_window_select()` before reading the log.
After `BC_OK`, check for `BC_PS2_SCANCODE_EMPTY` and `BC_PS2_SCANCODE_OVERFLOW`.

```c
uint8_t code;
if (bc_avr_window_select(BC_AVR_WINDOW_MODES) != BC_OK) return 1;
if (bc_ps2_scancode_read(&code) != BC_OK) return 1;
if (code != BC_PS2_SCANCODE_EMPTY && code != BC_PS2_SCANCODE_OVERFLOW) {
    /* Pass the byte to the application's scancode decoder. */
}
```

[input.h](include/baseconf/input.h) · [ps2.h](include/baseconf/ps2.h)

### RTC, AVR and timing

The RTC API reads date and time as binary or BCD and provides access to clock
registers and NVRAM. The AVR extension window provides firmware metadata,
video output settings and EEPROM access. Keep the window selection stable
during an operation, including from NMI. `bc_frame_period()` reports the nominal
frame period in 3.5 MHz T-states regardless of CPU speed.

```c
bc_datetime_t now;
uint32_t frame_tstates;
if (bc_rtc_read_datetime(&now, BC_RTC_BINARY) != BC_OK) return 1;
if (bc_avr_window_select(BC_AVR_WINDOW_MODES) != BC_OK) return 1;
if (bc_frame_period(&frame_tstates) != BC_OK) return 1;
```

`bc_set_cpu_speed()` selects 3.5, 7 or 14 MHz. Interrupt support covers saving and
restoring INT, requesting NMI, setting an execution trap, and configuring,
querying and acknowledging newirq sources. The application installs its handlers.
[rtc.h](include/baseconf/rtc.h) · [timing.h](include/baseconf/timing.h) ·
[interrupt.h](include/baseconf/interrupt.h) · [newirq.h](include/baseconf/newirq.h)

### UART

The UART API configures framing and the divisor, reads line and modem status,
and sends and receives bytes. This example uses 8N1 with divisor 1 and waits up
to 4096 polls for one byte. Configure the peer to match. An expired poll budget
returns `BC_BUSY`.

```c
const bc_uart_config_t config = {BC_UART_BITS_8, BC_UART_PARITY_NONE, 1, 1};
uint8_t byte;
if (bc_uart_configure(&config) != BC_OK) return 1;
bc_status_t received = bc_uart_wait_receive_polls(&byte, 4096);
```

[uart.h](include/baseconf/uart.h)

### Audio

In z88dk classic, `#include <sound.h>` provides beeper routines such as
`bit_beep()` and `bit_fx()`, plus simple AY melodies with `psg_play()`.
Use `#include <psg.h>` for direct control of AY tones, volume and registers.
[z88dk sound.h](https://github.com/z88dk/z88dk/blob/master/include/sound.h) ·
[z88dk psg.h](https://github.com/z88dk/z88dk/blob/master/include/psg.h)

`bc_covox_play()` plays packed 4-bit samples, high nibble first, and blocks until
playback ends. For AY, the library wraps the z88dk PT3 player. Keep the module
mapped and call `bc_pt3_play()` once per music tick. This example assumes one
music tick per frame and a frame interrupt as the CPU wake source.

```c
extern const uint8_t music_pt3[];
uint16_t frame;
if (bc_pt3_init(music_pt3) != BC_OK) return 1;
bc_pt3_start();
for (frame = 0; frame < 250; ++frame) {
    if (bc_wait_frame() != BC_OK) break;
    bc_pt3_play();
}
bc_pt3_stop();
```

[audio.h](include/baseconf/audio.h)

### Direct storage access

`baseconf/storage.h` is the low-level interface to the Nemo IDE, SD SPI and VG93 controllers.
The 512-byte IDE and SD sector calls issue controller commands directly. The volume
selected in ERS has no effect on them. For VG93, the API covers drive selection,
the system port, controller registers and the hardware overlay mask.

Nemo IDE supports LBA28 on device 0, IDENTIFY and write-cache flushing. This example
reads the first disk sector. The budget counts ATA status polls.

```c
static uint8_t sector[512];
if (bc_ide_read_sector(0, sector, 60000u) != BC_OK) return 1;
```

Initialize SD to determine its addressing type. `bc_sd_load()` also reads CID and
CSD, and `bc_sd_sector_count()` decodes capacity from CSD. The FPGA controller
uses a fixed SPI clock. The read budget counts SPI bytes, and the initialization
budget counts attempts.

```c
static uint8_t sector[512];
bc_sd_card_type_t card;
if (bc_sd_init(&card, 2000) != BC_OK) return 1;
if (bc_sd_read_sector(card, 0, sector, 60000u) != BC_OK) return 1;
```

To read the VG93 status register, exclude NMI and keep the FE overlay mask at zero.
Reading `BC_FDD_COMMAND` returns controller status.

```c
uint8_t fdd_status;
if (bc_fdd_reg_read(BC_FDD_COMMAND, &fdd_status) != BC_OK) return 1;
```

This layer serves the same role as z88dk's `sdcard.h` and `diskio`. Use the word,
block and SPI transaction APIs for custom commands. CSD decoding uses z88dk's
`extract_bits`. The ZEsarUX SD test covers commands, tokens and blocks.
Testing full initialization and write-busy release requires a card model that
implements those phases. [storage.h](include/baseconf/storage.h)

### ERS ROM services

ERS provides ROM-backed storage access. Its device manager and FAT routines work
with supported HDD and SD volumes, and its mounter manages floppy drives and disk
images. The library also wraps ROM version, ERS windows and text, fonts and palette,
CMOS, tape, printer and arithmetic services. The application prepares the ROM, RAM
and BASIC state required by `RST 08h`. Check firmware result fields as well as
the wrapper status for storage operations.

This example opens a file at the root of the current FAT partition reported by ERS.

```c
static const char name[16] = "README.TXT";
bc_ers_device_info_t volume;
bc_ers_fat_search_result_t file;
if (bc_ers_device_find(&volume) != BC_OK || !volume.found) return 1;
if (bc_ers_device_set_volume(volume.current_partition) != BC_OK) return 1;
if (bc_ers_fat_root_directory() != BC_OK) return 1;
if (bc_ers_fat_find_short_name(name, &file) != BC_OK || file.not_found) return 1;
if (bc_ers_fat_open_file() != BC_OK) return 1;
```

[ers.h](include/baseconf/ers.h) · [rom.h](include/baseconf/rom.h) ·
[ERS contracts in the manual](../../MANUAL.en.md#rst-storage)

## Tests and examples

The test suite is in `lib/baseconf/tests/`. It builds cases and runs them in ZEsarUX with BaseConf
support. Its README describes the case structure and how to run the suite:
`lib/baseconf/tests/README.md`. The main command is `make -C lib/baseconf/tests test`.

Small examples of library usage are in `lib/baseconf/tests/cases/`. These are test programs rather
than a separate application-example collection.

## Main headers

- `baseconf.h`, `baseconf/memory.h` — initialization, memory, and maps.
- `baseconf/video.h`, `graphics.h`, `text.h`, `console.h`, `printf.h`, `palette.h`, `font.h` —
  video modes and output.
- `baseconf/input.h` — Kempston mouse.
- `baseconf/ps2.h`, `audio.h`, `uart.h` — PS/2, audio, and serial I/O.
- `baseconf/rtc.h`, `timing.h`, `interrupt.h`, `newirq.h` — RTC/AVR, timing and interrupts.
- `baseconf/storage.h`, `rom.h`, `ers.h` — direct storage access and ROM services.
