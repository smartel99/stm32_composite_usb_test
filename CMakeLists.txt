set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
cmake_minimum_required(VERSION 3.22)

# required for cross-compilation otherwise the test program always fail
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
# must ALWAYS be after static_library option
project(usb_test C CXX ASM)

option(UNIT_TEST BOOL)
if (UNIT_TEST)
    add_subdirectory(tests)
    return()
endif ()

set(SRC_DIR ${CMAKE_SOURCE_DIR})

# project settings
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 11)

#Uncomment for hardware floating point
add_compile_definitions(ARM_MATH_CM4;ARM_MATH_MATRIX_CHECK;ARM_MATH_ROUNDING)
add_compile_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)
add_link_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)

add_compile_options(-mcpu=cortex-m4 -mthumb -mthumb-interwork)
add_compile_options(-ffunction-sections -fdata-sections -fno-common -fmessage-length=0)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic-errors -Wno-unused-function -Wno-unused-parameter -Wno-unknown-pragmas")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-register")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-volatile")

# Enable assembler files preprocessing
add_compile_options($<$<COMPILE_LANGUAGE:ASM>:-x$<SEMICOLON>assembler-with-cpp>)

# To reduce binary file output size.
add_compile_options(-fdata-sections)
add_compile_options(-ffunction-sections)
add_compile_options(-fmessage-length=0)
add_compile_options(-fno-common)
add_compile_options(-fno-exceptions)
add_compile_options(-Wno-psabi)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-enforce-eh-specs")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-use-cxa-atexit")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-threadsafe-statics")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fnothrow-opt")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstrict-enums")
add_compile_options(--specs=nano.specs)

add_compile_options(-fdiagnostics-color=always)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    message(STATUS "Maximum optimization for speed")
    add_compile_options(-Os -g)
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
    message(STATUS "Maximum optimization for speed, debug info included")
    add_compile_options(-Ofast -g)
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel")
    message(STATUS "Maximum optimization for size")
    add_compile_options(-Os)
else ()
    message(STATUS "Minimal optimization, debug info included")
    add_compile_options(-O0 -g)
endif ()

include_directories(
        ${SRC_DIR}
        ${SRC_DIR}/cep
        ${SRC_DIR}/cep/can_open
        ${SRC_DIR}/g473
        ${SRC_DIR}/g473/Core/Inc
        ${SRC_DIR}/g473/Drivers/STM32G4xx_HAL_Driver/Inc
        ${SRC_DIR}/g473/Drivers/STM32G4xx_HAL_Driver/Inc/Legacy
        ${SRC_DIR}/g473/Middlewares/Third_Party/FreeRTOS/Source/include
        ${SRC_DIR}/g473/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
        ${SRC_DIR}/g473/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
        ${SRC_DIR}/g473/Middlewares/ST/STM32_USB_Device_Library/Core/Inc
        ${SRC_DIR}/g473/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc
        ${SRC_DIR}/g473/Drivers/CMSIS/Device/ST/STM32G4xx/Include
        ${SRC_DIR}/g473/Drivers/CMSIS/Include
        ${SRC_DIR}/g473/Core/ThreadSafe
        ${SRC_DIR}/usb_composite/app
        ${SRC_DIR}/usb_composite/target
        ${SRC_DIR}/usb_composite/middlewares/st/class/dcdc/inc
        ${SRC_DIR}/vendor
        ${SRC_DIR}/vendor/cmbacktrace/cm_backtrace
        ${SRC_DIR}/vendor/FreeRTOS-Plus-CLI
        ${SRC_DIR}/vendor/CANopenNode
        ${SRC_DIR}/vendor/CANopenNode/301
        ${SRC_DIR}/vendor/CANopenNode/303
        ${SRC_DIR}/vendor/CANopenNode/304
        ${SRC_DIR}/vendor/CANopenNode/305
        ${SRC_DIR}/vendor/CANopenNode/309
        ${SRC_DIR}/vendor/CANopenNode/extra
        ${SRC_DIR}/vendor/CANopenNode/storage
)

add_definitions(-DDEBUG -DUSE_HAL_DRIVER -DSTM32G473xx -DSTM32_THREAD_SAFE_STRATEGY=4)
add_definitions(-DconfigCOMMAND_INT_MAX_OUTPUT_SIZE=256)

file(GLOB_RECURSE SOURCES
        "${SRC_DIR}/g473/Core/*.*"
        "${SRC_DIR}/g473/Middlewares/*.*"
        "${SRC_DIR}/g473/Drivers/*.*"
        "${SRC_DIR}/cep/*.*"
        "${SRC_DIR}/usb_composite/*.*"
        "${SRC_DIR}/vendor/logging/*.*"
        "${SRC_DIR}/vendor/cmbacktrace/cm_backtrace/cm_backtrace.c"
        "${SRC_DIR}/vendor/cmbacktrace/cm_backtrace/fault_handler/gcc/cmb_fault.s"
        "${SRC_DIR}/vendor/FreeRTOS-Plus-CLI/FreeRTOS_CLI.c"
        "${SRC_DIR}/vendor/CANopenNode/CANopen.c"
        "${SRC_DIR}/vendor/CANopenNode/301/*.c"
        "${SRC_DIR}/vendor/CANopenNode/303/*.c"
        "${SRC_DIR}/vendor/CANopenNode/304/*.c"
        "${SRC_DIR}/vendor/CANopenNode/305/*.c"
        "${SRC_DIR}/vendor/CANopenNode/309/*.c"
        "${SRC_DIR}/vendor/CANopenNode/extra/*.c"
        "${SRC_DIR}/vendor/CANopenNode/storage/CO_storage.c"
)

# Add cm_backtrace utility functions to FreeRTOS's task.c, if needed.
file(STRINGS ${SRC_DIR}/g473/Middlewares/Third_Party/FreeRTOS/Source/tasks.c already_cat REGEX "Support For CmBacktrace")
if (already_cat STREQUAL "")
    message(STATUS "Appending CmBacktrace functions to FreeRTOS/tasks.c")
    file(READ ${SRC_DIR}/vendor/cmbacktrace/freertos_task.c.template template_content)
    file(APPEND ${SRC_DIR}/g473/Middlewares/Third_Party/FreeRTOS/Source/tasks.c "${template_content}")
else ()
    message(STATUS "No need to add CmBacktrace functions to FreeRTOS/tasks.c!")
endif ()


set(LINKER_SCRIPT ${SRC_DIR}/g473/STM32G473QETX_FLASH.ld)

add_link_options(-Wl,-gc-sections,--print-memory-usage,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map)
add_link_options(-mcpu=cortex-m4 -mthumb -mthumb-interwork)
add_link_options(--specs=nano.specs)
add_link_options(-u _printf_float)
add_link_options(-Wl,--no-warn-rwx-segments) # Disable GCC 12.2 new warning for RWX segments
add_link_options(-T ${LINKER_SCRIPT})

add_executable(${PROJECT_NAME}.elf ${SOURCES} ${LINKER_SCRIPT})

set(BIN_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin)

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${PROJECT_NAME}.elf> ${BIN_FILE}
        COMMENT "Building ${HEX_FILE}
Building ${BIN_FILE}")
