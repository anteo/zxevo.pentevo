#!/bin/bash

#make sure it runs from correct place -- 'cd' to the dir where this script is situated
cd "$(dirname "$0")" || exit 1

# some variables
FPGA_PATH=${FPGA_PATH:-../../../fpga/base_trdemu/trunk/quartus}
FPGA_FILE=top.rbf

AVR_PATH=${AVR_PATH:-../../../avr/baseconf/trunk/build}
AVR_FILE=top.mlz

RESULT=zxevo_fw.bin

if [ "$(uname -s)" = Darwin ]; then
	if [ -z "${MHMT_BIN:-}" ]; then
		make -C ../../../tools/mhmt/linux || exit 1
		MHMT_BIN=../../../tools/mhmt/linux/mhmt
	fi
	if [ -z "${MAKEFW:-}" ]; then
		make -C ../../../tools/make_fw/linux || exit 1
		export MAKEFW="$(pwd -P)/../../../tools/make_fw/linux/make_fw"
	fi
else
	MHMT_BIN=${MHMT_BIN:-../../../tools/mhmt/mhmt}
fi


# build FPGA
cd "$FPGA_PATH" && make || exit 1
cd - || exit 1

# check whether top.rbf exists
if [ ! -f "$FPGA_PATH/$FPGA_FILE" ]; then
	echo Error: $FPGA_FILE was not built!
	exit 1
fi

# pack and copy fpga file to avr dir
"$MHMT_BIN" -maxwin2048 "$FPGA_PATH/$FPGA_FILE" "$AVR_PATH/$AVR_FILE" || { echo 'mhmt failed!' ; exit 1; }

# go build avr firmware
cd "$AVR_PATH" && make || exit 1
cd - || exit 1

# check whether there is end file
if [ ! -f "$AVR_PATH/$RESULT" ]; then
	echo Error: $RESULT was not build!
	exit 1
fi

cp "$AVR_PATH/$RESULT" ./ || exit 1

echo Done!

