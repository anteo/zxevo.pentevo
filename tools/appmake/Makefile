Z88DK_SRC ?=

.PHONY: all clean
all:
	@test -n "$(Z88DK_SRC)" || { echo "Set Z88DK_SRC to the z88dk src directory" >&2; exit 2; }
	Z88DK_SRC="$(Z88DK_SRC)" ./build.sh

clean:
	rm -rf build
