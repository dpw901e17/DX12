@echo off 
cls
cd %~dp0

set /p seconds="time in seconds: "
set /p cubeDim="Cube Dimension (N x N x N): "
set /p threadCount="Draw thread count: "
set "exename=OrganizedDirectX12"
set "drawArg=-cubeDim %cubeDim% -cubePad 1 -threadCount %threadCount%"

REM Pipeloins statistikler
%exename%.exe -csv -sec 3 -pipelineStatistics %drawArg%

REM OpenHardwareMonitor
start C:\PROGRA~2\OpenHardwareMonitor\OpenHardwareMonitor.exe 
timeout 10
%exename%.exe -csv -sec %seconds% -OHM -pi 1000 %drawArg%
taskkill /IM OpenHardwareMonitor.exe

REM Perfmonitor
logman delete dxd12basetutorialPerfData
logman create counter dxd12basetutorialPerfData -cf CollectWinPerfmonDataCfg.cfg -f csv -o "%cd%\perfdata" -si 1
logman start dxd12basetutorialPerfData -as
%exename%.exe -sec %seconds% %drawArg%
logman stop dxd12basetutorialPerfData 

REM Move files to data directory
FOR /F "delims=_. tokens=2" %%i IN ('dir /B data_*.csv') DO call :mover %%i
GOTO :EOF

:mover
 mkdir data\%1
 move perfdata*.csv data\%1\perfmon_%1.csv 
 move data_*.csv data\%1\ohmon_%1.csv
 move stat_*.csv data\%1\pipeline_%1.csv
 move conf_*.csv data\%1\
 GOTO :EOF
