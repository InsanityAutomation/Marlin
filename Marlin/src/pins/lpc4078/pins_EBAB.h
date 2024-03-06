/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#pragma once

#ifndef BOARD_INFO_NAME
  #define BOARD_INFO_NAME "Extra Big Ass Board"
#endif

#ifndef DEFAULT_MACHINE_NAME
  #define DEFAULT_MACHINE_NAME "XtraBigAssBoard"
#endif

#define LED_PIN                                P3_18 // PWM0_3

// EXTRA PINS
#define FIL_RUNOUT_PIN P1_12

// DRIVERS EXTRA PINS
#define DRIVERS_SCK                            P1_20
#define DRIVERS_MISO                           P1_23
#define DRIVERS_MOSI                           P1_24


//
// Servos
//
#define SERVO0_PIN                             P2_05 // PWM1_6
#define SERVO1_PIN                             P2_04 // PWM1_5
#define SERVO2_PIN                             P2_03 // PWM1_4
#define SERVO3_PIN                             P2_02 // PWM1_3


//
// Limit Switches
//
#define X_MIN_PIN                              P0_19
#define X_MAX_PIN                              P0_18
#define Y_MIN_PIN                              P0_20
#define Y_MAX_PIN                              P0_17
#define Z_MIN_PIN                              P0_21
#define Z_MAX_PIN                              P0_15

//
// Z Probe (when not Z_MIN_PIN)
//
#ifndef Z_MIN_PROBE_PIN
  #define Z_MIN_PROBE_PIN                     P0_21
#endif

//
// Steppers
//
#define X_STEP_PIN                            P0_10
#define X_DIR_PIN                             P4_03
#define X_ENABLE_PIN                          P0_01
#define X_CS_PIN                              P0_00
#define X_DIAG_PIN                            P2_17

#define Y_STEP_PIN                            P1_29
#define Y_DIR_PIN                             P2_14
#define Y_ENABLE_PIN                          P1_28
#define Y_CS_PIN                              P2_22
#define Y_DIAG_PIN                            P2_16

#define Z_STEP_PIN                            P4_02
#define Z_DIR_PIN                             P1_26
#define Z_ENABLE_PIN                          P1_25
#define Z_CS_PIN                              P4_01
#define Z_DIAG_PIN                            P2_21

#define E0_STEP_PIN                           P1_19
#define E0_DIR_PIN                            P0_14
#define E0_ENABLE_PIN                         P1_22
#define E0_CS_PIN                             P4_00
#define E0_DIAG_PIN                           P2_20

#define E1_STEP_PIN                           P2_23
#define E1_DIR_PIN                            P2_18
#define E1_ENABLE_PIN                         P3_23
#define E1_CS_PIN                             P1_18
#define E1_DIAG_PIN                           P2_19

#define E2_STEP_PIN                           P3_26
#define E2_DIR_PIN                            P2_25
#define E2_ENABLE_PIN                         P3_25
#define E2_CS_PIN                             P3_24
#define E2_DIAG_PIN                           P2_26

#define E3_STEP_PIN                           P4_19
#define E3_DIR_PIN                            P4_20
#define E3_ENABLE_PIN                         P4_26
#define E3_CS_PIN                             P4_21
#define E3_DIAG_PIN                           P0_22

#define E4_STEP_PIN                           P4_17
#define E4_DIR_PIN                            P4_18
#define E4_ENABLE_PIN                         P4_05
#define E4_CS_PIN                             P2_12
#define E4_DIAG_PIN                           P2_11

#define E5_STEP_PIN                           P0_11
#define E5_DIR_PIN                            P2_15
#define E5_ENABLE_PIN                         P4_04
#define E5_CS_PIN                             P4_16
#define E5_DIAG_PIN                           P2_13

#define E6_STEP_PIN                           P2_28
#define E6_DIR_PIN                            P0_28
#define E6_ENABLE_PIN                         P0_31
#define E6_CS_PIN                             P0_27
#define E6_DIAG_PIN                           P2_24

#define E7_STEP_PIN                           P1_30
#define E7_DIR_PIN                            P0_12
#define E7_ENABLE_PIN                         P0_13
#define E7_CS_PIN                             P2_29
#define E7_DIAG_PIN                           P2_27

