cmake_minimum_required(VERSION 3.15)

include(cmake/pico_sdk_import.cmake)    # File copied from $PICO_SDK_PATH/external
include(cmake/pico_extras_import.cmake) # File copied from $PICO_SDK_PATH/../pico-extras/external

project(raspi-pico-aprs-tnc VERSION 0.1)

set(CMAKE_C_STANDARD 11)

pico_sdk_init()


# Setup of eleccoder's version of Philip Heron's 'ax25beacon' project

include(FetchContent)
FetchContent_Declare(ax25_aprs_lib
    GIT_REPOSITORY    "https://github.com/eleccoder/ax25-aprs-lib.git" 
    GIT_SHALLOW       ON
)

FetchContent_MakeAvailable(ax25_aprs_lib)

add_library(ax25_aprs_lib::ax25beacon ALIAS ax25beacon)


# Compiler options

# Enable on demand only, since warnings from 3rd-party code will flood the console
# set(APRS_PICO_COMPILE_WARN_OPTIONS -Wundef -Wswitch-enum -Wall -Wextra -pedantic)


# ##### Configure the build of the 'aprs_pico' library #####

set(TARGET_LIB_NAME aprs_pico)

add_library(${TARGET_LIB_NAME}
    src/aprs_pico.c)

target_compile_options(${TARGET_LIB_NAME}
  PRIVATE "${APRS_PICO_COMPILE_WARN_OPTIONS}"
)

target_include_directories(${TARGET_LIB_NAME} PUBLIC
    include
)

set_target_properties(${TARGET_LIB_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)
  
target_link_libraries(${TARGET_LIB_NAME}
    ax25_aprs_lib::ax25beacon
    pico_audio_pwm        
    pico_stdlib
)


# ##### Configure the build of the DEMO applications #####

set(TARGET_DEMO_EXE_NAME aprs_pico_beacon_demo)

add_executable(${TARGET_DEMO_EXE_NAME}
    src/aprs_pico_beacon_demo.c
)

target_compile_options(${TARGET_DEMO_EXE_NAME}
  PRIVATE "${APRS_PICO_COMPILE_WARN_OPTIONS}"
)

target_include_directories(${TARGET_DEMO_EXE_NAME} PRIVATE
  include
)

# Create map/bin/hex file etc.
pico_add_extra_outputs(${TARGET_DEMO_EXE_NAME})

target_link_libraries(${TARGET_DEMO_EXE_NAME}
    ${TARGET_LIB_NAME}
    ax25_aprs_lib::ax25beacon
    pico_audio_pwm        
    pico_stdlib
)


# ##### Configure the build of the TEST application #####

set(TARGET_TEST_EXE_NAME aprs_pico_tone_test)

add_executable(${TARGET_TEST_EXE_NAME}
    src/aprs_pico_tone_test.c
)

target_compile_options(${TARGET_TEST_EXE_NAME}
  PRIVATE "${APRS_PICO_COMPILE_WARN_OPTIONS}"
)

target_include_directories(${TARGET_TEST_EXE_NAME} PRIVATE
  include
)

# Create map/bin/hex file etc.
pico_add_extra_outputs(${TARGET_TEST_EXE_NAME})

target_link_libraries(${TARGET_TEST_EXE_NAME}
    ${TARGET_LIB_NAME}
    pico_audio_pwm        
    pico_stdlib
)
