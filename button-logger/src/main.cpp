#include <Arduino.h>

// Watches three push buttons and reports each press over USB serial as
//   PRESS <colour> <millis since boot>
// Each button connects its pin to 3V3, so a pressed button reads HIGH.

struct Button {
  const char *name;
  uint8_t pin;
  bool stable;          // debounced level: true = pressed (HIGH)
  bool lastRead;        // most recent raw reading
  uint32_t changedAt;   // when the raw reading last changed
};

static Button buttons[] = {
  {"red", 5},
  {"yellow", 4},
  {"blue", 3},
};

static const uint32_t DEBOUNCE_MS = 20;

void setup() {
  Serial.begin(115200);
  for (Button &b : buttons) {
    pinMode(b.pin, INPUT_PULLDOWN);
    b.stable = b.lastRead = digitalRead(b.pin);
    b.changedAt = millis();
  }
  delay(1000);
  // Report the idle level of each pin; every button should read LOW when
  // untouched. A HIGH here means a button is stuck or wired differently.
  Serial.print("READY");
  for (Button &b : buttons) {
    Serial.printf(" %s=%s", b.name, b.stable ? "HIGH" : "LOW");
  }
  Serial.println();
}

void loop() {
  uint32_t now = millis();
  for (Button &b : buttons) {
    bool level = digitalRead(b.pin);
    if (level != b.lastRead) {
      b.lastRead = level;
      b.changedAt = now;
    } else if (level != b.stable && now - b.changedAt >= DEBOUNCE_MS) {
      b.stable = level;
      if (level) {
        Serial.printf("PRESS %s %lu\n", b.name, (unsigned long)now);
      }
    }
  }
  delay(1);
}
