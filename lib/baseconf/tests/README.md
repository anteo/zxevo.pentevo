# BaseConf tests

This directory contains the BaseConf emulator test framework and the older TAP probes. Framework
documentation is below; the console TAP tests are at the end. The font benchmark lives in
[`../bench/README.md`](../bench/README.md).

## Test framework

The framework runs library tests in ZEsarUX: it builds a C case, loads it through ZRCP/ZXHost,
executes steps and compares screen, text and data with expected files.

`cases/` contains package subdirectories with framework and functional cases. The runner discovers
`*.c` cases recursively. `list` shows paths relative to `cases/`, without the `.c` suffix. Expected
files sit beside their case and use the file stem.

### Quick start

```sh
make -C lib/baseconf/tests test            # build and check all cases
make -C lib/baseconf/tests shots           # write expected files (PASS only)
make -C lib/baseconf/tests framework-list  # list cases and expected files
python3 lib/baseconf/tests/run.py check --case core/helpers
python3 lib/baseconf/tests/run.py check --case core
```

Requirements: `zcc`, `z88dk-z80asm`, ZEsarUX with ZRCP, Python 3.

### Layout

```
lib/baseconf/tests/
  run.py                 runner
  profiles.json          shared settings and composable build layers
  Makefile               legacy TAP targets and framework targets
  cases/
    core/smoke.c                    framework self-check
    core/smoke.json                 optional case metadata
    core/smoke.01_zx256.png         expected file for step 1
    console/bc_printf.c       console package case
    graphics/graphics_primitives.c  graphics package case
    audio/audio.c                   audio package case
  harness/
    bc_test.h            test API
    bc_test.c            steps, ZXHost, channels, assertions
    mmap.inc             DATA/BSS layout
  build/<package>/<case>/   build, manifest, map (ignored)
  out/<package>/            actual files, diff, report, log
    core/smoke.01_zx256.png
    core/smoke.report.json
    core/smoke.zesarux.log
```

### Runner pipeline

1. Finds `cases/**/*.c` and reads the matching `.json` file beside each case.
2. Builds the case for each profile with `zcc`:
   - classic + SDCC, `+config/baseconf.cfg`, `-SO3 -m`;
   - `CRT_ORG_CODE=0x0100`, `CRT_ORG_DATA=0x6800`, `CRT_ORG_BSS=0x7000`, `REGISTER_SP=0x7E00`;
   - case sources, `harness/bc_test.c`, `lib/zxhost/src/zxhost.c`;
   - `zxhost_config.h` is generated from `lib/zxhost/protocol.json`.
3. Reads `.map` and validates addresses and sizes.
4. Writes `manifest.json` and the `code0`, `code1`, `DATA`, `vectors` images.
5. Starts ZEsarUX `--machine BaseConf` and loads the blocks:
   - `VECTORS` — `RET/RETI/RETN` at 0x0038/0x0066;
   - `CODE0` in W0 (page 32), `CODE1`/`DATA` in W1 (page 33);
   - `OUT 0xBF,1` and W0/W1 mappings.
6. Runs the steps:
   - waits for `DONE` with the expected `sequence`;
   - reads `result0..3` and the log;
   - on the `DATA` flag, reads `DATA <addr> <len> <kind>` and the memory;
   - on the `SHOT` flag, advances 200,000 cycles and saves the screen;
   - on the `MORE` flag, sends the next request; otherwise ends the session.
7. Compares the outputs with the expected files next to the case and writes
   `out/<package>/<case>.report.json`.

### Memory map

| Window | Addresses   | Contents                       | Page    |
| ------ | ----------- | ------------------------------ | ------- |
| W0     | `0000–3FFF` | vectors and code/rodata        | RAM 32  |
| W1     | `4000–7FFF` | code, ZXHost, DATA, BSS, stack | RAM 33  |
| W2     | `8000–BFFF` | screen surface 0               | `page0` |
| W3     | `C000–FFFF` | screen surface 1               | `page1` |

| Region          | Address         | Limit                      |
| --------------- | --------------- | -------------------------- |
| code and rodata | `0100–5FFF`     | 24,320 bytes, end ≤ `6000` |
| ZXHost header   | `6000–601F`     | —                          |
| ZXHost log      | `6100–67FF`     | 1792 bytes                 |
| DATA            | `6800–6FFF`     | 2 KiB                      |
| BSS             | `7000–…`        | end + 1 KiB ≤ `7E00`       |
| stack           | `7E00` downward | —                          |

`run.py` checks these bounds from `.map` and stops the build on violation. Large initialized tables
go in `const` (rodata), or `data_org`/`bss_org` are raised in the profile.

In this layout W0 is mapped as RAM. Tests run with `BC_ERS_NONE`; interrupts are disabled and
vectors are replaced with stubs.