#define E8_STEP_PIN                           P3_15
#define E8_DIR_PIN                            P3_07
#define E8_ENABLE_PIN                         P2_30
#define E8_CS_PIN                             P5_01
#define E8_DIAG_PIN                           P2_31


#if HAS_TMC_UART
  #define X_SERIAL_TX_PIN  P0_00
  #define X_SERIAL_RX_PIN  P0_00
  #define Y_SERIAL_TX_PIN  P2_22
  #define Y_SERIAL_RX_PIN  P2_22
  #define Z_SERIAL_TX_PIN  P4_01
  #define Z_SERIAL_RX_PIN  P4_01
  #define E0_SERIAL_TX_PIN P4_00
  #define E0_SERIAL_RX_PIN P4_00
  #define E1_SERIAL_TX_PIN P1_18
  #define E1_SERIAL_RX_PIN P1_18
  #define E2_SERIAL_TX_PIN P3_24
  #define E2_SERIAL_RX_PIN P3_24
  #define E3_SERIAL_TX_PIN P4_21
  #define E3_SERIAL_RX_PIN P4_21
  #define E4_SERIAL_TX_PIN P2_12
  #define E4_SERIAL_RX_PIN P2_12
  #define E5_SERIAL_TX_PIN P4_16
  #define E5_SERIAL_RX_PIN P4_16
  #define E6_SERIAL_TX_PIN P0_27
  #define E6_SERIAL_RX_PIN P0_27
  #define E7_SERIAL_TX_PIN P2_29
  #define E7_SERIAL_RX_PIN P2_29
  #define E8_SERIAL_TX_PIN P5_01
  #define E8_SERIAL_RX_PIN P5_01

  // Reduce baud rate to improve software serial reliability
  #define TMC_BAUD_RATE 19200
#endif // HAS_TMC_UART

//
// Temperature Sensors
//
#define TEMP_0_PIN                            P0_23
#define TEMP_1_PIN                            P0_24
#define TEMP_2_PIN                            P0_25
#define TEMP_3_PIN                            P0_26
#define TEMP_BED_PIN                          P1_31

//
// Heaters / Fans
//

#define HEATER_0_PIN                          P1_02
#define HEATER_1_PIN                          P1_10
#define HEATER_2_PIN                          P4_30
#define HEATER_3_PIN                          P1_09
#define HEATER_BED_PIN                        P4_23

#define FAN0_PIN                              P3_08
#define FAN1_PIN                              P3_00
#define FAN2_PIN                              P3_27 //PWM1_4
#define FAN3_PIN                              P5_04

#define EFAN0_PIN                             P1_08
#define EFAN1_PIN                             P4_31
#define EFAN2_PIN                             P3_01
#define EFAN3_PIN                             P3_10
//
// Misc. Functions
//
#if SD_CONNECTION_IS(LCD)
  #define SD_DETECT_PIN                       P3_02
  #define SD_SCK_PIN                          P1_00
  #define SD_MISO_PIN                         P1_04
  #define SD_MOSI_PIN                         P1_01
  #define SD_SS_PIN                           P3_03
  #define SDSS                            SD_SS_PIN
#elif SD_CONNECTION_IS(ONBOARD)
  #define SD_DETECT_PIN                        P_NC
  #define SD_SCK_PIN                          P0_07
  #define SD_MISO_PIN                         P0_08
  #define SD_MOSI_PIN                         P0_09
  #define ONBOARD_SD_CS_PIN                   P0_06
  #define SD_SS_PIN               ONBOARD_SD_CS_PIN
  #define SDSS                            SD_SS_PIN
#endif

#ifndef NEOPIXEL_PIN
  #define NEOPIXEL_PIN                        P_NC
#endif

#ifndef FILWIDTH_PIN
  #define FILWIDTH_PIN                        P_NC
#endif

#ifndef FIL_RUNOUT_PIN
  #define FIL_RUNOUT_PIN                      P_NC
#endif

#ifndef PS_ON_PIN
  //#define PS_ON_PIN                           P_NC
#endif

