cmake_minimum_required(VERSION 3.20)
project(f103-oled-test C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ── Sources ────────────────────────────────────────────────────
set(SRCS
    src/startup_stm32f103c8tx.c
    src/system_stm32f1xx.c
    src/ssd1306.c
    src/main.c
)

# ── Headers ────────────────────────────────────────────────────
set(INCLUDES
    inc
)

# ── Executable ─────────────────────────────────────────────────
add_executable(${CMAKE_PROJECT_NAME} ${SRCS})

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${INCLUDES})

target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
    -T${CMAKE_SOURCE_DIR}/ld/STM32F103C8TX_FLASH.ld
)

# ── Post-build: generate .bin and .hex ────────────────────────
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary
        $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
        ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.bin
    COMMAND ${CMAKE_OBJCOPY} -O ihex
        $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
        ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.hex
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
)
