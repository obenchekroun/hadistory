cmake_minimum_required(VERSION 3.13)

# ------------------------------------------------------------------------------
# Build-time configuration
#   cmake -S pico -B pico/build -DPICO_BOARD=pico2_w -DSTORAGE=SDCARD
#   cmake -S pico -B pico/build -DPICO_BOARD=pimoroni_pico_plus2_rp2350 -DSTORAGE=FLASH
#   cmake -S pico -B pico/build -DPICO_BOARD=waveshare_rp2350b_plus_w -DSTORAGE=FLASH   (16 MB flash; board header in pico/boards, SDK 2.1.1 or later)
#   add -DEMBED_STORIES="story_1;story_2" to embed a selection only (FLASH), default is all of stories/
#   add -DEMBED_QUALITY=70 to shrink the embedded JPEGs (FLASH), default 85
#   add -DWITH_GPT=ON -DOPENAI_API_KEY=... -DWIFI_SSID=... -DWIFI_PASSWORD=... for the AI mode, or several networks
#   with -DWIFI_NETWORKS="ssid1;password1;ssid2;password2" (tried in that order)
#   add -DSTORY_DIAL=KY040 or -DSTORY_DIAL=AS5600 for the story dial (ON means KY040)
#   add -DSTATUS_LED=ONBOARD to use the board's own LED instead of the external one
#   add -DDEEP_SLEEP=OFF to keep the board awake (keeps the USB serial console alive, for debugging)
#   add -DBATTERY_GAUGE=OFF to drop the battery bars (VSYS reading, see README "Battery gauge")
# Secrets can also go in pico/secrets.cmake (ignored by git), see secrets.cmake.example
# ------------------------------------------------------------------------------

# Board must be chosen before the SDK is initialised.
# Known to fit: pico2_w (RP2350A, wireless), pimoroni_pico_plus2_rp2350 (RP2350B, 16 MB flash, PSRAM, no wireless),
# waveshare_rp2350b_plus_w (RP2350B, 16 MB flash, wireless; header in pico/boards, SDK 2.1.1 or later).
if(NOT PICO_BOARD)
    set(PICO_BOARD pico2_w)
endif()
set(PICO_PLATFORM rp2350)

option(WITH_GPT "Build the AI mode (OpenAI over WiFi). Requires a board with wireless." OFF)
option(TLS_NO_VERIFY "AI mode: skip the TLS certificate check of api.openai.com (debug only, exposes the API key)" OFF)
option(DEEP_SLEEP "Power the core off between pages in story mode (RP2350 power manager), any button reboots the board." ON)
option(BATTERY_GAUGE "Battery bars on each page from the VSYS reading, display only." ON)
set(STORY_DIAL "OFF" CACHE STRING "Story dial choosing the story by number: OFF, KY040 (incremental encoder with detents and push button; ON means KY040) or AS5600 (absolute magnetic angle sensor on I2C)")
set_property(CACHE STORY_DIAL PROPERTY STRINGS OFF KY040 AS5600)
set(STATUS_LED "EXTERNAL" CACHE STRING "Status LED: EXTERNAL (LED_PIN in hadistory_config.h) or ONBOARD (the board's own LED)")
set_property(CACHE STATUS_LED PROPERTY STRINGS EXTERNAL ONBOARD)
set(STORAGE "SDCARD" CACHE STRING "Where the stories live: SDCARD (FAT microSD) or FLASH (embedded in the firmware)")
set_property(CACHE STORAGE PROPERTY STRINGS SDCARD FLASH)
set(EMBED_STORIES "" CACHE STRING "STORAGE=FLASH only: stories to embed, e.g. \"story_1;story_2\". Empty = all of stories/")
set(EMBED_QUALITY "85" CACHE STRING "STORAGE=FLASH only: JPEG quality 1-95 of the embedded illustrations (size vs look)")

if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/secrets.cmake)
    set(SECRETS_FOUND "secrets.cmake read")
    include(${CMAKE_CURRENT_LIST_DIR}/secrets.cmake)
else()
    set(SECRETS_FOUND "no secrets.cmake (see secrets.cmake.example)")
