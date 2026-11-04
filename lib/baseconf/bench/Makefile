.DEFAULT_GOAL := bench-font-draw
ROOT := ../../..
include ../baseconf.mk

PYTHON ?= python3
BUILD := build
BENCH_MODE ?= 0
BENCH_APP := $(BUILD)/font-draw-bench-$(BENCH_MODE)
CODE_ORG ?= 0x8000
STACK_ORG ?= 0xFF00
DATA_ORG ?= 0xD800
BSS_ORG ?= 0xD900
COMMON = +$(BASECONF_CFG) $(BASECONF_ABI_FLAGS) -SO3 -m \
	-L$(BASECONF_BUILD) -Ca-I$(CURDIR) \
	-pragma-define:CRT_MODEL=0 -pragma-define:MMAP=-1 \
	-pragma-define:CRT_ORG_CODE=$(CODE_ORG) -pragma-define:CRT_ORG_DATA=$(DATA_ORG) \
	-pragma-define:CRT_ORG_BSS=$(BSS_ORG) \
	-pragma-define:REGISTER_SP=$(STACK_ORG)
INCLUDES := -I$(BASECONF_INCLUDE)

.PHONY: all bench-font-draw clean
all: bench-font-draw

ensure-baseconf:
	$(MAKE) -C .. ZCC="$(ZCC)" Z80ASM="$(Z80ASM)" BASECONF_CONSOLE_NO_DEFAULT_FONT=$(BASECONF_CONSOLE_NO_DEFAULT_FONT)

$(BASECONF_LIB): | ensure-baseconf

$(BENCH_APP).tap: Makefile font_draw_bench.c mmap.inc $(BASECONF_LIBRARY_INPUTS) $(BASECONF_LIB) $(BASECONF_APPMAKE_BIN)
	mkdir -p $(BUILD)
	PATH=$(dir $(BASECONF_APPMAKE_BIN)):$$PATH $(ZCC) $(COMMON) $(INCLUDES) -DBENCH_MODE=$(BENCH_MODE) -create-app -Cz+baseconf -Cz--name=BCFONT $(APPMAKE_OPTIONS) -o $(BENCH_APP) font_draw_bench.c

ifeq ($(BENCH_MODE),4)
$(BENCH_APP).tap: CODE_ORG=0x8000
$(BENCH_APP).tap: STACK_ORG=0x8000
$(BENCH_APP).tap: DATA_ORG=0x5F40
$(BENCH_APP).tap: BSS_ORG=0x5F50
$(BENCH_APP).tap: APPMAKE_OPTIONS=-Cz--clearaddr=0x5E00
$(BENCH_APP).tap: BASECONF_CONSOLE_NO_DEFAULT_FONT=1
else
$(BENCH_APP).tap: CODE_ORG=0x8000
$(BENCH_APP).tap: STACK_ORG=0xFF00
$(BENCH_APP).tap: DATA_ORG=0xD800
$(BENCH_APP).tap: BSS_ORG=0xD900
$(BENCH_APP).tap: APPMAKE_OPTIONS=
$(BENCH_APP).tap: BASECONF_CONSOLE_NO_DEFAULT_FONT=0
endif

bench-font-draw: $(BENCH_APP).tap
	$(PYTHON) font_draw_bench.py --tap $(BENCH_APP).tap --map $(BENCH_APP).map --mode $(BENCH_MODE) --output $(BENCH_APP)-results.json

clean:
	rm -rf $(BUILD)

.NOTPARALLEL:
