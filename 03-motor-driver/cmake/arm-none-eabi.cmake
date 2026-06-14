set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(TOOLCHAIN_PREFIX $ENV{HOME}/.local/arm-gnu-toolchain-14.2.rel1-darwin-arm64-arm-none-eabi)
set(CMAKE_C_COMPILER    ${TOOLCHAIN_PREFIX}/bin/arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER  ${TOOLCHAIN_PREFIX}/bin/arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER  ${TOOLCHAIN_PREFIX}/bin/arm-none-eabi-gcc)
set(CMAKE_OBJCOPY       ${TOOLCHAIN_PREFIX}/bin/arm-none-eabi-objcopy)
set(CMAKE_SIZE          ${TOOLCHAIN_PREFIX}/bin/arm-none-eabi-size)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Cortex-M3 compile flags (no -specs here, only linker needs them)
set(CPU_FLAGS "-mcpu=cortex-m3 -mthumb")
set(CMAKE_C_FLAGS   "${CPU_FLAGS} -std=gnu11 -Wall -Wextra -Werror -fdata-sections -ffunction-sections" CACHE STRING "")
set(CMAKE_ASM_FLAGS "${CPU_FLAGS}" CACHE STRING "")
set(CMAKE_EXE_LINKER_FLAGS
    "${CPU_FLAGS} -specs=nosys.specs -Wl,--gc-sections -Wl,-Map=${CMAKE_PROJECT_NAME}.map"
    CACHE STRING ""
)
