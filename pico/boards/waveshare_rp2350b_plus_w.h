/*
 * Board header for the Waveshare RP2350B-Plus-W, which the Pico SDK does not ship (checked up to
 * 2.3.1 and the develop branch, 2026-11): RP2350B (48 GPIOs), 16 MB flash, 520 KB RAM, no PSRAM,
 * Raspberry Pi Radio Module 2 (CYW43439) on GPIO36 to 39, USB-C, Pico 2 W outline.
 * Found by the build through PICO_BOARD_HEADER_DIRS (pico/CMakeLists.txt):
 *     cmake -S pico -B pico/build-ws -DPICO_BOARD=waveshare_rp2350b_plus_w ...
 * It needs Pico SDK 2.1.1 or later: 2.1.0 has the wireless pins fixed on GPIO23 / 24 / 25 / 29.
 *
 * Sources: the Waveshare pinout and schematic (https://www.waveshare.com/wiki/RP2350B-Plus-W),
 * as transcribed by the arduino-pico variant waveshare_rp2350b_plus_w. NOT TESTED ON THE BOARD.
 *
 * Header pins 1 to 29 carry the same GPIOs as the Pico 2 W (GP0 to GP22). Pins 31, 32 and 34 are
 * GP40, GP41 and GP42 (ADC0 to ADC2) where the Pico has GP26 to GP28; GP24 to GP35 and GP43 to
 * GP45 are on pads under the board.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// -----------------------------------------------------
// NOTE: THIS HEADER IS ALSO INCLUDED BY ASSEMBLER SO
//       SHOULD ONLY CONSIST OF PREPROCESSOR DIRECTIVES
// -----------------------------------------------------

#ifndef _BOARDS_WAVESHARE_RP2350B_PLUS_W_H
#define _BOARDS_WAVESHARE_RP2350B_PLUS_W_H

// CMake reads these lines as text: the comment form for SDK 2.1.x, the macro form for 2.2 and later
// pico_cmake_set PICO_PLATFORM = rp2350
// pico_cmake_set PICO_CYW43_SUPPORTED = 1
// pico_cmake_set_default PICO_FLASH_SIZE_BYTES = (16 * 1024 * 1024)
// pico_cmake_set_default PICO_RP2350_A2_SUPPORTED = 1
#ifdef pico_board_cmake_set
pico_board_cmake_set(PICO_PLATFORM, rp2350)
pico_board_cmake_set(PICO_CYW43_SUPPORTED, 1)
pico_board_cmake_set_default(PICO_FLASH_SIZE_BYTES, (16 * 1024 * 1024))
pico_board_cmake_set_default(PICO_RP2350_A2_SUPPORTED, 1)
#endif

// For board detection
#define WAVESHARE_RP2350B_PLUS_W

// --- RP2350 VARIANT ---
#define PICO_RP2350A 0

// --- UART ---
#ifndef PICO_DEFAULT_UART
#define PICO_DEFAULT_UART 0
#endif
#ifndef PICO_DEFAULT_UART_TX_PIN
#define PICO_DEFAULT_UART_TX_PIN 0
#endif
#ifndef PICO_DEFAULT_UART_RX_PIN
#define PICO_DEFAULT_UART_RX_PIN 1
#endif

// --- LED ---
// LED2 of the board, a plain GPIO (PWM capable). LED1 hangs off the radio module (CYW43_WL_GPIO_LED_PIN).
#ifndef PICO_DEFAULT_LED_PIN
#define PICO_DEFAULT_LED_PIN 23
#endif

// --- I2C ---
#ifndef PICO_DEFAULT_I2C
#define PICO_DEFAULT_I2C 0
#endif
#ifndef PICO_DEFAULT_I2C_SDA_PIN
#define PICO_DEFAULT_I2C_SDA_PIN 4
#endif
#ifndef PICO_DEFAULT_I2C_SCL_PIN
#define PICO_DEFAULT_I2C_SCL_PIN 5
#endif

// --- SPI ---
#ifndef PICO_DEFAULT_SPI
#define PICO_DEFAULT_SPI 0
#endif
#ifndef PICO_DEFAULT_SPI_SCK_PIN
#define PICO_DEFAULT_SPI_SCK_PIN 18
#endif
#ifndef PICO_DEFAULT_SPI_TX_PIN
#define PICO_DEFAULT_SPI_TX_PIN 19
#endif
#ifndef PICO_DEFAULT_SPI_RX_PIN
#define PICO_DEFAULT_SPI_RX_PIN 16
#endif
#ifndef PICO_DEFAULT_SPI_CSN_PIN
#define PICO_DEFAULT_SPI_CSN_PIN 17
#endif

// --- FLASH ---
#define PICO_BOOT_STAGE2_CHOOSE_W25Q080 1

#ifndef PICO_FLASH_SPI_CLKDIV
#define PICO_FLASH_SPI_CLKDIV 2
#endif

#ifndef PICO_FLASH_SIZE_BYTES
#define PICO_FLASH_SIZE_BYTES (16 * 1024 * 1024)
#endif

#ifndef PICO_RP2350_A2_SUPPORTED
#define PICO_RP2350_A2_SUPPORTED 1
#endif

// --- VSYS ---
// VSYS sense on GPIO46 (ADC6), a pin of its own: unlike the Pico W family it is not shared with
// the wireless clock, so CYW43_USES_VSYS_PIN stays undefined. Divider assumed 3:1 as on the Pico.
#ifndef PICO_VSYS_PIN
#define PICO_VSYS_PIN 46
#endif

// --- CYW43 (Raspberry Pi Radio Module 2) ---
// gpio pin to power up the cyw43 chip
#ifndef CYW43_DEFAULT_PIN_WL_REG_ON
#define CYW43_DEFAULT_PIN_WL_REG_ON 36
#endif
// one data line: spi data out, data in and the irq line share it
#ifndef CYW43_DEFAULT_PIN_WL_DATA_OUT
#define CYW43_DEFAULT_PIN_WL_DATA_OUT 37
#endif
#ifndef CYW43_DEFAULT_PIN_WL_DATA_IN
#define CYW43_DEFAULT_PIN_WL_DATA_IN 37
#endif
#ifndef CYW43_DEFAULT_PIN_WL_HOST_WAKE
#define CYW43_DEFAULT_PIN_WL_HOST_WAKE 37
#endif
// gpio pin for the spi chip select to the cyw43 chip
#ifndef CYW43_DEFAULT_PIN_WL_CS
#define CYW43_DEFAULT_PIN_WL_CS 38
#endif
// gpio pin for the spi clock line to the cyw43 chip
#ifndef CYW43_DEFAULT_PIN_WL_CLOCK
#define CYW43_DEFAULT_PIN_WL_CLOCK 39
#endif

#ifndef CYW43_WL_GPIO_COUNT
#define CYW43_WL_GPIO_COUNT 3
#endif
#ifndef CYW43_WL_GPIO_LED_PIN
#define CYW43_WL_GPIO_LED_PIN 0
#endif
// CYW43_WL_GPIO_VBUS_PIN is left undefined: whether the module's GPIO2 senses VBUS here is not known

#endif
