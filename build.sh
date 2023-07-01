#!/usr/bin/env bash

scriptPath="$( cd "$(dirname "$0")" ; pwd -P )"
_CMakeBuildType=Debug
_CMakeToolchain=
_OutputPathPrefix=
_CMakeBuildTarget=libXexUnpack
_CMakeOsxArchitectures=
_CMakeGenerator=
_CMakeExtraBuildArgs=
_OSDir=

while :; do
    if [ $# -le 0 ]; then
        break
    fi

    lowerI="$(echo $1 | awk '{print tolower($0)}')"
    case $lowerI in
        debug|-debug)
            _CMakeBuildType=Debug
            ;;
        release|-release)
            _CMakeBuildType=Release
            ;;
        osx)
            _CMakeOsxArchitectures=$2
            _OSDir=osx
            shift
            ;;
        linux-x64)
            _OSDir=linux-x64
            ;;
        *)
            __UnprocessedBuildArgs="$__UnprocessedBuildArgs $1"
    esac

    shift
done

_OutputPath=$scriptPath/build/$_CMakeBuildType/$_OSDir

mkdir -p $_OutputPath
pushd $_OutputPath
cmake ../../.. -DCMAKE_BUILD_TYPE=$_CMakeBuildType $_CMakeGenerator $_CMakeToolchain $_CMakePlatform -DCMAKE_OSX_ARCHITECTURES="$_CMakeOsxArchitectures"
cmake --build . --target $_CMakeBuildTarget $_CMakeExtraBuildArgs

if [[ $_OSDir == "ios" ]]; then
    cp ./$_CMakeBuildType-*/* ./
fi
popd
