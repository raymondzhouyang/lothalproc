#!/bin/bash

function print_prompt
{
cat << CONFEOF
Usage: $0 [OPTION]... [VAR=VALUE]...

Configuration:
    --help                        display this help and exit
    --debug                       build for debug
    --build-dir=DIR               build directory
    --build-target=TARGET         target platform name (host|all3|kamino)
    --cmake-modules=DIR           directory of cmake modules file exist
    --dest-dir                    output directory
    --other                       other values

Dependencies:
    --external_dir=DIR            platform or not we build libs and header files dir

Cross Compile:
    --toolchain=DIR               toolchain install dir
    --cross-prefix=PREFIX         compiler name prefix
Android config:
    --android-ndk                 android ndk path
    --android-abi                 element of armeabi、armeabi-v7a、armeabi-v7a with NEON、armeabi-v7a with VFPV3、armeabi-v6 with VFP、x86、mips、arm64-v8a、x86_64、mips64
    --android-api-level           android sdk level, value such as 16、21
    --android-toolchain-name      you can find in NDK directory, such as arm-linux-androideabi-4.8
    --android-stl                 element of  none、system、system_re、gabi++_static、gabi++_shared、stlport_static、stlport_shared、gnustl_static、gnustl_shared
CONFEOF
}

builddir="build"
cmake_modules_dir="cmake"
dest_dir="output"
build_target=""
ndk=""

CMAKE_ARGS=
dest_dir=
for conf_opt
do
    case $conf_opt in
        *=?*) conf_optarg=`expr "X$conf_opt" : '[^=]*=\(.*\)'` ;;
        *) conf_optarg= ;;
    esac

    case $conf_opt in
    --help)
        print_prompt
        exit 0
        ;;
    --debug=*)
        CMAKE_ARGS=(${CMAKE_ARGS[@]} -DBUILD_DEBUG=${conf_optarg})
        ;;
    --build-dir=*)
        builddir=$conf_optarg
        ;;
    --build-target=*)
        build_target=$conf_optarg
        CMAKE_ARGS=(${CMAKE_ARGS[@]} -DBUILD_TARGET=$conf_optarg)
        ;;
    --cmake-modules=*)
        cmake_modules_dir=$conf_optarg
        ;;
    --external-dir=*)
        CMAKE_ARGS=(${CMAKE_ARGS[@]} -DexternalPrefix=$conf_optarg)
        ;;
    --toolchain=*)
        CMAKE_ARGS=(${CMAKE_ARGS[@]} -DTOOLCHAIN_HOME=$conf_optarg)
        CROSS_COMPILE=yes
        ;;
    --cross-prefix=*)
        CMAKE_ARGS=(${CMAKE_ARGS[@]} -DCROSS_PREFIX=$conf_optarg)
        ;;
    --dest-dir=*)
        dest_dir=$conf_optarg
        ;;
    --android-ndk=*)
        ndk=${conf_optarg}
        CMAKE_ARGS=(${CMAKE_ARGS[@]} -DANDROID_NDK=${conf_optarg})
        ;;
    --android-abi=*)
        CMAKE_ARGS=(${CMAKE_ARGS[@]} -DANDROID_ABI=$conf_optarg)
        ;;
    --android-api-level=*)
        CMAKE_ARGS=(${CMAKE_ARGS[@]} -DANDROID_NATIVE_API_LEVEL=android-$conf_optarg)
        ;;
    --android-toolchain-name=*)
        CROSS_COMPILE=yes
        CMAKE_ARGS=(${CMAKE_ARGS[@]} -DANDROID_TOOLCHAIN_NAME=$conf_optarg)
        ;;
    --android-stl=*)
        CMAKE_ARGS=(${CMAKE_ARGS[@]} -DANDROID_STL=$conf_optarg)
        ;;
    --other=*)
        CMAKE_ARGS=(${CMAKE_ARGS[@]} $conf_optarg)
        ;;
    esac
done

if [ "$dest_dir" != "" ] ;then
    CMAKE_ARGS=(${CMAKE_ARGS[@]} -DCMAKE_INSTALL_PREFIX=$dest_dir/$build_target)
fi

CMAKE_ARGS=(${CMAKE_ARGS[@]} -DCUSTOM_CMAKE_MODULES=$cmake_modules_dir)
CUR_DIR=`pwd`
if [ x$CROSS_COMPILE = x"yes" ]; then
    if [ x$build_target = x"android"  ]; then
        CMAKE_ARGS=(${CMAKE_ARGS[@]} -DCMAKE_TOOLCHAIN_FILE=$ndk/build/cmake/android.toolchain.cmake)
    elif [ x$build_target = x"k18" ]; then
        CMAKE_ARGS=(${CMAKE_ARGS[@]} -DCMAKE_TOOLCHAIN_FILE=$cmake_modules_dir/k18.toolchain.cmake)
    else
        CMAKE_ARGS=(${CMAKE_ARGS[@]} -DCMAKE_TOOLCHAIN_FILE=$cmake_modules_dir/a113.toolchain.cmake)
    fi
fi

mkdir -p $builddir
cd $builddir

echo ${CMAKE_ARGS[@]}
cmake ${CUR_DIR} ${CMAKE_ARGS[@]}

