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
#ifdef TARGET_LPC4078

#include "../../inc/MarlinConfig.h"
#include "../../core/millis_t.h"
#include "../../sd/cardreader.h"

#include <mcu_interface.h>
#include <driver/usb_device.h>

TERN_(POSTMORTEM_DEBUGGING, extern void install_min_serial());

void MarlinHAL::init() {

  MCUI::pwm_init();

  // Init LEDs
  #if PIN_EXISTS(LED)
    SET_DIR_OUTPUT(LED_PIN);
    WRITE_PIN_CLR(LED_PIN);
    #if PIN_EXISTS(LED2)
      SET_DIR_OUTPUT(LED2_PIN);
      WRITE_PIN_CLR(LED2_PIN);
      #if PIN_EXISTS(LED3)
        SET_DIR_OUTPUT(LED3_PIN);
        WRITE_PIN_CLR(LED3_PIN);
        #if PIN_EXISTS(LED4)
          SET_DIR_OUTPUT(LED4_PIN);
          WRITE_PIN_CLR(LED4_PIN);
        #endif
      #endif
    #endif

    // Flash status LED 3 times to indicate Marlin has started booting
    for(int i = 0; i < 6; ++i) {
      TOGGLE(LED_PIN);
      delay(100);
    }
  #endif

  // Init Servo Pins
  #define INIT_SERVO(N) OUT_WRITE(SERVO##N##_PIN, LOW)
  #if HAS_SERVO_0
    INIT_SERVO(0);
  #endif
  #if HAS_SERVO_1
    INIT_SERVO(1);
  #endif
  #if HAS_SERVO_2
    INIT_SERVO(2);
  #endif
  #if HAS_SERVO_3
    INIT_SERVO(3);
  #endif

  #if PIN_EXISTS(SD_SS)
    OUT_WRITE(SD_SS_PIN, HIGH);
  #endif

  #if PIN_EXISTS(ONBOARD_SD_CS) && ONBOARD_SD_CS_PIN != SD_SS_PIN
    OUT_WRITE(ONBOARD_SD_CS_PIN, HIGH);
  #endif

  TERN_(HAS_SD_HOST_DRIVE, MCUI::USBDevice::msc_storage_init()); // Enable USB SD card access
  MCUI::USBDevice::connect(2000, LED_PIN);

  HAL_timer_init();

  TERN_(POSTMORTEM_DEBUGGING, install_min_serial()); // Install the min serial handler
}

// HAL idle task
void MarlinHAL::idletask() {
  #if HAS_SHARED_MEDIA
    // If Marlin is using the SD card we need to lock it to prevent access from
    // a PC via USB.
    // Other HALs use IS_SD_PRINTING() and IS_SD_FILE_OPEN() to check for access but
    // this will not reliably detect delete operations. To be safe we will lock
    // the disk if Marlin has it mounted. Unfortunately there is currently no way
    // to unmount the disk from the LCD menu.
    // if (IS_SD_PRINTING() || IS_SD_FILE_OPEN())
    if (card.isMounted())
      MCUI::USBDevice::msc_storage_lock();
    else
      MCUI::USBDevice::msc_storage_release();
  #endif
  // Perform USB stack housekeeping
  MCUI::USBDevice::msc_run_deferred_commands();
}

#endif // TARGET_LPC4078
