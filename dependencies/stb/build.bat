@echo off

if not exist lib md lib

cl /O2 /c src\*.cpp /I ..\..\src
lib *.obj /out:lib\stb.lib

del *.obj