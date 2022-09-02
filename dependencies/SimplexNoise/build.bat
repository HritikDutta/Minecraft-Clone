@echo off

if not exist lib md lib

cl /EHsc /O2 /c src\*.c* /I src
lib *.obj /out:lib\SimplexNoise.lib

del *.obj