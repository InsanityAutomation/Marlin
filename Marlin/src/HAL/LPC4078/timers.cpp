/**
 * Marlin 3D Printer Firmware
 *
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 * Copyright (c) 2016 Bob Cousins bobcousins42@googlemail.com
 * Copyright (c) 2015-2016 Nico Tonnhofer wurstnase.reprap@gmail.com
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

/**
 * Description:
 *
 * Timers for LPC1768
 */

#ifdef TARGET_LPC4078

#include "../../inc/MarlinConfig.h"

void HAL_timer_init() {
  SBI(MCUI::system_control.PCONP, SBIT_TIMER0);  // Power ON Timer 0
  STEP_TIMER_REF.PR = (HAL_TIMER_RATE) / (STEPPER_TIMER_RATE) - 1; // Use prescaler to set frequency if needed

  SBI(MCUI::system_control.PCONP, SBIT_TIMER1);  // Power ON Timer 1
  TEMP_TIMER_REF.PR = (HAL_TIMER_RATE) / 1000000 - 1;
}

void HAL_timer_start(const uint8_t timer_num, const uint32_t frequency) {
  switch (timer_num) {
    case MF_TIMER_STEP:
      STEP_TIMER_REF.MCR = _BV(SBIT_MR0I) | _BV(SBIT_MR0R); // Match on MR0, reset on MR0, interrupts when NVIC enables them
      STEP_TIMER_REF.MR0 = uint32_t(STEPPER_TIMER_RATE) / frequency; // Match value (period) to set frequency
      STEP_TIMER_REF.TCR = _BV(SBIT_CNTEN); // Counter Enable

      MCUCore::nvic_set_priority(MCUI::IRQNumber::TIMER0, MCUCore::nvic_encode_priority(0, 1, 0));
      MCUCore::nvic_enable_irq(MCUI::IRQNumber::TIMER0);
      break;

    case MF_TIMER_TEMP:
      TEMP_TIMER_REF.MCR = _BV(SBIT_MR0I) | _BV(SBIT_MR0R); // Match on MR0, reset on MR0, interrupts when NVIC enables them
      TEMP_TIMER_REF.MR0 = uint32_t(TEMP_TIMER_RATE) / frequency;
      TEMP_TIMER_REF.TCR = _BV(SBIT_CNTEN); // Counter Enable

      MCUCore::nvic_set_priority(MCUI::IRQNumber::TIMER1, MCUCore::nvic_encode_priority(0, 2, 0));
      MCUCore::nvic_enable_irq(MCUI::IRQNumber::TIMER1);
      break;

    default: break;
  }
}

#endif // TARGET_LPC4078
