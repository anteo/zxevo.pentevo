
echo off

if not exist tmp md tmp
call ..\..\setpath.bat

cd %rootdir%\fat_boot\source

call build.bat

cd %rootdir%\page5\source

mhmt -mlz 8x8_ar.fnt 8x8_ar_pack.bin
mhmt -mlz 866_code.fnt 866_code_pack.bin
mhmt -mlz atm_code.fnt atm_code_pack.bin
mhmt -mlz perfpack.bin perfpack_pack.bin

rem asl -U -L -x -olist tmp\rst8service.lst -i %includes% rst8service.a80 || goto error
rem p2bin rst8service.p ..\rst8service.rom -r $-$ -k

asl -U -L -x -olist tmp\rst8service_fe.lst -i %includes% -D DOS_FE rst8service.a80 || goto error
p2bin rst8service.p ..\rst8service_fe.rom -r $-$ -k

echo ########################
echo # -= End Compile Ok =- #
echo ########################
goto exit

:error
echo #######################
echo # -= Error Compile =- #
echo #######################

:exit
