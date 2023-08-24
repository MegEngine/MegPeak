#!/usr/bin/env bash
set -e

BUILD_TYPE=Release
ARCH=arm64-v8a
OPENCL=OFF
ALL_BENCHMARK=OFF
CPUINFO=OFF
DOT=OFF
MMA=OFF
BFMMA=OFF
ARCH_LIST="arm64-v8a armeabi-v7a"

READLINK=readlink
MAKEFILE_TYPE="Ninja"
OS=$(uname -s)

function usage() {
    echo "$0 args1 .."
    echo "available args detail:"
    echo "-m : machine arch(arm64-v8a, armeabi-v7a)"
    echo "-d : enable build with dotprod"
    echo "-a : enable build with mma"
    echo "-f : enable build with bfmma"
    echo "-l : enable build with opencl"
    echo "-t : enable total benchmark"
    echo "-c : enable build with cpuinfo"
    echo "-h : show usage"
    echo "example: $0 -m armeabi-v7a"
    exit -1
}

while getopts "adlcfhm:t" arg
do
    case $arg in
        m)
            echo "build with arch:$OPTARG"
            ARCH=$OPTARG
            ;;
        l)
            echo "build with opencl"
            OPENCL=ON
            ;;
        c)
            echo "build with cpuinfo"
            CPUINFO=ON
            ;;
        a)
            echo "build with mma"
            MMA=ON
            ;;
        d)
            echo "build with dotprod"
            DOT=ON
            ;;
        f)
            echo "build with bfmmla"
            BFMMA=ON
            ;;
        h)
            echo "show usage"
            usage
            exit 0
            ;;
        t)
            echo "build with total benchmark"
            ALL_BENCHMARK=ON
            ;;
    esac
done

if [ $OS = "Darwin" ];then
    READLINK=greadlink
elif [[ $OS =~ "NT" ]]; then
    echo "BUILD in NT ..."
    MAKEFILE_TYPE="Unix"
fi
SRC_DIR=$($READLINK -f "`dirname $0`/")

function cmake_build() {
    if [ $NDK_ROOT ];then
        echo "NDK_ROOT: $NDK_ROOT"
    else
        echo "Please define env var NDK_ROOT first"
        exit 1
    fi

    BUILD_DIR=$SRC_DIR/build-${ARCH}/
    BUILD_ABI=$1
    BUILD_NATIVE_LEVEL=$2
    echo "build dir: $BUILD_DIR"
    echo "build ARCH: $ARCH"
    echo "build ABI: $BUILD_ABI"
    echo "build native level: $BUILD_NATIVE_LEVEL"
    echo "BUILD MAKEFILE_TYPE: $MAKEFILE_TYPE"

    echo "create build dir"
    mkdir -p $BUILD_DIR
    cd $BUILD_DIR
    cmake -G "$MAKEFILE_TYPE" \
        "-B$BUILD_DIR" \
        "-S$SRC_DIR" \
        -DCMAKE_TOOLCHAIN_FILE="$NDK_ROOT/build/cmake/android.toolchain.cmake" \
        -DANDROID_NDK="$NDK_ROOT" \
        -DANDROID_ABI=$BUILD_ABI \
        -DANDROID_NATIVE_API_LEVEL=$BUILD_NATIVE_LEVEL \
        -DMEGPEAK_ENABLE_OPENCL=${OPENCL} \
        -DMEGPEAK_ENABLE_ALL_BENCHMARK=${ALL_BENCHMARK} \
        -DMEGPEAK_ENABLE_DOT=${DOT} \
        -DMEGPEAK_ENABLE_MMA=${MMA} \
        -DMEGPEAK_ENABLE_BFMMA=${BFMMA} \
        -DMEGPEAK_USE_CPUINFO=${CPUINFO}

    ninja ${Target}
}

api_level=16
abi="armeabi-v7a with NEON"
IFS=""
if [ "$ARCH" = "arm64-v8a" ]; then
    api_level=21
    abi="arm64-v8a"
elif [ "$ARCH" = "armeabi-v7a" ]; then
    api_level=16
    abi="armeabi-v7a with NEON"
else
    echo "ERR CONFIG ABORT NOW!!"
    exit -1
fi

cmake_build $abi $api_level
