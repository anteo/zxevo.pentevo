
echo off

if not exist tmp md tmp
call ..\..\setpath.bat

asl -U -L -C -x -olist tmp\dos_fe.lst -i %includes% dos_fe.a80 || goto error
p2bin dos_fe.p tmp\dos_fe.rom -r $-$ -k

mhmt -mlz tmp\dos_fe.rom ..\dos_fe_pack.rom

echo ########################
echo # -= End Compile Ok =- #
echo ########################
goto exit

:error
echo #######################
echo # -= Error Compile =- #
echo #######################

:exit