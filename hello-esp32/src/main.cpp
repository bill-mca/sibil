#include <Arduino.h>

// Cycles the onboard RGB LED through red, green and blue,
// and prints a heartbeat with chip details over USB serial.

static const uint8_t colours[][3] = {
  {32, 0, 0},
  {0, 32, 0},
  {0, 0, 32},
};

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("hello from the ESP32-S3");
  Serial.printf("chip: %s rev %d, %d cores, %u MB flash\n",
                ESP.getChipModel(), ESP.getChipRevision(),
                ESP.getChipCores(), ESP.getFlashChipSize() / (1024 * 1024));
}

void loop() {
  static uint32_t tick = 0;
  const uint8_t *c = colours[tick % 3];
  neopixelWrite(RGB_BUILTIN, c[0], c[1], c[2]);
  Serial.printf("tick %lu\n", (unsigned long)tick++);
  delay(500);
}
