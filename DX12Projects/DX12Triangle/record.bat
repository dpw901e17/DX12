@echo off 
cls
cd %~dp0

set /p seconds="time in seconds: "

REM OpenHardwareMonitor
start C:\PROGRA~2\OpenHardwareMonitor\OpenHardwareMonitor.exe 
timeout 10
DX12Triangle.exe -csv -sec %seconds%
taskkill /IM OpenHardwareMonitor.exe

REM Perfmonitor
logman delete dxd12basetutorialPerfData
logman create counter dxd12basetutorialPerfData -cf CollectWinPerfmonDataCfg.cfg -f csv -o %cd%\perfdata -si 1
logman start dxd12basetutorialPerfData -as
DX12Triangle.exe -perfmon -sec %seconds%
logman stop dxd12basetutorialPerfData 

REM Move files to data directory
FOR /F "delims=_. tokens=2" %%i IN ('dir /B data_*.csv') DO call :mover %%i
GOTO :EOF

:mover
 mkdir data\%1
 move perfdata*.csv data\%1\perfmon_%1.csv 
 move data_*.csv data\%1\ohmon_%1.csv
 GOTO :EOF
