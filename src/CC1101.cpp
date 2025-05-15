#include "CC1101.h"

// CC1101 command strobes
#define CC1101_SRES      0x30  // Reset chip
#define CC1101_SIDLE     0x36  // Exit RX / TX, turn off frequency synthesizer
#define CC1101_STX       0x35  // Enable TX
#define CC1101_SRX       0x34  // Enable RX
#define CC1101_SFRX      0x3A  // Flush RX FIFO
#define CC1101_SFTX      0x3B  // Flush TX FIFO

// CC1101 register addresses for configuration (partial example)
#define CC1101_IOCFG0    0x02
#define CC1101_PKTLEN    0x06
#define CC1101_PKTCTRL1  0x07
#define CC1101_PKTCTRL0  0x08
#define CC1101_FSCTRL1   0x0B
#define CC1101_FREQ2     0x0D
#define CC1101_FREQ1     0x0E
#define CC1101_FREQ0     0x0F
#define CC1101_MDMCFG4   0x10
#define CC1101_MDMCFG3   0x11
#define CC1101_MDMCFG2   0x12
#define CC1101_DEVIATN   0x15
#define CC1101_MCSM0     0x18
#define CC1101_FOCCFG    0x19
#define CC1101_BSCFG     0x1A
#define CC1101_AGCCTRL2  0x1B
#define CC1101_WOREVT1   0x1E
#define CC1101_WOREVT0   0x1F
#define CC1101_WORCTRL   0x20
#define CC1101_FSCAL3    0x23
#define CC1101_FSCAL2    0x24
#define CC1101_FSCAL1    0x25
#define CC1101_FSCAL0    0x26
#define CC1101_TEST2     0x2C
#define CC1101_TEST1     0x2D
#define CC1101_TEST0     0x2E

// SPI read/write masks
#define CC1101_WRITE_SINGLE  0x00
#define CC1101_WRITE_BURST   0x40
#define CC1101_READ_SINGLE   0x80
#define CC1101_READ_BURST    0xC0

// Constructor
CC1101::CC1101(uint8_t csPin, uint8_t gdo0Pin, SPIClass *spi) 
  : _csPin(csPin), _gdo0Pin(gdo0Pin), _spi(spi) {
}

// Initialize SPI and reset/configure CC1101
bool CC1101::begin() {
    pinMode(_csPin, OUTPUT);
    digitalWrite(_csPin, HIGH);
    pinMode(_gdo0Pin, INPUT);

    // Begin SPI - SPI pins default to hardware SPI on ESP8266/ESP32
    _spi->begin();

    delay(100);

    reset();

    // Basic configuration, can be customized
    configCC1101();

    return true;
}

void CC1101::reset() {
    deselect();
    delay(1);
    select();
    delay(1);
    deselect();
    delay(1);

    select();
    spiTransfer(CC1101_SRES);
    deselect();
    delay(100);
}

void CC1101::select() {
    digitalWrite(_csPin, LOW);
}

void CC1101::deselect() {
    digitalWrite(_csPin, HIGH);
}

uint8_t CC1101::spiTransfer(uint8_t data) {
    return _spi->transfer(data);
}

uint8_t CC1101::command(uint8_t cmd) {
    select();
    uint8_t status = spiTransfer(cmd);
    deselect();
    return status;
}

void CC1101::writeReg(uint8_t addr, uint8_t value) {
    select();
    spiTransfer(addr | CC1101_WRITE_SINGLE);
    spiTransfer(value);
    deselect();
}

uint8_t CC1101::readReg(uint8_t addr) {
    select();
    spiTransfer(addr | CC1101_READ_SINGLE);
    uint8_t val = spiTransfer(0x00);
    deselect();
    return val;
}

void CC1101::writeTXFIFO(const uint8_t *data, uint8_t length) {
    select();
    spiTransfer(0x3F | CC1101_WRITE_BURST); // TX FIFO address is 0x3F
    for (uint8_t i = 0; i < length; i++) {
        spiTransfer(data[i]);
    }
    deselect();
}

void CC1101::readRXFIFO(uint8_t *data, uint8_t length) {
    select();
    spiTransfer(0x3F | CC1101_READ_BURST); // RX FIFO address is 0x3F
    for (uint8_t i = 0; i < length; i++) {
        data[i] = spiTransfer(0x00);
    }
    deselect();
}

void CC1101::startReceive() {
    command(CC1101_SRX);
}

void CC1101::startTransmit() {
    command(CC1101_STX);
}

bool CC1101::isGDO0High() {
    return digitalRead(_gdo0Pin) == HIGH;
}

// Basic config — can be extended or replaced with your own settings
void CC1101::configCC1101() {
    // Example settings for 433 MHz, 1.2kbps, etc. Adjust to your needs!

    writeReg(CC1101_IOCFG0, 0x06);     // GDO0 = RX FIFO Threshold
    writeReg(CC1101_PKTLEN, 0xFF);     // Max packet length
    writeReg(CC1101_PKTCTRL1, 0x04);   // Append status bytes to the packet
    writeReg(CC1101_PKTCTRL0, 0x05);   // Variable length, CRC enabled

    writeReg(CC1101_FSCTRL1, 0x06);    // Frequency synthesizer control

    writeReg(CC1101_FREQ2, 0x10);      // Frequency = 433.92 MHz
    writeReg(CC1101_FREQ1, 0xB0);
    writeReg(CC1101_FREQ0, 0x71);

    writeReg(CC1101_MDMCFG4, 0xF5);   // Data rate 1.2kbps, channel spacing
    writeReg(CC1101_MDMCFG3, 0x83);
    writeReg(CC1101_MDMCFG2, 0x13);   // Sync word config, Manchester enable

    writeReg(CC1101_DEVIATN, 0x15);    // Frequency deviation

    writeReg(CC1101_MCSM0, 0x18);     // Calibrate when going from idle to RX or TX
    writeReg(CC1101_FOCCFG, 0x16);
    writeReg(CC1101_BSCFG, 0x6C);
    writeReg(CC1101_AGCCTRL2, 0x43);

    writeReg(CC1101_WOREVT1, 0x87);
    writeReg(CC1101_WOREVT0, 0x6B);
    writeReg(CC1101_WORCTRL, 0xFB);

    writeReg(CC1101_FSCAL3, 0xE9);
    writeReg(CC1101_FSCAL2, 0x2A);
    writeReg(CC1101_FSCAL1, 0x00);
    writeReg(CC1101_FSCAL0, 0x1F);

    writeReg(CC1101_TEST2, 0x81);
    writeReg(CC1101_TEST1, 0x35);
    writeReg(CC1101_TEST0, 0x09);
}
