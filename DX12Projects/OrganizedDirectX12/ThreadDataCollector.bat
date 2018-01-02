@echo off
cd %~dp0

SET seconds=30
SET cubeDim=30
SET testCount=15
SET exename=OrganizedDirectX12
SET /P "output=Output Folder (in data): "
SET "drawArg=-cubeDim %cubeDim% -cubePad 1 -csv -fps -sec %seconds%"

DEL *.csv

FOR /L %%i IN (1,1,%testCount%) DO CALL :run %%i
ECHO Tests Complete!
PAUSE
EXIT

:run
START "Test #%1" /WAIT %exename%.exe %drawArg% -threadCount %1
FOR /F "delims=_. tokens=2" %%i IN ('dir /B fps_*.csv') DO CALL :mover %%i %output% %1
ECHO Test #%1 done!

GOTO :EOF


:mover
IF NOT DEFINED dir set "dir=data\%2\%1"
 mkdir %dir%\%3-threads
 move fps_*.csv %dir%\%3-threads\
 move conf_*.csv %dir%\%3-threads\
 GOTO :EOF