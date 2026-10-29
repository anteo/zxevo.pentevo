#!/bin/bash

if [ -z "${ASL_PATH:-}" ]; then
	ASL_PATH="../../../tools/asl/bin/"
fi

if [ -z "${MHMT_PATH:-}" ]; then
	MHMT_PATH="../../../tools/mhmt/"
fi

"$ASL_PATH/asl" -U -L -x -D DRV_VAR=0x4000 micro_boot_fat.a80
"$ASL_PATH/p2bin" micro_boot_fat.p ../micro_boot_fat_ers.rom -r '$-$' -k

"$ASL_PATH/asl" -U -L -x -D DRV_VAR=0x4000,ALL_DRVS micro_boot_fat.a80
"$ASL_PATH/p2bin" micro_boot_fat.p ../micro_boot_fat.rom -r '$-$' -k

"$MHMT_PATH/mhmt" -mlz ../micro_boot_fat_ers.rom ../micro_boot_fat_ers.pack
"$MHMT_PATH/mhmt" -mlz ../micro_boot_fat.rom ../micro_boot_fat.pack

