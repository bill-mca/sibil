#include <Arduino.h>

// Watches every GPIO that is safe to probe and reports when one changes,
// so we can find out which pins the buttons are really wired to.
//
// Each pin is sampled twice: once with the internal pull-up and once with the
// pull-down. An idle, unconnected pin follows whichever pull is on. A button
// to GND reads LOW even with the pull-up on; a button to 3V3 reads HIGH even
// with the pull-down on. Either counts as "pressed".
//
// Left out: 19/20 (USB), 26-37 (flash and PSRAM), 48 (RGB LED).

static const uint8_t pins[] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 21,
  38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
};
static const size_t pinCount = sizeof(pins) / sizeof(pins[0]);

// 0 = idle, 1 = pulled to GND, 2 = pulled to 3V3
static uint8_t state[pinCount];

static uint8_t sample(uint8_t pin) {
  pinMode(pin, INPUT_PULLUP);
  delayMicroseconds(50);
  bool lowWithPullup = digitalRead(pin) == LOW;
  pinMode(pin, INPUT_PULLDOWN);
  delayMicroseconds(50);
  bool highWithPulldown = digitalRead(pin) == HIGH;
  if (lowWithPullup && !highWithPulldown) return 1;
  if (highWithPulldown && !lowWithPullup) return 2;
  return 0;
}

static const char *describe(uint8_t s) {
  switch (s) {
    case 1: return "pressed (pulls to GND)";
    case 2: return "pressed (pulls to 3V3)";
    default: return "released";
  }
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  for (size_t i = 0; i < pinCount; i++) state[i] = sample(pins[i]);
  Serial.println("pin finder ready - press each button in turn");
  for (size_t i = 0; i < pinCount; i++) {
    if (state[i]) Serial.printf("GPIO %u is already %s at startup\n", pins[i], describe(state[i]));
  }
}

void loop() {
  for (size_t i = 0; i < pinCount; i++) {
    uint8_t s = sample(pins[i]);
    if (s != state[i]) {
      state[i] = s;
      Serial.printf("%8lu ms  GPIO %u %s\n", millis(), pins[i], describe(s));
    }
  }
  delay(10);
}
