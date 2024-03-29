set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(TOOLCHAIN_PATH /opt/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu/bin)
set(SYSROOT_PATH ${TOOLCHAIN_PATH}/../aarch64-none-linux-gnu/libc)
set(TOOLCHAIN_PREFIX aarch64-none-linux-gnu-)

set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}g++)
set(CMAKE_LD_COMPILER ${TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}ld)
set(CMAKE_AR_COMPILER ${TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}ar)

set(CMAKE_SYSROOT ${SYSROOT_PATH})
set(CMAKE_EXE_LINKER_FLAGS "--sysroot=${SYSROOT_PATH}")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

cmake_minimum_required(VERSION 3.1.15)
project(AstraMiniS LANGUAGES CXX C)

add_executable(
    ${PROJECT_NAME}
    src/main.cpp
    src/net/udpsender.cpp
)
message(${CMAKE_SOURCE_DIR})
set(OrbbecSDK_ROOT_DIR ${CMAKE_SOURCE_DIR}/SDK)
set(OrbbecSDK_LIBRARY_DIRS ${OrbbecSDK_ROOT_DIR}/lib)
set(OrbbecSDK_INCLUDE_DIR ${OrbbecSDK_ROOT_DIR}/include)

include_directories(${OrbbecSDK_INCLUDE_DIR}
${CMAKE_SOURCE_DIR}/src/common
${CMAKE_SOURCE_DIR}/src/net
)

link_directories(${OrbbecSDK_LIBRARY_DIRS})

target_link_libraries(${PROJECT_NAME} PRIVATE ${OrbbecSDK_LIBRARY_DIRS}/libOrbbecSDK.so
    # ${OpenCV_LIBS}
)