@echo off
cls
mkdir ..\x64\Debug
mkdir ..\x64\Release

copy /Y record.bat ..\x64\Debug\
copy /Y record.bat ..\x64\Release\

copy /Y VertexShader.hlsl ..\x64\Debug\
copy /Y VertexShader.hlsl ..\x64\Release\

copy /Y PixelShader.hlsl ..\x64\Debug\
copy /Y PixelShader.hlsl ..\x64\Release\

copy /Y CollectWinPerfmonDataCfg.cfg ..\x64\Debug\
copy /Y CollectWinPerfmonDataCfg.cfg ..\x64\Release\