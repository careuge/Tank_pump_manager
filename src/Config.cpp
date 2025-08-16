#include "Config.h"
#include <EEPROM.h>

// --- Default Configuration Values ---
// Replace these with your actual network credentials and settings.
// These are only used the very first time the device boots,
// or after a factory reset. All can be changed later via MQTT.
#define DEFAULT_WIFI_SSID "CHANGEME_SSID"
#define DEFAULT_WIFI_PASS "CHANGEME_PASSWORD"
#define DEFAULT_MQTT_SERVER "192.168.1.100"
#define DEFAULT_MQTT_PORT 1883
#define DEFAULT_MQTT_USER ""
#define DEFAULT_MQTT_PASS ""
#define DEFAULT_MQTT_TOPIC_BASE "home/watertank"

// Default pin assignments for NodeMCU/Wemos D1 Mini
// https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/
#define DEFAULT_PIN_SONAR_TRIG D1 // GPIO5
#define DEFAULT_PIN_SONAR_ECHO D2 // GPIO4
#define DEFAULT_PIN_FLOW_METER D5 // GPIO14
#define DEFAULT_PIN_PUMP_RELAY D6 // GPIO12
#define DEFAULT_PIN_RESET_BUTTON D0 // GPIO16

// Default physical properties of the tank
#define DEFAULT_TANK_RADIUS 0.5f // 50cm radius
#define DEFAULT_TANK_LENGTH 2.0f // 2 meters long
#define DEFAULT_MIN_WATER_LEVEL 0.1f // Stop pump at 10cm
#define DEFAULT_RESTART_WATER_LEVEL 0.2f // Allow restart at 20cm
#define DEFAULT_SENSOR_MOUNT_HEIGHT (DEFAULT_TANK_RADIUS * 2.0f) // Assume sensor is at the top
// --- End of Default Values ---

ConfigManager::ConfigManager() {
    // Constructor is intentionally left empty. Initialization is done in begin().
}

void ConfigManager::begin() {
    // Allocate EEPROM space. Size is determined by the AppConfig struct.
    EEPROM.begin(sizeof(AppConfig));

    // Read the configuration from EEPROM into the config object.
    EEPROM.get(0, config);

    // Check if the loaded data is valid using the magic number.
    if (config.magic != EEPROM_MAGIC) {
        // If not valid (e.g., first boot), load default values and save them.
        resetToDefaults();
    }
}

void ConfigManager::save() {
    // Write the current config object to EEPROM and commit the changes.
    EEPROM.put(0, config);
    EEPROM.commit();
}

void ConfigManager::resetToDefaults() {
    // Populate the config object with hardcoded default values.
    setDefaults();
    // Save these new default values to EEPROM.
    save();
}

AppConfig& ConfigManager::getConfig() {
    // Return a reference to the internal config object.
    return config;
}

void ConfigManager::setDefaults() {
    config.magic = EEPROM_MAGIC;

    // Use strncpy for safe string copying to avoid buffer overflows.
    strncpy(config.wifiSsid, DEFAULT_WIFI_SSID, sizeof(config.wifiSsid) - 1);
    config.wifiSsid[sizeof(config.wifiSsid) - 1] = '\0';

    strncpy(config.wifiPassword, DEFAULT_WIFI_PASS, sizeof(config.wifiPassword) - 1);
    config.wifiPassword[sizeof(config.wifiPassword) - 1] = '\0';

    strncpy(config.mqttServer, DEFAULT_MQTT_SERVER, sizeof(config.mqttServer) - 1);
    config.mqttServer[sizeof(config.mqttServer) - 1] = '\0';

    strncpy(config.mqttUser, DEFAULT_MQTT_USER, sizeof(config.mqttUser) - 1);
    config.mqttUser[sizeof(config.mqttUser) - 1] = '\0';

    strncpy(config.mqttPassword, DEFAULT_MQTT_PASS, sizeof(config.mqttPassword) - 1);
    config.mqttPassword[sizeof(config.mqttPassword) - 1] = '\0';

    strncpy(config.mqttTopicBase, DEFAULT_MQTT_TOPIC_BASE, sizeof(config.mqttTopicBase) - 1);
    config.mqttTopicBase[sizeof(config.mqttTopicBase) - 1] = '\0';

    config.mqttPort = DEFAULT_MQTT_PORT;
    config.pinSonarTrig = DEFAULT_PIN_SONAR_TRIG;
    config.pinSonarEcho = DEFAULT_PIN_SONAR_ECHO;
    config.pinFlowMeter = DEFAULT_PIN_FLOW_METER;
    config.pinPumpRelay = DEFAULT_PIN_PUMP_RELAY;
    config.pinResetButton = DEFAULT_PIN_RESET_BUTTON;
    config.tankRadius = DEFAULT_TANK_RADIUS;
    config.tankLength = DEFAULT_TANK_LENGTH;
    config.minWaterLevel = DEFAULT_MIN_WATER_LEVEL;
    config.restartWaterLevel = DEFAULT_RESTART_WATER_LEVEL;
    config.sensorMountHeight = DEFAULT_SENSOR_MOUNT_HEIGHT;
}
