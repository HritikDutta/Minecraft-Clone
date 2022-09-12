@echo off

if not exist lib md lib

set includes= /I src ^
              /I dependencies\glad\include ^
              /I dependencies\wglext\include ^
              /I dependencies\stb\include ^
              /I dependencies\OpenFBX\src ^
              /I dependencies\SimplexNoise\src

set libs= Shell32.lib                     ^
          msvcrt.lib                      ^
          dependencies\glad\lib\glad.lib  ^
          dependencies\stb\lib\stb.lib    ^
          dependencies\OpenFBX\lib\OpenFBX.lib ^
          dependencies\SimplexNoise\lib\SimplexNoise.lib

set compile_flags= /O2 /EHsc /std:c++17 /cgthreads8 /MP7 /GL

if "%1"=="release" (
    set defines= /DGN_USE_OPENGL /DGN_PLATFORM_WINDOWS /DGN_USE_DEDICATED_GPU /DGN_RELEASE /DNDEBUG
) else (
    set defines= /DGN_USE_OPENGL /DGN_PLATFORM_WINDOWS /DGN_USE_DEDICATED_GPU /DGN_DEBUG
)

rem Remove existing files
del *.exe *.pdb

rem Source
cl /c %compile_flags% src/containers/*.cpp %defines% %includes% & ^
cl /c %compile_flags% src/fileio/*.cpp %defines% %includes% & ^
cl /c %compile_flags% src/serialization/json/*.cpp %defines% %includes% & ^
cl /c %compile_flags% src/math/constants.cpp %defines% %includes% & ^
cl /c %compile_flags% src/core/application_internal.cpp %defines% %includes% & ^
cl /c %compile_flags% src/core/input_processing.cpp %defines% %includes% & ^
cl /c %compile_flags% src/platform/*.cpp %defines% %includes% & ^
cl /c %compile_flags% src/engine/*.cpp %defines% %includes% & ^
cl /c %compile_flags% src/graphics/*.cpp %defines% %includes%

rem Link and Make Library
lib *.obj %libs% /OUT:engine.lib

rem Delete Intermediate Files
del *.obj *.exp