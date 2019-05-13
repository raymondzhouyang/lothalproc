#!/bin/bash -x

export R2_BUILD_TOOL_DIR=${PWD}/../BuildTools
export R2_EXTERNAL_DIR=${PWD}/../external
export R2_CMAKE_MODULES_DIR=${PWD}/../cmake-modules
./build.sh --target=k18 --debug=true
