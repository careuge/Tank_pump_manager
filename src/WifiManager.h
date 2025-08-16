#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "Config.h"

class WifiManager {
public:
    // Constructor requires a reference to the config manager to get credentials.
    WifiManager(ConfigManager& configManager);

    // Initializes and starts the WiFi connection process.
    void setup();

    // Handles reconnection logic. To be called in the main loop.
    void loop();

    // Returns true if the device is connected to the WiFi network.
    bool isConnected();

private:
    ConfigManager& _configManager;
    unsigned long _lastConnectionAttempt;

    // The core connection logic.
    void connect();
};

#endif // WIFI_MANAGER_H
