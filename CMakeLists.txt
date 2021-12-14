cmake_minimum_required (VERSION 3.5)

project(netq_plc VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_FLAGS "-std=gnu++11 -fPIC -DPIC -DNDEBUG -DWEBRTC_POSIX -Wall -Wno-format-truncation -O2 -g -no-pie")
set(CMAKE_C_FLAGS "-fPIC -DPIC -DNDEBUG -DWEBRTC_POSIX -Wall -Wno-format-truncation -O2 -g -no-pie")


FILE (GLOB DSP_SOURCES "${PROJECT_SOURCE_DIR}/common_audio/signal_processing/*.c" )
FILE (GLOB BASE_SOURCES "${PROJECT_SOURCE_DIR}/base/*.cc" )
FILE (GLOB VAD_SOURCES "${PROJECT_SOURCE_DIR}/common_audio/vad/*.c" )

message(STATUS "DSP:  ${DSP_SOURCES}")

message(STATUS "BASE:  ${BASE_SOURCES}")

message(STATUS "VAD:  ${VAD_SOURCES}")

#######################################################

add_library(dsp STATIC 
    ${DSP_SOURCES}
    ${PROJECT_SOURCE_DIR}/common_audio/signal_processing/dot_product_with_scale.cc
    ${PROJECT_SOURCE_DIR}/common_audio/third_party/spl_sqrt_floor/spl_sqrt_floor.c
)

target_include_directories(dsp PUBLIC
    ${PROJECT_SOURCE_DIR}
)

########################################################

add_library(base STATIC 
    ${BASE_SOURCES}
)

target_include_directories(base PUBLIC
    ${PROJECT_SOURCE_DIR}
)

########################################################

add_library(vad STATIC 
    ${VAD_SOURCES}
)

target_include_directories(vad PUBLIC
    ${PROJECT_SOURCE_DIR}
)

########################################################

add_library(plc STATIC 
    ${PROJECT_SOURCE_DIR}/neteq/audio_vector.cc
    ${PROJECT_SOURCE_DIR}/neteq/audio_multi_vector.cc
    ${PROJECT_SOURCE_DIR}/neteq/cross_correlation.cc
    ${PROJECT_SOURCE_DIR}/neteq/post_decode_vad.cc
    ${PROJECT_SOURCE_DIR}/neteq/background_noise.cc
    ${PROJECT_SOURCE_DIR}/neteq/dsp_helper.cc
    ${PROJECT_SOURCE_DIR}/neteq/random_vector.cc
    ${PROJECT_SOURCE_DIR}/neteq/sync_buffer.cc
    ${PROJECT_SOURCE_DIR}/neteq/expand.cc
    ${PROJECT_SOURCE_DIR}/neteq/merge.cc
)

target_include_directories(plc PUBLIC
    ${PROJECT_SOURCE_DIR}
)


# utils test demo 
add_executable(mutetest 
    ${PROJECT_SOURCE_DIR}/test_mute.cc
)

add_executable(copytest 
    ${PROJECT_SOURCE_DIR}/test_copy.cc
)

add_executable(noisetest 
    ${PROJECT_SOURCE_DIR}/test_noise.cc
)

target_include_directories(noisetest PUBLIC
    ${PROJECT_SOURCE_DIR}
)

target_link_libraries(noisetest
    plc
    vad 
    dsp 
    base 
    rt
)

add_executable(plctest 
    ${PROJECT_SOURCE_DIR}/test_plc.cc
)

target_include_directories(plctest PUBLIC
    ${PROJECT_SOURCE_DIR}
)

target_link_libraries(plctest
    plc
    vad 
    dsp 
    base 
    rt
)