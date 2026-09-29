set(CMAKE_SYSTEM_NAME      Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER_ID   GNU)
set(CMAKE_CXX_COMPILER_ID GNU)

set(TOOLCHAIN_PREFIX arm-none-eabi-)

# Optional override: -DARM_TOOLCHAIN_PATH=... or env ARM_TOOLCHAIN_PATH
if(NOT ARM_TOOLCHAIN_PATH AND DEFINED ENV{ARM_TOOLCHAIN_PATH})
    set(ARM_TOOLCHAIN_PATH "$ENV{ARM_TOOLCHAIN_PATH}")
endif()

# Common Windows installs (first existing bin/ wins)
if(NOT ARM_TOOLCHAIN_PATH)
    set(_ARM_TOOLCHAIN_CANDIDATES
        "C:/ST/STM32CubeCLT_1.18.0/GNU-tools-for-STM32/bin"
        "C:/ST/STM32CubeCLT/GNU-tools-for-STM32/bin"
        "E:/STM32_Projects/arm-gnu-toolchain-15.2.rel1-mingw-w64-x86_64-arm-none-eabi/bin"
        "C:/Program Files (x86)/Arm GNU Toolchain arm-none-eabi/bin"
        "C:/Program Files/Arm GNU Toolchain arm-none-eabi/bin"
    )
    foreach(_cand IN LISTS _ARM_TOOLCHAIN_CANDIDATES)
        if(EXISTS "${_cand}/arm-none-eabi-gcc.exe" OR EXISTS "${_cand}/arm-none-eabi-gcc")
            set(ARM_TOOLCHAIN_PATH "${_cand}")
            break()
        endif()
    endforeach()
    unset(_cand)
    unset(_ARM_TOOLCHAIN_CANDIDATES)
endif()

if(ARM_TOOLCHAIN_PATH)
    list(APPEND CMAKE_PROGRAM_PATH "${ARM_TOOLCHAIN_PATH}")
    message(STATUS "ARM toolchain: ${ARM_TOOLCHAIN_PATH}")
endif()

find_program(CMAKE_C_COMPILER   NAMES ${TOOLCHAIN_PREFIX}gcc     REQUIRED)
find_program(CMAKE_CXX_COMPILER NAMES ${TOOLCHAIN_PREFIX}g++     REQUIRED)
find_program(CMAKE_ASM_COMPILER NAMES ${TOOLCHAIN_PREFIX}gcc     REQUIRED)
find_program(CMAKE_LINKER       NAMES ${TOOLCHAIN_PREFIX}g++     REQUIRED)
find_program(CMAKE_OBJCOPY      NAMES ${TOOLCHAIN_PREFIX}objcopy REQUIRED)
find_program(CMAKE_OBJDUMP      NAMES ${TOOLCHAIN_PREFIX}objdump REQUIRED)
find_program(CMAKE_SIZE         NAMES ${TOOLCHAIN_PREFIX}size    REQUIRED)
find_program(CMAKE_AR           NAMES ${TOOLCHAIN_PREFIX}ar      REQUIRED)
find_program(CMAKE_RANLIB       NAMES ${TOOLCHAIN_PREFIX}ranlib  REQUIRED)
find_program(CMAKE_STRIP        NAMES ${TOOLCHAIN_PREFIX}strip)

set(CMAKE_EXECUTABLE_SUFFIX_ASM ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_C   ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_CXX ".elf")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(TARGET_FLAGS "-mcpu=cortex-m0plus -mthumb -mfloat-abi=soft ")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${TARGET_FLAGS}")
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS} -x assembler-with-cpp -MMD -MP")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fdata-sections -ffunction-sections")

set(CMAKE_C_FLAGS_DEBUG "-Og -g3")
set(CMAKE_C_FLAGS_RELEASE "-Os -g0")
set(CMAKE_CXX_FLAGS_DEBUG "-Og -g3")
set(CMAKE_CXX_FLAGS_RELEASE "-Os -g0")

set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fno-rtti -fno-exceptions -fno-threadsafe-statics")

set(CMAKE_EXE_LINKER_FLAGS "${TARGET_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T \"${CMAKE_SOURCE_DIR}/cw32l012_flash.ld\"")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --specs=nano.specs --specs=nosys.specs")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map=${CMAKE_PROJECT_NAME}.map -Wl,--gc-sections")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--print-memory-usage")
set(TOOLCHAIN_LINK_LIBRARIES "m")