### Writing a test

A test is a normal C program with `main()`:

```c
#include <baseconf.h>
#include <baseconf/graphics.h>
#include "bc_test.h"

static const bc_surface_t surfaces[] = {
    {BC_VIDEO_ZX_256, 5, 5},
    /* ... */
};

int main(void) {
    if (bc_test_init() != BC_OK) {
        bc_test_finish();
        return 0;
    }

    BC_ASSERT(bc_select_screen(BC_SCREEN_0) == BC_OK);
    if (bc_set_video_mode(BC_VIDEO_ATM_640) == BC_OK) {
        bc_set_ink(7);
        clg();
        bc_draw_line(0, 0, 639, 199);
        bc_test_step_shot("atm640");
    } else {
        bc_test_step_skip("atm640", "mode unavailable");
    }

    bc_test_step_data(NULL, surfaces, sizeof(surfaces));
    bc_test_finish();
    return 0;
}
```

`bc_test_step_shot/data/binary/text/skip` complete the current step and wait for the host;
`bc_test_finish` ends the test. `name` is optional: `NULL` or an empty string uses the tag `out`.

| Function                                         | Purpose                                   |
| ------------------------------------------------ | ----------------------------------------- |
| `bc_test_init()`                                 | `bc_init`, `DI`, border 0, ink 7, paper 0 |
| `bc_test_step_shot(name)`                        | step with a screenshot                    |
| `bc_test_output_*()` + `bc_test_step_text(name)` | step with text                            |
| `bc_test_step_data(name, ptr, len)`              | step with data as a hex dump              |
| `bc_test_step_binary(name, ptr, len)`            | step with raw bytes                       |
| `BC_ASSERT(cond)`                                | check; a failure marks the step FAIL      |
| `bc_test_step_skip(name, reason)`                | SKIP step                                 |
| `bc_test_finish()`                               | final verdict; the CPU stays in a loop    |

Video tests select screen 0 once for reproducible screenshots. Each call to `bc_set_video_mode()`
prepares the screen's draw surface in the available CPU windows.

### Result channels

| Channel   | API                                            | Expected file    |
| --------- | ---------------------------------------------- | ---------------- |
| screen    | `bc_test_step_shot(name)`                      | `.png`           |
| text      | `bc_test_output_*` + `bc_test_step_text(name)` | `.txt`           |
| data      | `bc_test_step_data(name, ptr, len)`            | `.txt`, hex dump |
| raw bytes | `bc_test_step_binary(name, ptr, len)`          | `.bin`           |
| logic     | `BC_ASSERT(cond)`                              | no file          |

Non-graphics checks use one of three channels:

```c
/* table or structure */
static const bc_surface_t table[] = { /* ... */ };
bc_test_step_data(NULL, table, sizeof(table));

/* text */
bc_test_output_begin();
bc_test_output("paging=");
bc_test_output_hex8(bc_readback_register(0x0a));
bc_test_output(" border=");
bc_test_output_u16(0);
bc_test_output("\n");
bc_test_step_text(NULL);

/* large buffer */
bc_test_step_binary(NULL, buffer, sizeof(buffer));
```

A hex expected file looks like this:

```
0000: 00 05 05 01 05 05 02 04 05 03 01 05 04 01 05 05
0010: 01 05 06 08 08
```

The step log gets a `DATA <addr> <len> <kind>` line (`0` — hex, `1` — text, `2` — raw bytes). This
is a ZXHost log descriptor; after `DONE`, the host reads `len` bytes through ZRCP from `addr` while
the test waits for the next request, and compares them with the expected file. The buffer must be
mapped at that moment: BSS/data, stack, rodata or the current screen window. Length is `uint16_t`.
The text buffer is 512 bytes and truncates on overflow.

### Case metadata

`cases/<package>/<name>.json` (optional):

```json
{
  "description": "classic printf/gotoxy",
  "profiles": ["no-font8"],
  "timeout": 45,
  "defines": ["SOME_PARAMETER=1"],
  "sources": ["extra.c"]
}
```

- `profiles` — profile layers to merge for this case; omitted means use `default` only.
- `timeout` — seconds per step; default 30.
- `defines` — additional `-D` options.
- `sources` — additional files relative to the case source file.

### Profiles

`default` is always applied first and holds shared settings such as memory layout and the library
build directory. The names in a case's `profiles` array are then merged in order into one build
configuration. z88dk defaults `CRT_ENABLE_STDIO` to `1`, so cases that use CRT stdio need no stdio
layer. Select `no-stdio` only for cases that need to omit it.

