#include <Arduino.h>
#include <SPI.h>
#include <CC1101.h>

// Define pins for CC1101 (change according to your wiring)
#define CC1101_CS   5
#define CC1101_GDO0 4

CC1101 radio(CC1101_CS, CC1101_GDO0);

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Starting CC1101...");
  if (radio.begin()) {
    Serial.println("CC1101 initialized");
  } else {
    Serial.println("Failed to initialize CC1101");
  }

  radio.startReceive();
}

void loop() {
  if (radio.isGDO0High()) {
    Serial.println("Packet received!");
    uint8_t len = radio.readReg(0x3F);  // RX FIFO bytes available
    if (len > 0 && len <= 64) {
      uint8_t buffer[64];
      radio.readRXFIFO(buffer, len);
      Serial.print("Received data: ");
      for (uint8_t i = 0; i < len; i++) {
        Serial.print(buffer[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
    }
    // Flush RX FIFO after reading
    radio.command(0x3A); // SFRX
    radio.startReceive();
  }
}
