cmake_minimum_required(VERSION 3.15)
include(cmake/pico_sdk_import.cmake)

project(raspi-pico-aprs-tnc VERSION 0.1)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()


# Setup of eleccoder's 'ax25-aprs-lib'

include(FetchContent)
FetchContent_Declare(ax25_aprs_lib
  GIT_REPOSITORY    "https://github.com/eleccoder/ax25-aprs-lib.git" 
  GIT_SHALLOW       ON
)

FetchContent_MakeAvailable(ax25_aprs_lib)

add_library(ax25_aprs_lib::ax25beacon ALIAS ax25beacon)


# Setup of the application

set(EXE_NAME aprs_pico)

add_executable(${EXE_NAME}
src/aprs_pico.c
)

target_include_directories(${EXE_NAME} PRIVATE include)

# Set the console interface
pico_enable_stdio_usb(aprs_pico 1) # USB
# pico_enable_stdio_uart(aprs_pico 1) # UART

pico_add_extra_outputs(${EXE_NAME})
target_link_libraries(${EXE_NAME} pico_stdlib ax25_aprs_lib::ax25beacon)

