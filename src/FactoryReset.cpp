#include "FactoryReset.h"

FactoryResetHandler::FactoryResetHandler(ConfigManager& config, uint8_t buttonPin)
    : _config(config),
      _buttonPin(buttonPin),
      _lastButtonState(HIGH),
      _pressStartTime(0),
      _resetTriggered(false) {}

void FactoryResetHandler::setup() {
    // Configure the button pin as an input with an internal pull-up resistor.
    // When the button is pressed, it will connect the pin to ground (LOW).
    pinMode(_buttonPin, INPUT_PULLUP);
}

void FactoryResetHandler::loop() {
    // If a reset has already been triggered, do nothing further.
    if (_resetTriggered) {
        return;
    }

    int currentState = digitalRead(_buttonPin);

    if (currentState == LOW && _lastButtonState == HIGH) {
        // Button was just pressed. Record the start time.
        _pressStartTime = millis();
        Serial.println("Reset button pressed. Hold for 10 seconds to reset.");
    } else if (currentState == LOW && _lastButtonState == LOW) {
        // Button is still being held down.
        if (millis() - _pressStartTime > LONG_PRESS_DURATION_MS) {
            Serial.println("Factory reset triggered!");

            // 1. Tell the config manager to erase settings and save defaults.
            _config.resetToDefaults();

            _resetTriggered = true; // Prevent this from running again

            // Optional: Provide visual feedback, like blinking the built-in LED
            pinMode(LED_BUILTIN, OUTPUT);
            for (int i = 0; i < 10; i++) {
                digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
                delay(100);
            }

            Serial.println("Reset complete. Restarting device...");
            delay(1000); // Wait for serial message to send

            // 2. Restart the device to apply the new default settings.
            ESP.restart();
        }
    } else if (currentState == HIGH && _lastButtonState == LOW) {
        // Button was released.
        Serial.println("Reset button released too early.");
        _pressStartTime = 0;
    }

    _lastButtonState = currentState;
}
