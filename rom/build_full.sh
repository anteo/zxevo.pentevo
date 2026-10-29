#!/bin/bash

set -ex

#if ASL and MHMT paths are already set, convert them to absolute right now
if [ -n "${ASL_PATH:-}" ]; then
	ASL_PATH="$(cd "$ASL_PATH" && pwd -P)"
fi

if [ -n "${MHMT_PATH:-}" ]; then
	MHMT_PATH="$(cd "$MHMT_PATH" && pwd -P)"
fi


# cd to the dir where the script is situated
cd "$(dirname "$0")"


# set ASL and MHMT paths if not already set, also converting them to absolute
if [ -z "${ASL_PATH:-}" ]; then
	if [ "$(uname -s)" = Darwin ]; then
		make -C ../tools/asl
		ASL_PATH="../tools/asl/build/"
	else
		ASL_PATH="../tools/asl/bin/"
	fi
	ASL_PATH="$(cd "$ASL_PATH" && pwd -P)"
fi

if [ -z "${MHMT_PATH:-}" ]; then
	if [ "$(uname -s)" = Darwin ]; then
		make -C ../tools/mhmt/linux
		MHMT_PATH="../tools/mhmt/linux/"
	else
		MHMT_PATH="../tools/mhmt/"
	fi
	MHMT_PATH="$(cd "$MHMT_PATH" && pwd -P)"
fi

# Component scripts run two directories below rom; ASL needs this include path.
export ASCMD="${ASCMD:+$ASCMD }-i ../.."


# BUILD FAT BOOT
pushd fat_boot/source
source build.sh
popd

# BUILD MAIN MENU
pushd mainmenu/src
source build.sh
popd

# BUILD DOS_FE
pushd page1/dos_fe
source build.sh
popd

# BUILD START PAGE
pushd page0/source
source build.sh
popd

# BUILD NEO-DOS
pushd page1/neo-dos
source build.sh
popd

# BUILD BASIC 128
pushd page2/source
source build.sh
popd

# BUILD BASIC 48
pushd page3/source
source build.sh
popd

# BUILD ATM CP/M
pushd atm_cpm/source
source build.sh
popd

# BUILD RST 8 SERVICES
pushd page5/source
source build.sh
popd

# BUILD TR-DOS 6.10 
pushd trdos_v6/source
source build.sh
popd


# BUILD ERS
cat ff_16k.rom ff_16k.rom page5/rst8service_fe.rom ff_16k.rom page3/basic48_128.rom page1/neo-dos.rom page2/basic128.rom page0/services_fe.rom > ers_fe.rom

# BUILD PENT GLUK 
cat page3/2006.rom trdos_v6/dosatm3.rom page2/basic128.rom page0/glukpen.rom > glukpent.rom

# BUILD ATM CP/M 
cat atm_cpm/rbios.rom page3/basic48_128_std.rom page2/128_std.rom page3/basic48_orig.rom > basics_std.rom

# BUILD FULL ERS ROM EMUL FDD FE
#rem    64          64            64                128               192
ROM_TEMP="$(mktemp ./zxevo_fe.rom.XXXXXX)"
trap 'rm -f "$ROM_TEMP"' EXIT
cat ff_64k.rom basics_std.rom glukpent.rom profrom/evoprofrom.rom ers_fe.rom > "$ROM_TEMP"
ROM_SIZE="$(wc -c < "$ROM_TEMP" | tr -d '[:space:]')"
if [ "$ROM_SIZE" -ne 524288 ]; then
	echo "Unexpected ROM size: $ROM_SIZE bytes (expected 524288)" >&2
	exit 1
fi
chmod 644 "$ROM_TEMP"
# rm ers.rom
rm ers_fe.rom
rm glukpent.rom
rm basics_std.rom
mv -f "$ROM_TEMP" zxevo_fe.rom

#read -rsn1 -p "Press any key to continue . . .";
