@echo off
cls
mkdir ..\x64\Debug
mkdir ..\x64\Release

copy /Y *.bat ..\x64\Debug\
copy /Y *.bat ..\x64\Release\


copy /Y *.hlsl ..\x64\Debug\
copy /Y *.hlsl ..\x64\Release\

copy /Y *.jpg ..\x64\Debug\
copy /Y *.jpg ..\x64\Release\
copy /Y *.png ..\x64\Debug\
copy /Y *.png ..\x64\Release\
copy /Y *.bmp ..\x64\Debug\
copy /Y *.bmp ..\x64\Release\

copy /Y CollectWinPerfmonDataCfg.cfg ..\x64\Debug\
copy /Y CollectWinPerfmonDataCfg.cfg ..\x64\Release\