//
// Průša i3 MK2 Multiplexer Support
//
#if HAS_PRUSA_MMU1
  #ifndef E_MUX0_PIN
    #define E_MUX0_PIN                        P_NC
  #endif
  #ifndef E_MUX1_PIN
    #define E_MUX1_PIN                        P_NC
  #endif
  #ifndef E_MUX2_PIN
    #define E_MUX2_PIN                        P_NC
  #endif
#endif

/**
 * Default pins for TMC software SPI
 */
#if ENABLED(TMC_USE_SW_SPI)
  #ifndef TMC_SPI_MOSI
    #define TMC_SPI_MOSI                       P1_24
  #endif
  #ifndef TMC_SPI_MISO
    #define TMC_SPI_MISO                       P1_23
  #endif
  #ifndef TMC_SPI_SCK
    #define TMC_SPI_SCK                        P1_20
  #endif
#endif


//////////////////////////
// LCDs and Controllers //
//////////////////////////

#if ANY(TFT_COLOR_UI, TFT_CLASSIC_UI, TFT_LVGL_UI)

  #define TFT_A0_PIN                          43
  #define TFT_CS_PIN                          49
  #define TFT_DC_PIN                          43
  #define TFT_SCK_PIN                 SD_SCK_PIN
  #define TFT_MOSI_PIN               SD_MOSI_PIN
  #define TFT_MISO_PIN               SD_MISO_PIN
  #define LCD_USE_DMA_SPI

  #define BTN_EN1                             40
  #define BTN_EN2                             63
  #define BTN_ENC                             59
  #define BEEPER_PIN                          42

  #define TOUCH_CS_PIN                        33

  #define SD_DETECT_PIN                       41

  #define SPI_FLASH
  #if ENABLED(SPI_FLASH)
    #define SPI_DEVICE                         1
    #define SPI_FLASH_SIZE             0x1000000  // 16MB
    #define SPI_FLASH_CS_PIN                  31
    #define SPI_FLASH_MOSI_PIN       SD_MOSI_PIN
    #define SPI_FLASH_MISO_PIN       SD_MISO_PIN
    #define SPI_FLASH_SCK_PIN         SD_SCK_PIN
  #endif

  #define TFT_BUFFER_SIZE                 0xFFFF
  #ifndef TFT_DRIVER
    #define TFT_DRIVER                    ST7796
  #endif
  #ifndef TOUCH_SCREEN_CALIBRATION
    #if ENABLED(TFT_RES_320x240)
      #ifndef TOUCH_CALIBRATION_X
        #define TOUCH_CALIBRATION_X        20525
      #endif
      #ifndef TOUCH_CALIBRATION_Y
        #define TOUCH_CALIBRATION_Y        15335
      #endif
      #ifndef TOUCH_OFFSET_X
        #define TOUCH_OFFSET_X                -1
      #endif
      #ifndef TOUCH_OFFSET_Y
        #define TOUCH_OFFSET_Y                 0
      #endif
    #elif ENABLED(TFT_RES_480x272)
      #ifndef TOUCH_CALIBRATION_X
        #define TOUCH_CALIBRATION_X        30715
      #endif
      #ifndef TOUCH_CALIBRATION_Y
        #define TOUCH_CALIBRATION_Y        17415
      #endif
      #ifndef TOUCH_OFFSET_X
        #define TOUCH_OFFSET_X                 0
      #endif
      #ifndef TOUCH_OFFSET_Y
        #define TOUCH_OFFSET_Y                -1
      #endif
    #elif ENABLED(TFT_RES_480x320)
      #ifndef TOUCH_CALIBRATION_X
        #define TOUCH_CALIBRATION_X        30595
      #endif
      #ifndef TOUCH_CALIBRATION_Y
        #define TOUCH_CALIBRATION_Y        20415
      #endif
      #ifndef TOUCH_OFFSET_X
        #define TOUCH_OFFSET_X                 2
      #endif
      #ifndef TOUCH_OFFSET_Y
        #define TOUCH_OFFSET_Y                 1
      #endif
    #elif ENABLED(TFT_RES_1024x600)
      #ifndef TOUCH_CALIBRATION_X
        #define TOUCH_CALIBRATION_X        65533
      #endif
      #ifndef TOUCH_CALIBRATION_Y
        #define TOUCH_CALIBRATION_Y        38399
      #endif
      #ifndef TOUCH_OFFSET_X
        #define TOUCH_OFFSET_X                 2
      #endif
      #ifndef TOUCH_OFFSET_Y
        #define TOUCH_OFFSET_Y                 1
      #endif
    #endif
  #endif

  #define BTN_BACK                            70

