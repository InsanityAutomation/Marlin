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

/**
 * Emulate EEPROM storage using Flash Memory
 *
 * Use a single 32K flash sector to store EEPROM data. To reduce the
 * number of erase operations a simple "leveling" scheme is used that
 * maintains a number of EEPROM "slots" within the larger flash sector.
 * Each slot is used in turn and the entire sector is only erased when all
 * slots have been used.
 *
 * A simple RAM image is used to hold the EEPROM data during I/O operations
 * and this is flushed to the next available slot when an update is complete.
 * If RAM usage becomes an issue we could store this image in one of the two
 * 16Kb I/O buffers (intended to hold DMA USB and Ethernet data, but currently
 * unused).
 */
#include "../../inc/MarlinConfig.h"

#include "../shared/eeprom_api.h"

//#include <cstdio>

#ifndef MARLIN_EEPROM_SIZE
  #define MARLIN_EEPROM_SIZE 4032 // 4KB
#endif

size_t PersistentStore::capacity() { return 4032; }

constexpr uint32_t page_size = MCUI::EEPROM::page_size;
uint32_t buffered_page = 0;
uint8_t page_buffer[page_size];
bool buffered_page_dirty = false;

bool PersistentStore::access_start() {
  MCUI::EEPROM::init();
  MCUI::EEPROM::read_page(buffered_page, page_buffer, page_size);
  return true;
}

bool PersistentStore::access_finish() {
  if (buffered_page_dirty) {
    MCUI::EEPROM::write_page(buffered_page, page_buffer, page_size);
  }
  return true;
}

bool PersistentStore::write_data(int &pos, const uint8_t *value, size_t size, uint16_t *crc) {
  uint32_t address = pos;
  //printf("Marlin::eeprom::write(pos = %d, size = %d)\n", pos, size);
  uint32_t value_index = 0;
  for (uint32_t addr = address; addr < address + size; ++addr) {
    uint32_t page_address = addr & 0xFFFFFFC0;
    if (page_address != buffered_page) {
      //printf("Marlin::eeprom::write(@%ld):: page change new(%ld) != current(%d)\n",addr, page_address, buffered_page);
      if (buffered_page_dirty == true) MCUI::EEPROM::write_page(buffered_page, page_buffer, page_size);
      buffered_page = page_address;
      buffered_page_dirty = false;
      MCUI::EEPROM::read_page(buffered_page, page_buffer, page_size);
    }
    page_buffer[addr & ~0xFFFFFFC0] = value[value_index++];
    buffered_page_dirty = true;
  }

  pos += size;
  crc16(crc, value, size);

  return false;  // return true for any error
}

bool PersistentStore::read_data(int &pos, uint8_t *value, size_t size, uint16_t *crc, const bool writing/*=true*/) {
  uint32_t address = pos;
  //printf("Marlin::eeprom::read(pos = %d, size = %d, writing = %d)\n", pos, size, writing);
  uint32_t value_index = 0;
  for (uint32_t addr = address; addr < address + size; ++addr) {
    uint32_t page_address = addr & 0xFFFFFFC0;
    if (page_address != buffered_page) {
      if (buffered_page_dirty) {
        MCUI::EEPROM::write_page(buffered_page, page_buffer, page_size);
        buffered_page_dirty = false;
      }
      buffered_page = page_address;
      MCUI::EEPROM::read_page(buffered_page, page_buffer, page_size);
    }
    if (writing) value[value_index++] = page_buffer[addr & ~0xFFFFFFC0];
    else crc16(crc, &page_buffer[addr & ~0xFFFFFFC0], 1);
  }

  if (writing) crc16(crc, value, size);
  pos += size;

  return false;  // return true for any error
}

#endif // TARGET_LPC4078
