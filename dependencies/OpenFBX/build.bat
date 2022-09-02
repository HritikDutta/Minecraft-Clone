@echo off

if not exist lib md lib

cl /EHsc /O2 /c src\*.c* /I src /I ..\..\src
lib *.obj /out:lib\OpenFBX.lib

del *.obj