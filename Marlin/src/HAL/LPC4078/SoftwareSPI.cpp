#ifdef TARGET_LPC4078

#include <Arduino.h>
#include "SoftwareSPI.h"

#include <mcu_interface.h>

uint8_t swSpiTransfer(uint8_t b, const uint8_t spi_speed, const pin_t sck_pin, const pin_t miso_pin, const pin_t mosi_pin) {
  for (uint8_t i = 0; i < 8; i++) {
    if (spi_speed == 0) {
      MCUI::gpio_set(mosi_pin, !!(b & 0x80));
      MCUI::gpio_set(sck_pin, HIGH);
      b <<= 1;
      if (miso_pin >= 0 && MCUI::gpio_get(miso_pin)) b |= 1;
      MCUI::gpio_set(sck_pin, LOW);
    }
    else {
      const uint8_t state = (b & 0x80) ? HIGH : LOW;
      for (uint8_t j = 0; j < spi_speed; j++)
        MCUI::gpio_set(mosi_pin, state);

      for (uint8_t j = 0; j < spi_speed + (miso_pin >= 0 ? 0 : 1); j++)
        MCUI::gpio_set(sck_pin, HIGH);

      b <<= 1;
      if (miso_pin >= 0 && MCUI::gpio_get(miso_pin)) b |= 1;

      for (uint8_t j = 0; j < spi_speed; j++)
        MCUI::gpio_set(sck_pin, LOW);
    }
  }
  return b;
}

void swSpiBegin(const pin_t sck_pin, const pin_t miso_pin, const pin_t mosi_pin) {
  pinMode(sck_pin, OUTPUT);
  if (MCUI::pin_is_valid(miso_pin)) pinMode(miso_pin, INPUT);
  pinMode(mosi_pin, OUTPUT);
}

uint8_t swSpiInit(const uint8_t spiRate, const pin_t sck_pin, const pin_t mosi_pin) {
  MCUI::gpio_set(mosi_pin, HIGH);
  MCUI::gpio_set(sck_pin, LOW);
  return (SystemCoreClock == 120000000 ? 44 : 38) / std::pow(2, 6 - std::min(spiRate, (uint8_t)6));
}

#endif // TARGET_LPC4078
