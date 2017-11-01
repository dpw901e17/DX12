@echo off
cls
mkdir ..\x64\Debug
mkdir ..\x64\Release

echo f|xcopy record.bat ..\x64\Debug
echo f|xcopy record.bat ..\x64\Release

echo f|xcopy VertexShader.hlsl ..\x64\Debug
echo f|xcopy VertexShader.hlsl ..\x64\Release

echo f|xcopy PixelShader.hlsl ..\x64\Debug
echo f|xcopy PixelShader.hlsl ..\x64\Release

echo f|xcopy CollectWinPerfmonDataCfg.cfg ..\x64\Debug
echo f|xcopy CollectWinPerfmonDataCfg.cfg ..\x64\Release