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

#include <mcu_interface.h>
#include <interface/uart.h>
#include <driver/usb_cdc.h>

#include <UART.h>

#include "../../inc/MarlinConfigPre.h"
#if ENABLED(EMERGENCY_PARSER)
  #include "../../feature/e_parser.h"
#endif
#include "../../core/serial_hook.h"

#ifndef SERIAL_PORT
  #define SERIAL_PORT 0
#endif
#ifndef RX_BUFFER_SIZE
  #define RX_BUFFER_SIZE 128
#endif
#ifndef TX_BUFFER_SIZE
  #define TX_BUFFER_SIZE 32
#endif

class MarlinCDCSerial : public HardwareSerial {
  public:
    MarlinCDCSerial() {}
    void begin(unsigned long baudrate) override {}
    void begin(unsigned long baudrate, uint16_t config) override { }
    void end() override { }
    int available() override {
      return MCUI::CDCSerial0::available();
    }
    int peek() override { char c = 0; return MCUI::CDCSerial0::peek((uint8_t*)&c)? c : -1; }
    int read() override { char c = 0; return MCUI::CDCSerial0::read((uint8_t*)&c, 1)? c : -1; }
    void flush() override {};
    size_t write(uint8_t c) override { return write(&c, 1); }
    size_t write(const uint8_t *buffer, size_t size) override { return MCUI::CDCSerial0::connected() ? MCUI::CDCSerial0::write(buffer, size) : 0; }
    using Print::write;
    operator bool() override { return true; }
};
extern MarlinCDCSerial MCDCSerial0;

class MarlinSerial : public HardwareSerial {
  public:
    MarlinSerial(const uint32_t uart_id) : uart_device(uart_id) {}
    void begin(unsigned long baudrate) override {
      uart_device.configure_pins(P0_02, P0_03);
      uart_device.init({ .baud = baudrate });
      #if ENABLED(EMERGENCY_PARSER)
      uart_device.set_rx_callback([this](char rx_value){ return this->recv_callback(rx_value); } );
      #endif
    }
    void begin(unsigned long baudrate, uint16_t config) override {
      begin(baudrate);
    }
    void end() override { }
    int available() override {
      return uart_device.rx_available();
    }
    int peek() override { char c = 0; return uart_device.peek(&c)? c : -1; }
    int read() override { char c = 0; return uart_device.read(&c)? c : -1; }
    void flush() override {};
    size_t write(uint8_t c) override { return write(&c, 1); }
    size_t write(const uint8_t *buffer, size_t size) override { return uart_device.write((const char *)buffer, size); }
    using Print::write; // pull in write(str) and write(buf, size) from Print
    operator bool() override { return true; }

    bool recv_callback(char value);
  private:
    MCUI::BufferedUARTC uart_device;
};

// On LPC176x framework, HardwareSerial does not implement the same interface as Arduino's Serial, so overloads
// of 'available' and 'read' method are not used in this multiple inheritance scenario.
// Instead, use a ForwardSerial here that adapts the interface.
typedef ForwardSerial1Class<MarlinSerial> MSerialT;
extern MSerialT MSerial0;
extern MSerialT MSerial1;
extern MSerialT MSerial2;
extern MSerialT MSerial3;

// Consequently, we can't use a RuntimeSerial either. The workaround would be to use
// a RuntimeSerial<ForwardSerial<MarlinSerial>> type here. Ignore for now until it's actually required.
#if ENABLED(SERIAL_RUNTIME_HOOK)
  #error "SERIAL_RUNTIME_HOOK is not yet supported for LPC176x."
#endif
