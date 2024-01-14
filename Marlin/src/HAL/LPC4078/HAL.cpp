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
#include "../shared/Delay.h"
#include "../../../gcode/parser.h"

DefaultSerial1 USBSerial(false, MCDCSerial0);

uint32_t MarlinHAL::adc_result = 0;

// U8glib required functions
extern "C" {
  void u8g_xMicroDelay(uint16_t val) { DELAY_US(val); }
  void u8g_MicroDelay()              { u8g_xMicroDelay(1); }
  void u8g_10MicroDelay()            { u8g_xMicroDelay(10); }
  void u8g_Delay(uint16_t val)       { delay(val); }
}

// return free heap space
int freeMemory() {
  char stack_end;
  void *heap_start = malloc(sizeof(uint32_t));
  if (heap_start == 0) return 0;

  uint32_t result = (uint32_t)&stack_end - (uint32_t)heap_start;
  free(heap_start);
  return result;
}

void MarlinHAL::reboot() { MCUCore::nvic_system_reset(); }

uint8_t MarlinHAL::get_reset_source() {
  #if ENABLED(USE_WATCHDOG)
    if (watchdog_timed_out()) return RST_WATCHDOG;
  #endif
  return RST_POWER_ON;
}

void MarlinHAL::clear_reset_source() { watchdog_clear_timeout_flag(); }

void flashFirmware(const int16_t) {
  delay(500);          // Give OS time to disconnect
  //USB_Connect(false);  // USB clear connection
  delay(1000);         // Give OS time to notice
  hal.reboot();
}

#if ENABLED(USE_WATCHDOG)

  #define WDT_TIMEOUT TERN(WATCHDOG_DURATION_8S, 8.0f, 4.0f) // 4 or 8 second timeout

  void MarlinHAL::watchdog_init() {
    #if ENABLED(WATCHDOG_RESET_MANUAL)
      // We enable the watchdog timer, but only for the interrupt.

      // Configure WDT to only trigger an interrupt
      // Disable WDT interrupt (just in case, to avoid triggering it!)
      NVIC_DisableIRQ(WDT_IRQn);

      // We NEED memory barriers to ensure Interrupts are actually disabled!
      // ( https://dzone.com/articles/nvic-disabling-interrupts-on-arm-cortex-m-and-the )
      __DSB();
      __ISB();

      // WDT defaults to trigger an interrupt

      // Configure and enable WDT interrupt.
      NVIC_ClearPendingIRQ(WDT_IRQn);
      NVIC_SetPriority(WDT_IRQn, 0); // Use highest priority, so we detect all kinds of lockups
      NVIC_EnableIRQ(WDT_IRQn);
    #else
      MCUI::watchdog_set_timeout_triggers_reset();
    #endif
    MCUI::watchdog_set_timeout_in_seconds(WDT_TIMEOUT);
    MCUI::watchdog_enable();
  }

  void MarlinHAL::watchdog_refresh() {
    MCUI::watchdog_feed();
    #if DISABLED(PINS_DEBUGGING) && PIN_EXISTS(LED)
      static millis_t next_flash = millis();
      if (ELAPSED(millis(), next_flash)) {
        next_flash = millis() + 200;
        TOGGLE(LED_PIN);  // heartbeat indicator
      }
    #endif
  }

  // Timeout state
  bool MarlinHAL::watchdog_timed_out() { return MCUI::watchdog_has_triggered(); }
  void MarlinHAL::watchdog_clear_timeout_flag() { MCUI::watchdog_clear_timeout_flag(); }

#endif // USE_WATCHDOG

// For M42/M43, scan command line for pin code
//   return index into pin map array if found and the pin is valid.
//   return dval if not found or not a valid pin.
int16_t PARSED_PIN_INDEX(const char code, const int16_t dval) {
  const uint16_t val = (uint16_t)parser.intval(code, -1), port = val / 100, pin = val % 100;
  const  int16_t ind = (port < ((NUM_DIGITAL_PINS) >> 5) && pin < 32) ? ((port << 5) | pin) : -2;
  return ind > -1 ? ind : dval;
}

#endif // TARGET_LPC4078
