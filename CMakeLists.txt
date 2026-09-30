cmake_minimum_required(VERSION 3.22)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

set(CMAKE_PROJECT_NAME cw32l012_blank)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

enable_language(C ASM)

add_executable(${CMAKE_PROJECT_NAME})

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    Libraries/inc
    USER/inc
    Board
    BSP
)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    CW32L012x8
    "__WEAK=__attribute__((weak))"
    I2C_WAIT_TIMEOUT=0x4000U
)

file(GLOB_RECURSE DRIVER_SOURCES "Libraries/src/*.c")
list(REMOVE_ITEM DRIVER_SOURCES "${CMAKE_SOURCE_DIR}/Libraries/src/system_cw32l012.c")

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    startup_cw32l012x8.s
    BSP/BSP_led.c
    BSP/BSP_UART.c
    BSP/BSP_Potentiometer.c
    BSP/BSP_Button.c
    BSP/BSP_MOTOR.c
    BSP/BSP_FOC.c
    BSP/BSP_Current.c
    BSP/BSP_Vbus.c
    BSP/BSP_AS5600.c
    BSP/BSP_DebugSnap.c
    BSP/BSP_MotorDetect.c
    USER/src/main.c
    USER/src/interrupts_cw32l012.c
    USER/src/system_cw32l012_override.c
    ${DRIVER_SOURCES}
)

list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE m)

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary ${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.bin
    COMMENT "Generating hex and bin files"
)
