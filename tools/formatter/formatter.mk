# Shared formatting targets for ZX Evolution projects.
#
# The project lists the sources it wants formatted and includes this fragment
# at the end of its Makefile:
#
#   FORMAT_FILES   := $(wildcard src/*.c src/*.h src/*.asm)
#   FORMATTER_ARGS := --dialect=z80asm    # optional, forwarded by extension
#   include ../tools/formatter/formatter.mk
#
# One list, one call: the dispatcher routes every file by its extension and
# gives each formatter the options that formatter declares, so this fragment
# never looks at extensions either.
#
# Targets provided here: format, format-check, format-plan.

# Resolve the dispatcher relative to this included file, not to the caller.
# It finds the built-in formatters next to itself, so no mapping is needed.
FORMATTER_MK := $(abspath $(lastword $(MAKEFILE_LIST)))
FORMATTER_DIR := $(patsubst %/,%,$(dir $(FORMATTER_MK)))
FORMATTER ?= $(FORMATTER_DIR)/formatter

# Dispatcher arguments, empty by default. They reach only the formatters that
# declare them, for example --dialect=z80asm or --clang-format=/opt/clang.
FORMATTER_ARGS ?=

FORMAT_CMD = $(FORMATTER) $(FORMATTER_ARGS)

# The first target of an included file silently becomes the default goal, so
# remember the caller's goal here and restore it at the end of this file.
FORMATTER_DEFAULT_GOAL := $(.DEFAULT_GOAL)

.PHONY: format format-check format-plan

# One list, one call. The dispatcher fails loudly on an empty list.
format:
	$(FORMAT_CMD) $(FORMAT_FILES)

# Report-only variant: writes nothing and exits non-zero when a file needs
# formatting, which is what CI wants.
format-check:
	$(FORMAT_CMD) --check $(FORMAT_FILES)

format-plan:
	@printf 'formatter: %s\n' '$(FORMATTER)'
	@printf '  %s\n' $(FORMAT_FILES)

# Restore the default goal so `make` without arguments still does what the
# project expects; the fragment may be included anywhere in the Makefile.
ifneq ($(FORMATTER_DEFAULT_GOAL),)
.DEFAULT_GOAL := $(FORMATTER_DEFAULT_GOAL)
endif
