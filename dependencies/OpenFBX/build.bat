@echo off

if not exist lib md lib

cl /O2 /EHsc /std:c++17 /cgthreads8 /MP7 /GL /c src\*.c* /I src /I ..\..\src
lib *.obj /out:lib\OpenFBX.lib

del *.obj