| Layer           | Setting                               | Effect                                                  |
| --------------- | ------------------------------------- | ------------------------------------------------------- |
| `no-stdio`      | `CRT_ENABLE_STDIO=0`                  | Disable CRT stdio and the BaseConf generic console hook |
| `no-font8`      | `BASECONF_CONSOLE_NO_DEFAULT_FONT=1`  | Omit the default 8×8 font from the library              |
| `no-primitives` | `BASECONF_VIDEO_PRIMITIVE_MODES=NONE` | Omit graphics primitive backends                        |

For example, `no-stdio` plus `no-primitives` builds a text-only configuration. With only `no-font8`,
the default CRT stdio setting remains enabled. Conflicting assignments are reported as an error.

Profile fields:

| Field                  | Purpose                                             |
| ---------------------- | --------------------------------------------------- |
| `layout`               | addresses, pages, windows; inherited from `default` |
| `library.build_dir`    | library build directory                             |
| `library.make_targets` | `make` targets                                      |
| `library.make_args`    | `make` arguments                                    |
| `library.extra_libs`   | additional `-l<name>` options                       |
| `cflags`               | additional `zcc` flags                              |
| `defines`              | additional `-D` options                             |

`newirq`: `src/newirq` objects are part of `baseconf.lib`; `<baseconf/newirq.h>` is always
available.

### Step protocol

ZXHost header — `0x6000` (32 bytes), log — `0x6100` (1792 bytes).

| Field     | Value                                          |
| --------- | ---------------------------------------------- |
| `result0` | `0` PASS, `1` FAIL, `2` SKIP                   |
| `result1` | step number                                    |
| `result2` | flags: `1` screenshot, `2` next step, `4` data |
| `result3` | 0                                              |

Step log:

```
<step name>
DATA <addr> <len> <kind>
FAIL line=<n> <expression>
```

`FAIL` is appended when `BC_ASSERT` fails.

Sequence:

1. The host writes a request with `sequence=1` before starting the CPU.
2. The test runs a step, writes results, calls `zxhost_finish(DONE)` and waits.
3. The host sees `DONE` with its `sequence`, reads data and the screenshot, and sends `sequence+1`
   (`state=0`, sequence written last).
4. The test sees the new sequence, calls `zxhost_init()` and continues.

The screenshot is taken after `DONE`, when the test has stopped drawing.

### Expected files and names

Expected files live beside the case source and use its filename stem:

```
cases/<package>/<case>.<NN>_<tag>.<ext>
```

- `NN` — step number, starting at 1.
- `tag` — `name` from `bc_test_step_*`, lower-cased; `NULL`/empty uses `out`.
- `ext` — `.png`, `.txt` or `.bin` for the channel.
- Actual files live in the matching `out/<package>/` directory with the same name.
- Build artifacts live in `build/<package>/<case>/`; report and log live in `out/<package>/`.
  Profile selection affects the build and run, not test or artifact names.

Example for `core/smoke.c`:

```
cases/core/smoke.01_zx256.png
cases/core/smoke.02_out.txt
cases/core/smoke.03_atm640.png
cases/core/smoke.04_out.txt
```

- `make ... shots` (`run.py check --update`) writes expected files on PASS.
- On PASS, `--update` removes expected files of this case/profile that no longer have a step.
- `make ... test` (`run.py check`) compares:
  - PNG: differing pixel count, first difference coordinate, `out/<package>/<name>.diff.png`;
  - text: unified diff in `out/<package>/<stem>.diff.txt`;
  - binary: first differing offset and a hex window.
- An expected file without a produced output is an error; remove the file manually or through
  `--update` on PASS.

### CLI

```sh
python3 lib/baseconf/tests/run.py list [--case PATH]
python3 lib/baseconf/tests/run.py build [--case PATH] [--profile NAME]
python3 lib/baseconf/tests/run.py check [--case PATH] [--filter REGEX]
                                       [--update] [--no-build] [--fail-fast]
                                       [--timeout SEC] [--port N] [--verbose]
python3 lib/baseconf/tests/run.py clean [--case PATH]
```

- `check` is the default command.
- `--update` writes expected files next to the case and removes stale ones (PASS only).
- `--no-build` reuses the matching `build/<package>/<case>/manifest.json`.
- `--fail-fast` stops after the first failing case.
- `--case` takes a path relative to `cases/`. Use `core/helpers` for one case or `core` for all
  cases under that directory. A unique file stem such as `helpers` also works. Repeat `--case` to
  combine selections.
- `--profile` is repeatable and selects cases that use a named layer with their full profile.
- `clean` removes generated files in `build/` and `out/`. With `--case`, it removes only the
  selected cases' generated files.

Exit codes: `0` — PASS, `1` — FAIL or build error, `2` — emulator/ZRCP error.

### Determinism

- machine `BaseConf`, `--realvideo`, `--vo null`;
- 2,000,000 boot cycles, with keyboard input opt-in per test;
- `DI`, border 0; scenes without FLASH/ULAPlus/RTC/PS2/UART;
- screenshot 200,000 cycles after `DONE`;
- full-frame comparison at 720×576 by default;
- ZEsarUX version in `out/<package>/<case>.zesarux.log`; CI pins the version.