#elif HAS_WIRED_LCD

  //
  // LCD Display output pins
  //
  #if ENABLED(REPRAPWORLD_GRAPHICAL_LCD)

    #define LCD_PINS_RS                       P3_06  // CS chip select /SS chip slave select
    #define LCD_PINS_EN                       P3_14  // SID (MOSI)
    #define LCD_PINS_D4                       P3_30  // SCK (CLK) clock

  #elif ALL(IS_NEWPANEL, PANEL_ONE)

    #define LCD_PINS_RS                       40
    #define LCD_PINS_EN                       42
    #define LCD_PINS_D4                       65
    #define LCD_PINS_D5                       66
    #define LCD_PINS_D6                       44
    #define LCD_PINS_D7                       64

  #else

    #if ENABLED(CR10_STOCKDISPLAY)

      #define LCD_PINS_RS                     27
      #define LCD_PINS_EN                     29
      #define LCD_PINS_D4                     25

      #if !IS_NEWPANEL
        #define BEEPER_PIN                    37
      #endif

    #elif ENABLED(ZONESTAR_LCD)

      #define LCD_PINS_RS                     64
      #define LCD_PINS_EN                     44
      #define LCD_PINS_D4                     63
      #define LCD_PINS_D5                     40
      #define LCD_PINS_D6                     42
      #define LCD_PINS_D7                     65

    #else

      #if ANY(MKS_12864OLED, MKS_12864OLED_SSD1306)
        #define LCD_PINS_DC                   25  // Set as output on init
        #define LCD_PINS_RS                   27  // Pull low for 1s to init
        // DOGM SPI LCD Support
        #define DOGLCD_CS                     16
        #define DOGLCD_MOSI                   17
        #define DOGLCD_SCK                    23
        #define DOGLCD_A0            LCD_PINS_DC
      #else
        #define LCD_PINS_RS                   P3_06
        #define LCD_PINS_EN                   P3_14
        #define LCD_PINS_D4                   P3_30
        #define LCD_PINS_D5                   P3_05
        #define LCD_PINS_D6                   P3_29
      #endif

      #define LCD_PINS_D7                     P5_00

      #if !IS_NEWPANEL
        #define BEEPER_PIN                    33
      #endif

    #endif

    #if !IS_NEWPANEL
      // Buttons attached to a shift register
      // Not wired yet
      //#define SHIFT_CLK_PIN                 38
      //#define SHIFT_LD_PIN                  42
      //#define SHIFT_OUT_PIN                 40
      //#define SHIFT_EN_PIN                  17
    #endif

  #endif

  //
  // LCD Display input pins
  //
  #if IS_NEWPANEL

    #if ENABLED(REPRAP_DISCOUNT_SMART_CONTROLLER)

      #define BEEPER_PIN                      P3_31

      #if ENABLED(CR10_STOCKDISPLAY)
        #define BTN_EN1                       17
        #define BTN_EN2                       23
      #else
        #define BTN_EN1                       P3_11
        #define BTN_EN2                       P3_12
      #endif

      #define BTN_ENC                         P3_28
      #define SD_DETECT_PIN                   P3_02
      //#define KILL_PIN                        P_NC

      #if ENABLED(BQ_LCD_SMART_CONTROLLER)
        #define LCD_BACKLIGHT_PIN             39
      #endif

    #elif ENABLED(REPRAPWORLD_GRAPHICAL_LCD)

      #define BTN_EN1                         64
      #define BTN_EN2                         59
      #define BTN_ENC                         63
      #define SD_DETECT_PIN                   42

    #elif ENABLED(LCD_I2C_PANELOLU2)

      #define BTN_EN1                         47
      #define BTN_EN2                         43
      #define BTN_ENC                         32
      #define LCD_SDSS                      SDSS
      #define KILL_PIN                        41

    #elif ENABLED(LCD_I2C_VIKI)

      #define BTN_EN1                         22  // https://files.panucatt.com/datasheets/viki_wiring_diagram.pdf explains 40/42.
      #define BTN_EN2                          7  // 22/7 are unused on RAMPS_14. 22 is unused and 7 the SERVO0_PIN on RAMPS_13.
      #define BTN_ENC                         -1

      #define LCD_SDSS                      SDSS
      #define SD_DETECT_PIN                   49

    #elif ANY(VIKI2, miniVIKI)

      #define DOGLCD_CS                       45
      #define DOGLCD_A0                       44

      #define BEEPER_PIN                      33
      #define STAT_LED_RED_PIN                32
      #define STAT_LED_BLUE_PIN               35

      #define BTN_EN1                         22
      #define BTN_EN2                          7
      #define BTN_ENC                         39

      #define SD_DETECT_PIN                   -1  // Pin 49 for display sd interface, 72 for easy adapter board
      #define KILL_PIN                        31

      #define LCD_SCREEN_ROTATE              180  // 0, 90, 180, 270

    #elif ENABLED(ELB_FULL_GRAPHIC_CONTROLLER)

      #define DOGLCD_CS                       29
      #define DOGLCD_A0                       27

      #define BEEPER_PIN                      23
      #define LCD_BACKLIGHT_PIN               33

      #define BTN_EN1                         35
      #define BTN_EN2                         37
      #define BTN_ENC                         31

      #define LCD_SDSS                      SDSS
      #define SD_DETECT_PIN                   49
      #define KILL_PIN                        41

    #elif ENABLED(MKS_MINI_12864)

      #define DOGLCD_A0                       27
      #define DOGLCD_CS                       25

      #define BEEPER_PIN                      37
      // not connected to a pin
      #define LCD_BACKLIGHT_PIN               65  // backlight LED on A11/D65

      #define BTN_EN1                         31
      #define BTN_EN2                         33
      #define BTN_ENC                         35

      #define SD_DETECT_PIN                   49
      #define KILL_PIN                        64

      //#define LCD_SCREEN_ROTATE            180  // 0, 90, 180, 270

    #elif ENABLED(MINIPANEL)

      #define BEEPER_PIN                      42
      // not connected to a pin
      #define LCD_BACKLIGHT_PIN               65  // backlight LED on A11/D65

      #define DOGLCD_A0                       44
      #define DOGLCD_CS                       66

      #define BTN_EN1                         40
      #define BTN_EN2                         63
      #define BTN_ENC                         59

      #define SD_DETECT_PIN                   49
      #define KILL_PIN                        64

      //#define LCD_SCREEN_ROTATE            180  // 0, 90, 180, 270

    #elif ENABLED(ZONESTAR_LCD)

      #define ADC_KEYPAD_PIN                  12

    #elif ENABLED(AZSMZ_12864)

      // Pins only defined for RAMPS_SMART currently

    #else

      // Beeper on AUX-4
      #define BEEPER_PIN                      P3_31

      // Buttons are directly attached to AUX-2
      #if IS_RRW_KEYPAD
        #define SHIFT_OUT_PIN                 40
        #define SHIFT_CLK_PIN                 44
        #define SHIFT_LD_PIN                  42
        #define BTN_EN1                       64
        #define BTN_EN2                       59
        #define BTN_ENC                       63
      #elif ENABLED(PANEL_ONE)
        #define BTN_EN1                       59  // AUX2 PIN 3
        #define BTN_EN2                       63  // AUX2 PIN 4
        #define BTN_ENC                       49  // AUX3 PIN 7
      #else
        #define BTN_EN1                       P3_11
        #define BTN_EN2                       P3_12
        #define BTN_ENC                       P3_28
      #endif

      #if ENABLED(G3D_PANEL)
        #define SD_DETECT_PIN                 49
        #define KILL_PIN                      41
      #endif
    #endif

    // CUSTOM SIMULATOR INPUTS
    #define BTN_BACK                          70

  #endif // IS_NEWPANEL

#endif // HAS_WIRED_LCD
