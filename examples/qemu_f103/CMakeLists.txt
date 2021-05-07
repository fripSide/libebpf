cmake_minimum_required(VERSION 3.6)

# https://gitlab.kitware.com/cmake/cmake/-/issues/18730

#---------------------------------------------------------------------------------------
# Set toolchain paths
#---------------------------------------------------------------------------------------
set(TOOLCHAIN_PREFIX /mnt/f/IoT/Tools/gcc-arm-none-eabi-9-2020-q2-update/)
set(TOOLCHAIN arm-none-eabi)
if(NOT DEFINED TOOLCHAIN_PREFIX)
    if(CMAKE_HOST_SYSTEM_NAME STREQUAL Linux)
        set(TOOLCHAIN_PREFIX "/usr")
    elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL Darwin)
        set(TOOLCHAIN_PREFIX "/usr/local")
    elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
        message(STATUS "Please specify the TOOLCHAIN_PREFIX !\n For example: -DTOOLCHAIN_PREFIX=\"C:/Program Files/GNU Tools ARM Embedded\" ")
    else()
        set(TOOLCHAIN_PREFIX "/usr")
        message(STATUS "No TOOLCHAIN_PREFIX specified, using default: " ${TOOLCHAIN_PREFIX})
    endif()
endif()
set(TOOLCHAIN_BIN_DIR ${TOOLCHAIN_PREFIX}/bin)
set(TOOLCHAIN_INC_DIR ${TOOLCHAIN_PREFIX}/${TOOLCHAIN}/include)
set(TOOLCHAIN_LIB_DIR ${TOOLCHAIN_PREFIX}/${TOOLCHAIN}/lib)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR ARM)


set(CROSS_COMPILE_PREFIX arm-none-eabi)
set(CMAKE_C_COMPILER ${CROSS_COMPILE_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${CROSS_COMPILE_PREFIX}-g++)
set(CMAKE_ASM_COMPILER ${CROSS_COMPILE_PREFIX}-gcc)
set(CMAKE_OBJCOPY ${CROSS_COMPILE_PREFIX}-objcopy)
set(CMAKE_OBJDUMP ${CROSS_COMPILE_PREFIX}-objdump)


# disable compiler check
set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)

#---------------------------------------------------------------------------------------
# Set compilers
#---------------------------------------------------------------------------------------
set(CMAKE_C_COMPILER ${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN}-gcc${TOOLCHAIN_EXT} CACHE INTERNAL "C Compiler")
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN}-g++${TOOLCHAIN_EXT} CACHE INTERNAL "C++ Compiler")
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN}-gcc${TOOLCHAIN_EXT} CACHE INTERNAL "ASM Compiler")

set(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_PREFIX}/${${TOOLCHAIN}} ${CMAKE_PREFIX_PATH})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

SET(CMAKE_GENERATOR "Unix Makefiles")
project(arm-start VERSION 0.1 LANGUAGES C ASM)

set(IOTPATCH_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../PatchCore/)


# set(CMAKE_C_FLAGS -fno-common -ffreestanding -O0 -gdwarf-2 -g3 -Wall -Werror -mcpu=cortex-m4 -mthumb -Wl,-Tstart.ld -nostartfiles CACHE INTERNAL "C compiler flags")

message("PROJECT DIR:" ${PROJECT_SOURCE_DIR})

file(GLOB iotpatchsrc
        "${IOTPATCH_DIR}/hotpatch/include/*.h"
        "${IOTPATCH_DIR}/hotpatch/src/*.c"
        "${IOTPATCH_DIR}/app/*.h"
        "${IOTPATCH_DIR}/app/*.c"
        "${IOTPATCH_DIR}/libebpf/include/*.h"
        "${IOTPATCH_DIR}/libebpf/src/*.c")

list(REMOVE_ITEM iotpatchsrc "${IOTPATCH_DIR}/libebpf/src/main.c")

file(GLOB ARM_SRC
    ${PROJECT_SOURCE_DIR}/src/*.c
)
set(EXECUTABLE ${PROJECT_NAME}.elf)

include_directories("${PROJECT_SOURCE_DIR}/src/"
    "${IOTPATCH_DIR}/"
    "${IOTPATCH_DIR}/hotpatch"
    "${IOTPATCH_DIR}/app"
    "${IOTPATCH_DIR}/libebpf"
)

add_executable(${EXECUTABLE} ${ARM_SRC} ${iotpatchsrc})

add_definitions(-DSYS_CORTEX_M4 -DDEV_QEMU)

target_compile_options(${EXECUTABLE} PRIVATE
        -mcpu=cortex-m4
        -mthumb
        -mfpu=fpv4-sp-d16
        -mfloat-abi=hard
        -g3 
        -fdata-sections
        -ffunction-sections

        -Wall

        $<$<CONFIG:Debug>:-Og>
        )

target_link_options(${EXECUTABLE} PRIVATE
        -T${CMAKE_SOURCE_DIR}/src/os.ld
        -mcpu=cortex-m4
        -mthumb
        -mfpu=fpv4-sp-d16
        -mfloat-abi=hard
        -specs=nano.specs
        -lc
        -lm
        -lnosys
        -Wl,-Map=${PROJECT_NAME}.map,--cref,--print-memory-usage
        -Wl,--gc-sections
        )

# Print executable size
add_custom_command(TARGET ${EXECUTABLE}
        POST_BUILD
        COMMAND arm-none-eabi-size ${EXECUTABLE})

# Create hex file
add_custom_command(TARGET ${EXECUTABLE}
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex ${EXECUTABLE} ${PROJECT_NAME}.hex
        COMMAND ${CMAKE_OBJCOPY} -O binary ${EXECUTABLE} ${PROJECT_NAME}.bin
        COMMAND ${CMAKE_OBJDUMP} -S ${EXECUTABLE} > ${PROJECT_NAME}.list
    )

add_custom_target(qemu 
    COMMAND make
    COMMAND qemu-system-arm -M stm32-p103 -semihosting -nographic -kernel ${PROJECT_NAME}.bin
    )

add_custom_target(debug
    COMMAND make
    COMMAND qemu-system-arm -M stm32-p103 -semihosting -nographic -kernel ${PROJECT_NAME}.bin -s -S
    )

add_custom_target(gdb
    COMMAND arm-none-eabi-gdb ${PROJECT_NAME}.elf -ex "target remote:1234"
    )