### Limitations

- The default layout maps RAM in W0. The `ers-rom` profile keeps ROM in W0.
- The harness starts with maskable interrupts disabled. Interrupt cases install their own handlers.
- Code and rodata up to 24,320 bytes; large scenes are split into cases.
- One emulator per run; parallel runs on the same port are not supported.
- Flash programming, paged AVR EEPROM and `newirq` sources require hardware support beyond this
  emulator. The RTC clock case checks BCD and the PS/2 case checks make and break codes.

Cases can pass extra ZEsarUX command-line options as an `emulator_args` array in their JSON
metadata. Interactive cases can use `emulator_steps`. Each key names a target marker. After resuming
that marker, the runner can generate `nmi`, run `settle` instructions, send `before_shot_keys`,
capture a `shot`, then send `keys`. Key scripts accept names and chords such as `f;break;0`. An
optional `region` of `[x, y, width, height]` limits comparison to stable pixels such as a menu,
while the saved screenshot retains the full frame. An action with `uart.send` or `uart.expect`
exchanges byte arrays over a raw POSIX pseudoterminal while the target waits at that marker. The
UART case requires a ZEsarUX build with BaseConf UART support and the `--uartbridge-file` option.

### Coverage plan

| Case                  | Checks                                                             | Modes                          | Channel                                   |
| --------------------- | ------------------------------------------------------------------ | ------------------------------ | ----------------------------------------- |
| `core_init`           | init/readback/status/setters                                       | —                              | check + text                              |
| `memory_windows`      | map, readback, protect, leases                                     | —                              | check + data                              |
| `paging_decode`       | `BC_PAGING_*`, `BC_CONFIG_*`                                       | —                              | data                                      |
| `video_modes`         | `screen_surface`, `set/get_video_mode`, `show_screen`              | all 7                          | shot + data                               |
| `palette_border`      | ATM 16 colors, RGB444, border                                      | ATM320/640                     | shot                                      |
| `graphics_line`       | line/hline/vline/drawto                                            | 5 graphics modes               | shot                                      |
| `graphics_circle`     | circle/uncircle/xorcircle                                          | 5                              | shot                                      |
| `graphics_fill`       | clg/clga/fillb/scroll                                              | 5                              | shot                                      |
| `bc_printf`           | formatting, font8, wrapping, no scroll                             | 5 graphics modes, ATM/Evo text | shot + counts (`no-stdio`)                |
| `crt_printf`          | CRT printf/conio, font5, wrapping and scroll                       | ZX, ATM320/640, ATM/Evo text   | shot + counts (`no-font8`; stdio default) |
| `graphics_primitives` | `cplot/cpoint`, line, hline, vline, circle, ellipse; SET/CLEAR/XOR | 5 bitmap modes                 | checks + shot (`no-stdio` + `no-font8`)   |
| `text80`              | `text_putc/puts/draw`                                              | ATM/Evo text                   | shot + text                               |
| `font_upload`         | `upload_font_to`, BF.2, destination constraints                    | 2                              | shot + data                               |
| `input_keyboard`      | matrix, `key_poll`                                                 | —                              | data/text                                 |
| `ps2`                 | modifiers/log                                                      | —                              | data                                      |
| `rtc`                 | BCD/time/NVRAM                                                     | —                              | data                                      |
| `storage`             | SD/IDE/FDD status                                                  | —                              | data                                      |
| `uart_contract`       | UART configuration, status, RX FIFO reset, host RX/TX              | —                              | checks + host PTY                         |
| `interrupt`           | DI/EI, trap, NMI                                                   | —                              | check                                     |
| `ers`                 | status/stubs                                                       | —                              | data                                      |
| `newirq`              | new IRQ API                                                        | —                              | check                                     |
| `helpers`             | ZX attribute and paging bitfields                                  | —                              | check                                     |
| `palette`             | Pentagon pixel pack/unpack for all 256 byte values                 | —                              | check (`no-stdio`)                        |

### CI

```sh
make -C lib/baseconf
make -C lib/baseconf/tests test
```

CI pins the ZEsarUX version, caches `lib/baseconf/build*` and `tests/build`, and publishes
`tests/out/**` on failure. `shots` is run manually or in a separate PR with a diff review.

## Console TAP tests

`make -C lib/baseconf/tests run TEST_MODE=0 TEST_FONT=5 TEST_SCROLL=1` builds and runs the ZX256
`bc_printf` TAP test in ZEsarUX. `make -C lib/baseconf/tests run-crt TEST_MODE=0` checks ordinary
CRT `printf` through the generic console. The TAP reports and screenshots are written under
`tests/build/`.
