#ifndef CC1101_H
#define CC1101_H

#include <Arduino.h>
#include <SPI.h>

class CC1101 {
public:
    // Constructor: CS pin, GDO0 pin, and optional SPIClass pointer (default SPI)
    CC1101(uint8_t csPin, uint8_t gdo0Pin, SPIClass *spi = &SPI);

    // Initialize SPI and CC1101 module
    bool begin();

    // Reset CC1101 chip
    void reset();

    // Send a command strobe to CC1101 (like SIDLE, STX, SRX)
    uint8_t command(uint8_t cmd);

    // Write a single byte to a CC1101 register
    void writeReg(uint8_t addr, uint8_t value);

    // Read a single byte from a CC1101 register
    uint8_t readReg(uint8_t addr);

    // Write multiple bytes to CC1101 TX FIFO
    void writeTXFIFO(const uint8_t *data, uint8_t length);

    // Read multiple bytes from CC1101 RX FIFO
    void readRXFIFO(uint8_t *data, uint8_t length);

    // Start RX mode
    void startReceive();

    // Start TX mode
    void startTransmit();

    // Check if GDO0 pin is HIGH (used for sync, packet received etc)
    bool isGDO0High();

private:
    uint8_t _csPin;
    uint8_t _gdo0Pin;
    SPIClass *_spi;

    void select();
    void deselect();

    uint8_t spiTransfer(uint8_t data);

    // Basic config registers — add your own based on your needs
    void configCC1101();
};

#endif
