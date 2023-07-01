@setlocal
@echo off

set _CMAKE_BUILD_TYPE=Debug
set _BUILD_ARCH=x64
set _CMAKE_GENERATOR_PLATFORM=x64
set _NDK_DIR=
set _OS_DIR=

:ArgLoop
if [%1] == [] goto LocateVS
if /i [%1] == [release] (set _CMAKE_BUILD_TYPE=Release&& shift & goto ArgLoop)
if /i [%1] == [debug] (set _CMAKE_BUILD_TYPE=Debug&& shift & goto ArgLoop)
shift
goto ArgLoop

:LocateVS

set _CMAKE_ARGS=-DCMAKE_BUILD_TYPE=%_CMAKE_BUILD_TYPE%
set _CMAKE_ARGS=%_CMAKE_ARGS% -DCMAKE_GENERATOR_PLATFORM=%_CMAKE_GENERATOR_PLATFORM%
set _OS_DIR=win


set _BUILD_DIR=.\build\%_CMAKE_BUILD_TYPE%\%_OS_DIR%-%_BUILD_ARCH%


set _CMAKE_ARGS=%_CMAKE_ARGS% ..\..\..

If NOT exist "%BUILD_DIR%" (
  mkdir %_BUILD_DIR%
)
pushd %_BUILD_DIR%
cmake %_CMAKE_ARGS%
cmake --build . --config %_CMAKE_BUILD_TYPE% --target libXexUnpack

if "%_OS_DIR%" == "win" (
copy .\%_CMAKE_BUILD_TYPE%\* .\
)

popd

:Success
exit /b 0

:NoVisualStudio
echo Unable to locate Visual Studio installation. Terminating.
exit /b 1
