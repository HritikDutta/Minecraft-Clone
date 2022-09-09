@echo off

if not exist lib md lib

cl /O2 /EHsc /std:c++17 /cgthreads8 /MP7 /GL /c src\glad.c /I include /I ..\..\src
lib *.obj /out:lib\glad.lib

del *.obj