endif()
set(WIFI_SSID "" CACHE STRING "WiFi SSID (AI mode): a single network, tried first; see WIFI_NETWORKS for more")
set(WIFI_PASSWORD "" CACHE STRING "WiFi password (AI mode)")
set(WIFI_NETWORKS "" CACHE STRING "WiFi networks (AI mode), a list of ssid;password pairs tried in this order")
set(OPENAI_API_KEY "" CACHE STRING "OpenAI API key (AI mode)")

# Boards the SDK does not know: their headers live in pico/boards (waveshare_rp2350b_plus_w)
list(APPEND PICO_BOARD_HEADER_DIRS ${CMAKE_CURRENT_LIST_DIR}/boards)

# Include the Pico SDK initialization script
include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)
include($ENV{PICO_SDK_PATH}/../pimoroni-pico/pimoroni_pico_import.cmake)

project(hadistory C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the Pico SDK
pico_sdk_init()

# The radio of the RP2350B-Plus-W is on GPIO36 to 39: SDK 2.1.0 only knows the Pico W pins
if(PICO_BOARD STREQUAL "waveshare_rp2350b_plus_w" AND PICO_SDK_VERSION_STRING VERSION_LESS "2.1.1")
    message(FATAL_ERROR "PICO_BOARD=waveshare_rp2350b_plus_w needs Pico SDK 2.1.1 or later (relocatable wireless pins), this one is ${PICO_SDK_VERSION_STRING}")
endif()

set(REPO_ROOT ${CMAKE_CURRENT_LIST_DIR}/..)

# Pimoroni display stack: PicoGraphics frame buffer, the UC8159 (Inky Impression 5.7") driver
# and the JPEG decoder used for every page illustration
include(libraries/pico_graphics/pico_graphics)
include(drivers/uc8159/uc8159)
include(libraries/jpegdec/jpegdec)

set(hadistory_SRC
    main.cpp
    src/led.cpp
    src/buttons.cpp
    src/display.cpp
    src/render.cpp
    src/story.cpp
)
set(my_lib
    pico_stdlib
    pico_rand
    hardware_spi
    hardware_pwm
    pico_graphics
    uc8159
    jpegdec
)

# ---------------------------------------------------------------- storage backend
if(STORAGE STREQUAL "SDCARD")
    # Pimoroni drivers used from C: SD card over PIO SPI, and ChaN FatFs
    include(drivers/sdcard/sdcard)
    include(drivers/fatfs/fatfs)
    list(APPEND hadistory_SRC src/storage_sdcard.cpp)
    list(APPEND my_lib hardware_pio sdcard fatfs)
    set(STORAGE_DEFINE HADISTORY_STORAGE_SDCARD=1)
elseif(STORAGE STREQUAL "FLASH")
    # Pages are prepared by tools/prepare_pages.py at build time (JPEG + text) and embedded
    # with .incbin. Needs python3 with Pillow. Re-runs when the stories or the script change.
    set(EMBED_DIR ${CMAKE_BINARY_DIR}/embedded_stories)
    file(GLOB_RECURSE STORY_SOURCES CONFIGURE_DEPENDS
         ${REPO_ROOT}/stories/*/txt/*.txt ${REPO_ROOT}/stories/*/img/*.png)
    # Optional selection: -DEMBED_STORIES="story_1;story_2" -> --story story_1 --story story_2
    set(EMBED_STORY_ARGS --quality ${EMBED_QUALITY})
    foreach(story IN LISTS EMBED_STORIES)
        list(APPEND EMBED_STORY_ARGS --story ${story})
    endforeach()
    if(EMBED_STORIES)
        message(STATUS "Embedding stories: ${EMBED_STORIES} (JPEG quality ${EMBED_QUALITY})")
    else()
        message(STATUS "Embedding all stories from ${REPO_ROOT}/stories (JPEG quality ${EMBED_QUALITY})")
    endif()
    # Selection and quality are recorded in a stamp file so that changing them re-runs the preparation
    file(WRITE ${CMAKE_BINARY_DIR}/embed_stories_selection.txt "${EMBED_STORIES} q${EMBED_QUALITY}")
    add_custom_command(
        OUTPUT ${EMBED_DIR}/stories_data.S ${EMBED_DIR}/stories_table.c
        COMMAND python3 ${REPO_ROOT}/tools/prepare_pages.py
                --out ${CMAKE_BINARY_DIR}/prepared_pages --embed ${EMBED_DIR} ${EMBED_STORY_ARGS}
        DEPENDS ${REPO_ROOT}/tools/prepare_pages.py ${STORY_SOURCES} ${CMAKE_BINARY_DIR}/embed_stories_selection.txt
        COMMENT "Preparing stories for embedding in flash"
        VERBATIM
    )
    list(APPEND hadistory_SRC src/storage_flash.cpp ${EMBED_DIR}/stories_data.S ${EMBED_DIR}/stories_table.c)
    list(APPEND my_lib hardware_flash hardware_sync)
    set(STORAGE_DEFINE HADISTORY_STORAGE_FLASH=1)
else()
    message(FATAL_ERROR "STORAGE must be SDCARD or FLASH (got '${STORAGE}')")
endif()

# ---------------------------------------------------------------- status LED
if(STATUS_LED STREQUAL "ONBOARD")
    set(LED_DEFINE HADISTORY_LED_ONBOARD=1)
    # On the Pico W family the onboard LED hangs off the wireless chip: the cyw43 driver is
    # needed even without the AI mode (pico_cyw43_arch_none, no network stack).
    if(PICO_CYW43_SUPPORTED AND NOT WITH_GPT)
        list(APPEND my_lib pico_cyw43_arch_none)
    endif()
elseif(STATUS_LED STREQUAL "EXTERNAL")
    set(LED_DEFINE HADISTORY_LED_ONBOARD=0)
else()
    message(FATAL_ERROR "STATUS_LED must be EXTERNAL or ONBOARD (got '${STATUS_LED}')")
endif()

# ---------------------------------------------------------------- deep sleep
if(DEEP_SLEEP)
    list(APPEND hadistory_SRC src/power.cpp)
    list(APPEND my_lib hardware_powman)
endif()

# ---------------------------------------------------------------- battery gauge
if(BATTERY_GAUGE)
    list(APPEND hadistory_SRC src/battery.cpp)
    list(APPEND my_lib hardware_adc)
endif()

# ---------------------------------------------------------------- story dial
string(TOUPPER "${STORY_DIAL}" STORY_DIAL_KIND)
if(STORY_DIAL_KIND MATCHES "^(ON|TRUE|YES|Y|1|KY040|KY-040)$")
    set(STORY_DIAL_KIND KY040)
    list(APPEND hadistory_SRC src/dial.cpp)
    set(DIAL_DEFINES HADISTORY_STORY_DIAL=1 HADISTORY_DIAL_KY040=1 HADISTORY_DIAL_AS5600=0)
elseif(STORY_DIAL_KIND STREQUAL "AS5600")
    list(APPEND hadistory_SRC src/dial_as5600.cpp)
    list(APPEND my_lib hardware_i2c)
    set(DIAL_DEFINES HADISTORY_STORY_DIAL=1 HADISTORY_DIAL_KY040=0 HADISTORY_DIAL_AS5600=1)
elseif(STORY_DIAL_KIND MATCHES "^(OFF|FALSE|NO|N|0|)$")
    set(STORY_DIAL_KIND OFF)
    set(DIAL_DEFINES HADISTORY_STORY_DIAL=0 HADISTORY_DIAL_KY040=0 HADISTORY_DIAL_AS5600=0)
else()
    message(FATAL_ERROR "STORY_DIAL must be OFF, KY040 (or ON) or AS5600 (got '${STORY_DIAL}')")
endif()

# ---------------------------------------------------------------- AI mode
if(WITH_GPT)
    if(NOT PICO_CYW43_SUPPORTED)
        message(FATAL_ERROR "WITH_GPT=ON needs a board with wireless (e.g. -DPICO_BOARD=pico2_w)")
    endif()
    list(APPEND hadistory_SRC src/wifi.cpp src/gpt.cpp src/https.cpp src/json.cpp src/prompt.cpp src/base64.cpp)

    # Known networks -> generated/wifi_networks.h: WIFI_SSID / WIFI_PASSWORD first, then the
    # WIFI_NETWORKS pairs, as the initialiser of an array of { ssid, password } C strings
    # (a semicolon inside a name or password is written \; in WIFI_NETWORKS; it is kept aside while the list is split)
    string(REPLACE "\\;" "@SEMI@" WIFI_STR "${WIFI_NETWORKS}")
    set(WIFI_LIST ${WIFI_STR})
    if(WIFI_SSID)
        string(REPLACE ";" "@SEMI@" WIFI_S1 "${WIFI_SSID}")
        string(REPLACE ";" "@SEMI@" WIFI_P1 "${WIFI_PASSWORD}")
        set(WIFI_LIST "${WIFI_S1}" "${WIFI_P1}" ${WIFI_STR})
    endif()
    list(LENGTH WIFI_LIST WIFI_ITEMS)
    math(EXPR WIFI_ODD "${WIFI_ITEMS} % 2")
    if(WIFI_ODD)
        message(FATAL_ERROR "WIFI_NETWORKS must be ssid;password pairs (got ${WIFI_ITEMS} items)")
    endif()
    math(EXPR WIFI_COUNT "${WIFI_ITEMS} / 2")
    set(WIFI_INIT "")
    set(WIFI_I 0)
    while(WIFI_I LESS WIFI_ITEMS)
        list(GET WIFI_LIST ${WIFI_I} WIFI_S)
        math(EXPR WIFI_J "${WIFI_I} + 1")
        list(GET WIFI_LIST ${WIFI_J} WIFI_P)
        foreach(v WIFI_S WIFI_P)
            string(REPLACE "@SEMI@" ";" ${v} "${${v}}")
            string(REPLACE "\\" "\\\\" ${v} "${${v}}")
            string(REPLACE "\"" "\\\"" ${v} "${${v}}")
        endforeach()
        string(APPEND WIFI_INIT " { \"${WIFI_S}\", \"${WIFI_P}\" },")
        math(EXPR WIFI_I "${WIFI_I} + 2")
    endwhile()
    if(WIFI_COUNT EQUAL 0)
        message(WARNING "WITH_GPT=ON but no WiFi network configured: set WIFI_NETWORKS (or WIFI_SSID / WIFI_PASSWORD) in secrets.cmake")
        set(WIFI_INIT " { \"\", \"\" },")
    endif()
    file(WRITE ${CMAKE_BINARY_DIR}/generated/wifi_networks.h.in
        "// Generated by CMake from secrets.cmake, do not edit\n#define WIFI_NETWORKS_COUNT ${WIFI_COUNT}\n#define WIFI_NETWORKS_INIT {${WIFI_INIT} }\n")
    configure_file(${CMAKE_BINARY_DIR}/generated/wifi_networks.h.in ${CMAKE_BINARY_DIR}/generated/wifi_networks.h COPYONLY)
    list(APPEND my_lib pico_cyw43_arch_lwip_threadsafe_background pico_lwip_mbedtls pico_mbedtls)
    # prompts/prompts.txt embedded as a NUL-terminated string (hadistory_prompts_txt)
    set(PROMPTS_ASM ${CMAKE_BINARY_DIR}/prompts_data.S)
    file(WRITE ${PROMPTS_ASM}
"    .section .rodata.hadistory_prompts, \"a\"
    .global hadistory_prompts_txt
hadistory_prompts_txt:
    .incbin \"${REPO_ROOT}/prompts/prompts.txt\"
    .byte 0
")
    set_source_files_properties(${PROMPTS_ASM} PROPERTIES OBJECT_DEPENDS ${REPO_ROOT}/prompts/prompts.txt)
    list(APPEND hadistory_SRC ${PROMPTS_ASM})
    if(TLS_NO_VERIFY)
        set(TLS_DEFINES HADISTORY_TLS_NO_VERIFY=1)
    else()
        # Verify the server against the roots in src/openai_roots.h, and fail if it does not match
        set(TLS_DEFINES HADISTORY_TLS_NO_VERIFY=0 ALTCP_MBEDTLS_AUTHMODE=MBEDTLS_SSL_VERIFY_REQUIRED)
    endif()
    # Ignore a warning from lwip code: lwIP's own file (mbedtls 2.28, SDK 2.1.x) and the SDK's patched
    # copy of it, which replaces it with mbedtls 3 (SDK 2.2 and later)
    set_source_files_properties(${PICO_LWIP_PATH}/src/apps/altcp_tls/altcp_tls_mbedtls.c
                                ${PICO_SDK_PATH}/src/rp2_common/pico_lwip/altcp_tls_mbedtls.c
                                PROPERTIES COMPILE_OPTIONS "-Wno-unused-result")
endif()

add_executable(${PROJECT_NAME} ${hadistory_SRC})

target_include_directories(${PROJECT_NAME} PRIVATE src ${CMAKE_BINARY_DIR}/generated)

# hadistory_config.h is force-included in every translation unit so that the pin
# definitions it holds also reach the pimoroni sdcard driver (SDCARD_PIN_SPI0_*).
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<COMPILE_LANGUAGE:C,CXX>:-include ${CMAKE_CURRENT_LIST_DIR}/src/hadistory_config.h>
    -Wall
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    # The USB console drops the rest of a long line when the host has not drained its
    # buffer within this delay (SDK default 0.5 s): long prompts and stories were cut
    PICO_STDIO_USB_STDOUT_TIMEOUT_US=2000000
    HADISTORY_WITH_GPT=$<BOOL:${WITH_GPT}>
    HADISTORY_DEEP_SLEEP=$<BOOL:${DEEP_SLEEP}>
    HADISTORY_BATTERY=$<BOOL:${BATTERY_GAUGE}>
    ${DIAL_DEFINES}
    ${STORAGE_DEFINE}
    ${LED_DEFINE}
    ${TLS_DEFINES}
)
if(WITH_GPT)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        OPENAI_API_KEY=\"${OPENAI_API_KEY}\"
    )
endif()

target_link_libraries(${PROJECT_NAME} ${my_lib})

# ---------------------------------------------------------------- summary
# What this build is, so that the console of a cmake run says where it goes
message(STATUS "hadistory build:")
message(STATUS "  PICO_BOARD     ${PICO_BOARD} (Pico SDK ${PICO_SDK_VERSION_STRING})")
if(STORAGE STREQUAL "FLASH")
    if(EMBED_STORIES)
        set(EMBED_WHAT "${EMBED_STORIES}")
    else()
        set(EMBED_WHAT "all of stories/")
    endif()
    message(STATUS "  STORAGE        FLASH, stories embedded: ${EMBED_WHAT}, EMBED_QUALITY ${EMBED_QUALITY}")
else()
    message(STATUS "  STORAGE        SDCARD")
endif()
message(STATUS "  STORY_DIAL     ${STORY_DIAL_KIND}")
message(STATUS "  DEEP_SLEEP     ${DEEP_SLEEP}")
message(STATUS "  BATTERY_GAUGE  ${BATTERY_GAUGE}")
message(STATUS "  STATUS_LED     ${STATUS_LED}")
if(WITH_GPT)
    if(OPENAI_API_KEY)
        set(KEY_WHAT "set")
    else()
        set(KEY_WHAT "MISSING")
    endif()
    set(NET_WHAT "")
    set(WIFI_I 0)
    while(WIFI_I LESS WIFI_ITEMS)
        list(GET WIFI_LIST ${WIFI_I} WIFI_S)
        string(REPLACE "@SEMI@" ";" WIFI_S "${WIFI_S}")
        string(APPEND NET_WHAT " \"${WIFI_S}\"")
        math(EXPR WIFI_I "${WIFI_I} + 2")
    endwhile()
    if(NOT NET_WHAT)
        set(NET_WHAT " NONE")
    endif()
    message(STATUS "  WITH_GPT       ON, ${SECRETS_FOUND}, OPENAI_API_KEY ${KEY_WHAT}, TLS_NO_VERIFY ${TLS_NO_VERIFY}")
    message(STATUS "  WIFI_NETWORKS  ${WIFI_COUNT}, in order:${NET_WHAT}")
else()
    message(STATUS "  WITH_GPT       OFF (${SECRETS_FOUND}, not needed)")
endif()

# enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(${PROJECT_NAME})
