cmake_minimum_required(VERSION 3.15)

include(cmake/pico_sdk_import.cmake)    # File copied from $PICO_SDK_PATH/external
include(cmake/pico_extras_import.cmake) # File copied from $PICO_SDK_PATH/../pico-extras/external

project(raspi-pico-aprs-tnc VERSION 0.1)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()


# (1) Setup of eleccoder's version of Philip Heron's 'ax25beacon' project

include(FetchContent)
FetchContent_Declare(ax25_aprs_lib
    GIT_REPOSITORY    "https://github.com/eleccoder/ax25-aprs-lib.git" 
    GIT_SHALLOW       ON
)

FetchContent_MakeAvailable(ax25_aprs_lib)

add_library(ax25_aprs_lib::ax25beacon ALIAS ax25beacon)


# (2) Configure the library build

set(TARGET_LIB_NAME aprs_pico)

add_library(${TARGET_LIB_NAME}
    src/aprs_pico.c)

set_target_properties(${TARGET_LIB_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)
  
target_include_directories(${TARGET_LIB_NAME} PUBLIC
    include
)

# Enable on demand
#target_compile_options(${TARGET_LIB_NAME} PRIVATE "-Wundef;-Wall")

# This properly resolves the include dirs of the dependent libraries
target_link_libraries(${TARGET_LIB_NAME}
    ax25_aprs_lib::ax25beacon
    pico_audio_pwm        
    pico_stdlib
)


# (3) Configure the example application build

set(TARGET_EXAMPLE_EXE_NAME aprs_pico_example)

add_executable(${TARGET_EXAMPLE_EXE_NAME}
    src/aprs_pico_example.c
)

target_include_directories(${TARGET_EXAMPLE_EXE_NAME} PRIVATE
  include
)

# Set the console interface
pico_enable_stdio_usb(${TARGET_EXAMPLE_EXE_NAME} 1) # USB
#pico_enable_stdio_uart(${TARGET_EXAMPLE_EXE_NAME} 1) # UART

# create map/bin/hex file etc.
pico_add_extra_outputs(${TARGET_EXAMPLE_EXE_NAME})

target_link_libraries(${TARGET_EXAMPLE_EXE_NAME}
    ${TARGET_LIB_NAME}
    ax25_aprs_lib::ax25beacon
    pico_audio_pwm        
    pico_stdlib
)

