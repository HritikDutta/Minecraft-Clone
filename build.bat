@echo off

set includes= /I src ^
              /I dependencies\glad\include ^
              /I dependencies\wglext\include ^
              /I dependencies\stb\include ^
              /I dependencies\OpenFBX\src ^
              /I dependencies\SimplexNoise\src

set libs= Shell32.lib                     ^
          User32.lib                      ^
          Gdi32.lib                       ^
          OpenGL32.lib                    ^
          msvcrt.lib                      ^
          Comdlg32.lib                    ^
          lib/engine.lib                  ^
          dependencies\glad\lib\glad.lib  ^
          dependencies\stb\lib\stb.lib    ^
          dependencies\OpenFBX\lib\OpenFBX.lib ^
          dependencies\SimplexNoise\lib\SimplexNoise.lib

if "%1"=="release" (
    set defines= /DGN_USE_OPENGL /DGN_PLATFORM_WINDOWS /DGN_USE_DEDICATED_GPU /DGN_RELEASE /DNDEBUG /DGN_COMPILER_MSVC
    set compile_flags= /O2 /EHsc /std:c++17 /cgthreads8 /MP7 /GL
    set link_flags= /NODEFAULTLIB:LIBCMT /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup /LTCG
) else (
    set defines= /DGN_USE_OPENGL /DGN_PLATFORM_WINDOWS /DGN_USE_DEDICATED_GPU /DGN_DEBUG /DGN_COMPILER_MSVC
    set compile_flags= /Zi /EHsc /std:c++17 /cgthreads8 /MP7 /GL
    set link_flags= /DEBUG /NODEFAULTLIB:LIBCMT /LTCG
)

rem Source
cl /c %compile_flags% src/game/*.cpp %defines% %includes% & ^
cl /c %compile_flags% src/main.cpp %defines% %includes% & ^
cl /c %compile_flags% src/core/entry.cpp %defines% %includes%

rem Link and Make Executable
link *.obj %libs% /OUT:main.exe %link_flags%

rem Delete Intermediate Files
del *.obj *.exp *.lib