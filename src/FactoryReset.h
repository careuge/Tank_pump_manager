#ifndef FACTORY_RESET_H
#define FACTORY_RESET_H

#include "Config.h"
#include <Arduino.h>

// Define how long the button must be pressed to trigger a reset (in milliseconds)
const unsigned long LONG_PRESS_DURATION_MS = 10000; // 10 seconds

class FactoryResetHandler {
public:
    // Constructor takes a reference to the ConfigManager and the button pin.
    FactoryResetHandler(ConfigManager& config, uint8_t buttonPin);

    // Sets up the button pin.
    void setup();

    // Checks the button state. To be called in the main loop.
    void loop();

private:
    ConfigManager& _config;
    uint8_t _buttonPin;

    int _lastButtonState;
    unsigned long _pressStartTime;
    bool _resetTriggered;
};

#endif // FACTORY_RESET_H
