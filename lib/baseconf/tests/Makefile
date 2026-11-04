.DEFAULT_GOAL := build
ROOT := ../../..
include ../baseconf.mk
PYTHON ?= python3
INSTRUCTIONS ?= 50000000
BUILD := build
TEST_MODE ?= 4
TEST_FONT ?= 5
TEST_SCROLL ?= 0
APP := $(BUILD)/console-printf-$(TEST_MODE)-$(TEST_FONT)$(if $(filter 1,$(TEST_SCROLL)),-scroll,)
CODE_ORG ?= 0x8000
STACK_ORG ?= 0xFF00
DATA_ORG ?= 0xD800
BSS_ORG ?= 0xD900
ifneq ($(filter $(TEST_MODE),4 5 6),)
CODE_ORG := 0x8000
STACK_ORG := 0x8000
DATA_ORG := 0x5F40
BSS_ORG := 0x5F50
APPMAKE_OPTIONS := -Cz--clearaddr=0x5E00
endif
COMMON = +$(BASECONF_CFG) $(BASECONF_ABI_FLAGS) -SO3 -m \
	-L$(BASECONF_BUILD) -Ca-I$(CURDIR) \
	-pragma-define:CRT_MODEL=0 -pragma-define:MMAP=-1 \
	-pragma-define:CRT_ORG_CODE=$(CODE_ORG) -pragma-define:CRT_ORG_DATA=$(DATA_ORG) \
	-pragma-define:CRT_ORG_BSS=$(BSS_ORG) \
	-pragma-define:REGISTER_SP=$(STACK_ORG)
INCLUDES := -I$(BASECONF_INCLUDE)

.PHONY: all build run run-crt clean
all: build
build: $(APP).tap

ensure-baseconf:
	$(MAKE) -C .. ZCC="$(ZCC)" Z80ASM="$(Z80ASM)" BASECONF_CONSOLE_NO_DEFAULT_FONT=$(BASECONF_CONSOLE_NO_DEFAULT_FONT)

$(BASECONF_LIB): | ensure-baseconf

$(APP).tap: Makefile console_printf.c mmap.inc $(BASECONF_LIBRARY_INPUTS) $(BASECONF_LIB) $(BASECONF_APPMAKE_BIN)
	mkdir -p $(BUILD)
	PATH=$(dir $(BASECONF_APPMAKE_BIN)):$$PATH $(ZCC) $(COMMON) $(INCLUDES) -DCONSOLE_TEST_MODE=$(TEST_MODE) -DCONSOLE_TEST_FONT=$(TEST_FONT) -DCONSOLE_TEST_SCROLL=$(TEST_SCROLL) -create-app -Cz+baseconf -Cz--name=BCONSOLE $(APPMAKE_OPTIONS) -o $(APP) console_printf.c

run: build
	$(PYTHON) $(ROOT)/tools/zesarux/tap_runner.py --tap $(APP).tap --machine BaseConf --tape-at-start --keys 'i;w40;load;quote;quote;enter;w40' --instructions $(INSTRUCTIONS) --shot $(APP).png --log $(APP).log --report $(APP)-report.txt

$(BUILD)/console-crt.tap: Makefile console_crt.c mmap.inc $(BASECONF_LIBRARY_INPUTS) $(BASECONF_LIB) $(BASECONF_APPMAKE_BIN)
	mkdir -p $(BUILD)
	PATH=$(dir $(BASECONF_APPMAKE_BIN)):$$PATH $(ZCC) $(COMMON) $(INCLUDES) -create-app -Cz+baseconf -Cz--name=BCCRT -o $(BUILD)/console-crt console_crt.c

run-crt: $(BUILD)/console-crt.tap
	$(PYTHON) $(ROOT)/tools/zesarux/tap_runner.py --tap $(BUILD)/console-crt.tap --machine BaseConf --tape-at-start --keys 'i;w40;load;quote;quote;enter;w40' --instructions $(INSTRUCTIONS) --shot $(BUILD)/console-crt.png --log $(BUILD)/console-crt.log --report $(BUILD)/console-crt-report.txt

clean:
	rm -rf $(BUILD)

.NOTPARALLEL:

# BaseConf test framework: one .c per case, recursively discovered under
# tests/cases. The TAP targets above remain for the old ad-hoc probes.
.PHONY: test shots framework-list framework-build framework-check framework-update framework-clean
test: framework-check
shots: framework-update

framework-list:
	$(PYTHON) run.py list

framework-build:
	$(PYTHON) run.py build

framework-check:
	$(PYTHON) run.py check

framework-update:
	$(PYTHON) run.py check --update

framework-clean:
	$(PYTHON) run.py clean
