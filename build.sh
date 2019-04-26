#!/bin/sh

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0;0m'
info() {
    echo ${GREEN}$1${NC}
}

error() {
    echo ${RED}$1${NC}
}

PROJECT_DIR=$(pwd)
OUTPUT_DIR="${PROJECT_DIR}/output"
BUILD_DIR="${PROJECT_DIR}/output/intermediates"

if [ "${R2_BUILD_TOOL_DIR}" = "" ]; then
    R2_BUILD_TOOL_DIR="/opt/r2_build_tool/BuildTools"
    info "Set build tools directory to ${R2_BUILD_TOOL_DIR}"
fi

if [ "${R2_CMAKE_MODULES_DIR}" = "" ]; then
    R2_CMAKE_MODULES_DIR="/opt/r2_build_tool/cmake-modules"
    info "Set build tools directory to ${R2_CMAKE_MODULES_DIR}"
fi

if [ "${R2_EXTERNAL_DIR}" = "" ]; then
    R2_EXTERNAL_DIR="/opt/r2_build_tool/external"
    info "Set build tools directory to ${R2_EXTERNAL_DIR}"
fi

info "R2_BUILD_TOOL_DIR=${R2_BUILD_TOOL_DIR}"
info "R2_EXTERNAL_DIR=${R2_EXTERNAL_DIR}"
info "R2_CMAKE_MODULES_DIR=${R2_CMAKE_MODULES_DIR}"

export NDK="${R2_BUILD_TOOL_DIR}/android-ndk-r15c"
a113ToolChain=${R2_BUILD_TOOL_DIR}/toolchain/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu
k18ToolChain=${R2_BUILD_TOOL_DIR}/toolchain/toolchain-arm_cortex-a7+neon_gcc-5.3.0_glibc-2.22_eabi

BUILD_DEBUG=OFF

for opt
do
    case ${opt} in
        *=?*) optarg=$(expr "X${opt}" : '[^=]*=\(.*\)') ;;
        *) optarg= ;;
    esac

    case ${opt} in
        --target=*)
            BUILD_TARGET=${optarg}
            ;;
        --debug=*)
            BUILD_DEBUG=${optarg}
            ;;
    esac
done

CONFIG_OPTS="--cmake-modules=${R2_CMAKE_MODULES_DIR}   \
    --external-dir=${R2_EXTERNAL_DIR}/${BUILD_TARGET}  \
    --dest-dir=${OUTPUT_DIR}        \
    --build-dir=${BUILD_DIR}        \
    --build-target=${BUILD_TARGET}  \
    --debug=${BUILD_DEBUG} 
"

GIT_SHA1=`git describe --match=NeVeRmAtCh --always --abbrev=40 --dirty`
# GIT_SHA1=`git describe --always --abbrev=40 --dirty`
FEATURES="-DHAVE_FLORA=1 -DHAVE_MIC_ARRAY=1 -DGIT_SHA1=${GIT_SHA1}"

if [ "${BUILD_TARGET}" = "a113" ]; then
    ARCH=arm64

    FEATURES="${FEATURES}"
    CONFIG_OPTS="${CONFIG_OPTS}           \
        --toolchain=${a113ToolChain}      \
        --cross-prefix=aarch64-linux-gnu-"
elif [ "${BUILD_TARGET}" = "android" ]; then
    FEATURES="${FEATURES} -DANDROID_ARM_NEON=TRUE -DHAVE_FLORA=0"
    CONFIG_OPTS="${CONFIG_OPTS}        \
        --android-ndk=${NDK}           \
        --android-abi=armeabi-v7a      \
        --android-api-level=20         \
        --android-toolchain-name=clang \
        --android-stl=c++_static"

elif [ "${BUILD_TARGET}" = "k18" ]; then
    ARCH=arm32
    export STAGING_DIR=${BUILD_DIR}/STAGING_DIR
    FEATURES="${FEATURES}"
    CONFIG_OPTS="${CONFIG_OPTS}                  \
        --toolchain=${k18ToolChain}              \
        --cross-prefix=arm-openwrt-linux-gnueabi-"
elif [ "${BUILD_TARGET}" = "host" ]; then
    FEATURES="${FEATURES} -DHAVE_ALSA=0 -DHAVE_MIC_ARRAY=0 -DHAVE_FLORA=0"
else
    echo "Usage: build.sh --target=TARGET [--debug=ON|OFF]"
    echo "Supported targets: host, a113, k18 and android"
    echo "Features to select: ALSA, MIC_ARRAY, FLORA, DEBUG"
    exit 1
fi

if [ ! -d ${R2_BUILD_TOOL_DIR} ]; then
    error "Cannot find the build tools directory ${R2_BUILD_TOOL_DIR}"
    error "Read README.md to get help."
    exit 1
fi

info "Clean ${BUILD_DIR}"
rm -rf ${BUILD_DIR}

info "Show config options:"
info "${CONFIG_OPTS}"
./config.sh ${CONFIG_OPTS} --other="${FEATURES}"
if [ $? -ne 0 ]; then
    error "Failed to config, exit now."
    exit 1
fi

cd ${BUILD_DIR}
make ARCH=${ARCH} -j4 V=1 && make install
cd ${PROJECT_DIR}

