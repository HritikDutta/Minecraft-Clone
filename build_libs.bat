@echo off

rem Build Libraries

pushd dependencies

pushd glad
call build.bat
popd

pushd stb
call build.bat
popd

pushd OpenFBX
call build.bat
popd

pushd SimplexNoise
call build.bat
popd

popd