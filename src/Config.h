#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// A magic value to check if the data in EEPROM is valid
#define EEPROM_MAGIC 0xDEADBEEF

// A struct to hold all configuration parameters, making it easy to save/load.
struct AppConfig {
    // Validation
    uint32_t magic;

    // WiFi Credentials
    char wifiSsid[33];
    char wifiPassword[65];

    // MQTT Broker Details
    char mqttServer[65];
    int mqttPort;
    char mqttUser[33];
    char mqttPassword[65];
    char mqttTopicBase[65]; // e.g., "home/watertank"

    // Hardware Pins (using GPIO numbers)
    uint8_t pinSonarTrig;
    uint8_t pinSonarEcho;
    uint8_t pinFlowMeter;
    uint8_t pinPumpRelay;
    uint8_t pinResetButton;

    // Tank Dimensions (in meters)
    float tankRadius;
    float tankLength;

    // Water Level Thresholds (in meters from the bottom)
    float minWaterLevel;     // Level to stop the pump
    float restartWaterLevel; // Level to allow pump to operate again

    // Physical setup
    float sensorMountHeight; // Height of the sonar sensor from the tank bottom (in meters)
};

class ConfigManager {
public:
    ConfigManager();

    // Loads configuration from EEPROM. If invalid, loads defaults and saves them.
    void begin();

    // Saves the current configuration to EEPROM.
    void save();

    // Resets the configuration to factory defaults and saves them.
    void resetToDefaults();

    // Provides access to the configuration data.
    AppConfig& getConfig();

private:
    AppConfig config;

    // Sets the hardcoded default values.
    void setDefaults();
};

#endif // CONFIG_H
