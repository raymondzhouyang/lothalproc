CMAKE_MINIMUM_REQUIRED(VERSION 3.2)

include(CheckSymbolExists)
include(CMakePushCheckState)
include(GNUInstallDirs)
include(${CUSTOM_CMAKE_MODULES}/common.mk)
include(buildFlag.mk)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

message("CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}")

execute_process(COMMAND git describe --match=NeVeRmAtCh --always --abbrev=40 --dirty
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_SHA1
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
add_definitions(-DCOMMIT="${GIT_SHA1}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option(USE_GCC "force use gcc" OFF)
if (USE_GCC)
    SET(CMAKE_C_COMPILER gcc)
    SET(CMAKE_CXX_COMPILER g++)
endif()
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
    set(COMPILER_IS_CLANG TRUE)
else()
    set(COMPILER_IS_CLANG FALSE)
endif()

if(BUILD_TARGET STREQUAL "")
    set(BUILD_TARGET host)
endif()

message("##### ${externalPrefix}")

set(lib_name rokid)
set(third_lib_name third-party)

set(common_libs
    lothal
    r2base
    )

if (NOT BUILD_TARGET MATCHES "android")
set(common_libs
    ${common_libs}
    fftw3f
    )
endif()

if (NOT BUILD_TARGET MATCHES "k18")
set(common_libs
    ${common_libs}
    opus
    )
endif()

set(common_include_paths
    include
    include/r2base
    include/lothal
    )

set(common_headers
    )

findPackage(common REQUIRED
    HINTS ${externalPrefix}
    INC_PATH_SUFFIX ${common_include_paths}
    LIB_PATH_SUFFIX ${lib_name} ${third_lib_name}
    SHARED_LIBS ${common_libs}
    RPATH_LINK
)

include_directories(
    ${externalPrefix}/include
    ${externalPrefix}/include/r2base
    ${externalPrefix}/include/lothal
)

if(HAVE_FLORA)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAVE_FLORA")

    set(speech_libs
        caps flora-cli
    )
    set(speech_include_paths
        include/caps
    )
    set(speech_headers
        caps.h flora-cli.h
    )
else()
    set(speech_libs
        speech uWS ssl crypto z
    )
    set(speech_include_paths
        speech
        include/speech
    )
    set(speech_headers
        speech.h
    )
endif(HAVE_FLORA)

message("### find speech or flora")
findPackage(speech REQUIRED
    HINTS ${externalPrefix}
    INC_PATH_SUFFIX ${speech_include_paths}
    LIB_PATH_SUFFIX ${lib_name} ${third_lib_name}
    HEADERS ${speech_headers}
    SHARED_LIBS ${speech_libs}
    RPATH_LINK
    )

if(HAVE_MIC_ARRAY)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAVE_MIC_ARRAY")
    set(micarray_include_paths
        include/micarray/include
    )

    if(BUILD_TARGET STREQUAL a113)
        set(micarray_libs
            android_hardware android_cutils
        )
        set(micarray_headers
            hardware/mic_array.h
        )
    elseif(BUILD_TARGET STREQUAL k18)
        set(micarray_libs
            property mic_array.default android_hardware android_cutils
        )
        set(micarray_headers
            ${micarray_headers}
            r2hw/mic_array.h
        )
    elseif(BUILD_TARGET STREQUAL android)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
        set(micarray_headers
            ${micarray_headers}
            r2hw/mic_array.h
        )
        set(micarray_libs
            hardware
        )
    endif()

    message("micarray_libs ${micarray_libs}")
    findPackage(micarray REQUIRED
        HINTS ${externalPrefix}
        HEADERS ${micarray_headers}
        INC_PATH_SUFFIX ${micarray_include_paths}
        LIB_PATH_SUFFIX ${lib_name} ${third_lib_name}
        SHARED_LIBS ${micarray_libs}
        RPATH_LINK
        )
endif()

file(GLOB_RECURSE demo_src
    src/*.cpp
    src/*.hpp
)

add_executable(lothalproc ${demo_src})

target_include_directories(lothalproc PRIVATE
    ${common_INCLUDE_DIRS}
    ${speech_INCLUDE_DIRS}
    ${micarray_INCLUDE_DIRS}
)

target_link_libraries(lothalproc
    ${common_LIBRARIES}
    ${speech_LIBRARIES}
    ${micarray_LIBRARIES}
)

install(TARGETS lothalproc
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
)
