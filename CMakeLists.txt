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
    Middlewares/SguanFOC
)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    CW32L012x8
    "__WEAK=__attribute__((weak))"
    I2C_WAIT_TIMEOUT=0x4000U
)

file(GLOB_RECURSE DRIVER_SOURCES "Libraries/src/*.c")
list(REMOVE_ITEM DRIVER_SOURCES "${CMAKE_SOURCE_DIR}/Libraries/src/system_cw32l012.c")

# SguanFOC v3.1.0 无感 SMO 所需源文件（裁剪未用观测器以省 Flash）
set(SGUANFOC_SOURCES
    Middlewares/SguanFOC/SguanFOC.c
    Middlewares/SguanFOC/Sguan_math.c
    Middlewares/SguanFOC/Sguan_IQmath.c
    Middlewares/SguanFOC/Sguan_PID.c
    Middlewares/SguanFOC/Sguan_PLL.c
    Middlewares/SguanFOC/Sguan_Filter.c
    Middlewares/SguanFOC/Sguan_SVPWM.c
    Middlewares/SguanFOC/Sguan_SPWM.c
    Middlewares/SguanFOC/Sguan_SMO.c
    Middlewares/SguanFOC/Sguan_LTD.c
    Middlewares/SguanFOC/Sguan_Feedforward.c
    Middlewares/SguanFOC/Sguan_Optimize.c
    Middlewares/SguanFOC/Sguan_Identify.c
    Middlewares/SguanFOC/Sguan_MotorStatus.c
    Middlewares/SguanFOC/Sguan_printf.c
    Middlewares/SguanFOC/Sguan_SMC.c
    Middlewares/SguanFOC/Sguan_STA.c
    Middlewares/SguanFOC/Sguan_Ladrc.c
)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    startup_cw32l012x8.s
    BSP/BSP_led.c
    BSP/BSP_UART.c
    BSP/BSP_Potentiometer.c
    BSP/BSP_Button.c
    BSP/BSP_MOTOR.c
    BSP/BSP_Current.c
    USER/src/main.c
    USER/src/interrupts_cw32l012.c
    USER/src/system_cw32l012_override.c
    ${SGUANFOC_SOURCES}
    ${DRIVER_SOURCES}
)

list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE m)

# soft-float FOC 体积偏大，适当优化
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
    $<$<COMPILE_LANGUAGE:C>:-ffast-math>
)

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary ${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.bin
    COMMENT "Generating hex and bin files"
)
