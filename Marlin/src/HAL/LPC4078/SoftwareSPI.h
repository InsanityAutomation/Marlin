#include <mcu_interface.h>

uint8_t swSpiTransfer(uint8_t b, const uint8_t spi_speed, const pin_t sck_pin, const pin_t miso_pin, const pin_t mosi_pin);
void swSpiBegin(const pin_t sck_pin, const pin_t miso_pin, const pin_t mosi_pin);
uint8_t swSpiInit(const uint8_t spiRate, const pin_t sck_pin, const pin_t mosi_pin);
