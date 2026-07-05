set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# MSYS2 ucrt64 ARM toolchain (Windows)
set(TOOLCHAIN_PREFIX c:/msys64/ucrt64)
set(CMAKE_C_COMPILER    ${TOOLCHAIN_PREFIX}/bin/arm-none-eabi-gcc.exe)
set(CMAKE_CXX_COMPILER  ${TOOLCHAIN_PREFIX}/bin/arm-none-eabi-g++.exe)
set(CMAKE_ASM_COMPILER  ${TOOLCHAIN_PREFIX}/bin/arm-none-eabi-gcc.exe)
set(CMAKE_OBJCOPY       ${TOOLCHAIN_PREFIX}/bin/arm-none-eabi-objcopy.exe)
set(CMAKE_SIZE          ${TOOLCHAIN_PREFIX}/bin/arm-none-eabi-size.exe)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Cortex-M3 compile flags
set(CPU_FLAGS "-mcpu=cortex-m3 -mthumb")
set(CMAKE_C_FLAGS   "${CPU_FLAGS} -std=gnu11 -Wall -Wextra -Werror -fdata-sections -ffunction-sections" CACHE STRING "")
set(CMAKE_ASM_FLAGS "${CPU_FLAGS}" CACHE STRING "")
set(CMAKE_EXE_LINKER_FLAGS
    "${CPU_FLAGS} -specs=nosys.specs -Wl,--gc-sections -Wl,-Map=${CMAKE_PROJECT_NAME}.map"
    CACHE STRING ""
)
