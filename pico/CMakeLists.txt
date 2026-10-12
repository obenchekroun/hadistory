cmake_minimum_required(VERSION 3.13)

# ------------------------------------------------------------------------------
# Build-time configuration
#   cmake -S pico -B pico/build -DPICO_BOARD=pico2_w -DSTORAGE=SDCARD
#   cmake -S pico -B pico/build -DPICO_BOARD=pimoroni_pico_plus2_rp2350 -DSTORAGE=FLASH
#   add -DWITH_GPT=ON -DWIFI_SSID=... -DWIFI_PASSWORD=... -DOPENAI_API_KEY=... for the AI mode
# Secrets can also go in pico/secrets.cmake (ignored by git), see secrets.cmake.example
# ------------------------------------------------------------------------------

# Board must be chosen before the SDK is initialised.
# Known to fit: pico2_w (RP2350A, wireless), pimoroni_pico_plus2_rp2350 (RP2350B, 16 MB flash, PSRAM, no wireless).
if(NOT PICO_BOARD)
    set(PICO_BOARD pico2_w)
endif()
set(PICO_PLATFORM rp2350)

option(WITH_GPT "Build the AI mode (OpenAI over WiFi). Requires a board with wireless." OFF)
set(STORAGE "SDCARD" CACHE STRING "Where the stories live: SDCARD (FAT microSD) or FLASH (embedded in the firmware)")
set_property(CACHE STORAGE PROPERTY STRINGS SDCARD FLASH)

include(${CMAKE_CURRENT_LIST_DIR}/secrets.cmake OPTIONAL)
set(WIFI_SSID "" CACHE STRING "WiFi SSID (AI mode)")
set(WIFI_PASSWORD "" CACHE STRING "WiFi password (AI mode)")
set(OPENAI_API_KEY "" CACHE STRING "OpenAI API key (AI mode)")

# Include the Pico SDK initialization script
include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)
include($ENV{PICO_SDK_PATH}/../pimoroni-pico/pimoroni_pico_import.cmake)

project(hadistory C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the Pico SDK
pico_sdk_init()

set(REPO_ROOT ${CMAKE_CURRENT_LIST_DIR}/..)

set(hadistory_SRC
    main.c
    src/led.c
    src/buttons.c
    src/display.c
    src/story.c
)
set(my_lib
    pico_stdlib
    pico_rand
    hardware_spi
    hardware_pwm
)

# ---------------------------------------------------------------- storage backend
if(STORAGE STREQUAL "SDCARD")
    # Pimoroni drivers used from C: SD card over PIO SPI, and ChaN FatFs
    include(drivers/sdcard/sdcard)
    include(drivers/fatfs/fatfs)
    list(APPEND hadistory_SRC src/storage_sdcard.c)
    list(APPEND my_lib hardware_pio sdcard fatfs)
    set(STORAGE_DEFINE HADISTORY_STORAGE_SDCARD=1)
elseif(STORAGE STREQUAL "FLASH")
    # Pages are rendered by tools/render_pages.py at build time and embedded with .incbin.
    # Needs python3 with Pillow and numpy. Re-runs when the stories or the script change.
    set(EMBED_DIR ${CMAKE_BINARY_DIR}/embedded_stories)
    file(GLOB_RECURSE STORY_SOURCES CONFIGURE_DEPENDS
         ${REPO_ROOT}/stories/*/txt/*.txt ${REPO_ROOT}/stories/*/img/*.png)
    add_custom_command(
        OUTPUT ${EMBED_DIR}/stories_data.S ${EMBED_DIR}/stories_table.c
        COMMAND python3 ${REPO_ROOT}/tools/render_pages.py
                --out ${CMAKE_BINARY_DIR}/rendered_pages --embed ${EMBED_DIR}
        DEPENDS ${REPO_ROOT}/tools/render_pages.py ${STORY_SOURCES}
        COMMENT "Rendering stories for embedding in flash"
        VERBATIM
    )
    list(APPEND hadistory_SRC src/storage_flash.c ${EMBED_DIR}/stories_data.S ${EMBED_DIR}/stories_table.c)
    list(APPEND my_lib hardware_flash hardware_sync)
    set(STORAGE_DEFINE HADISTORY_STORAGE_FLASH=1)
else()
    message(FATAL_ERROR "STORAGE must be SDCARD or FLASH (got '${STORAGE}')")
endif()

# ---------------------------------------------------------------- AI mode
if(WITH_GPT)
    if(NOT PICO_CYW43_SUPPORTED)
        message(FATAL_ERROR "WITH_GPT=ON needs a board with wireless (e.g. -DPICO_BOARD=pico2_w)")
    endif()
    list(APPEND hadistory_SRC src/gpt.c)
    list(APPEND my_lib pico_cyw43_arch_lwip_threadsafe_background)
endif()

add_executable(${PROJECT_NAME} ${hadistory_SRC})

target_include_directories(${PROJECT_NAME} PRIVATE src)

# hadistory_config.h is force-included in every translation unit so that the pin
# definitions it holds also reach the pimoroni sdcard driver (SDCARD_PIN_SPI0_*).
target_compile_options(${PROJECT_NAME} PRIVATE -include ${CMAKE_CURRENT_LIST_DIR}/src/hadistory_config.h -Wall)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    HADISTORY_WITH_GPT=$<BOOL:${WITH_GPT}>
    ${STORAGE_DEFINE}
)
if(WITH_GPT)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        WIFI_SSID=\"${WIFI_SSID}\"
        WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
        OPENAI_API_KEY=\"${OPENAI_API_KEY}\"
    )
endif()

target_link_libraries(${PROJECT_NAME} ${my_lib})

# enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(${PROJECT_NAME})
