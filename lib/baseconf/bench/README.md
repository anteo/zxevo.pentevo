# Font draw benchmarks

Run `make -C lib/baseconf/bench` for ZX256 or `make -C lib/baseconf/bench BENCH_MODE=4` for ATM640.
The harness measures ZEsarUX's partial Z80 T-state counter between start/end breakpoints.

The ZX256 run draws 64 opaque `p` glyphs through each font descriptor callback. The 5×7 positions
cycle through x=0, 6, 12, and 18, covering all four byte alignments. Both C and Z80 measurements use
the same callback workload. The current result is saved to
`bench/build/font-draw-bench-0-results.json`.

| Renderer         | T-states / 64 glyphs | T-states / glyph |
| ---------------- | -------------------: | ---------------: |
| font5 C baseline |            1,338,989 |         20,921.7 |
| font5 Z80 kernel |              357,309 |          5,583.0 |
| font8 C baseline |              734,445 |         11,475.7 |
| font8 Z80 kernel |              618,797 |          9,668.7 |

The ZX256 font5 path is about 3.75× faster and font8 about 1.19× faster with the Z80 kernels.

The ATM640 workload draws the same 64 glyphs through the C renderer and measured 2,304,066 T-states
(36,001 per glyph). The ATM640 path remains C; the ZX row kernels are not used there. Its result is
saved to `bench/build/font-draw-bench-4-results.json`.
