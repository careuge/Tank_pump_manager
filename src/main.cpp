#include <Arduino.h>
#include "Config.h"
#include "WifiManager.h"
#include "MqttManager.h"
#include "SonarSensor.h"
#include "FlowMeter.h"
#include "PumpController.h"
#include "FactoryReset.h"
#include "TankModel.h"

// --- Global Object Declarations ---
ConfigManager configManager;
WifiManager wifiManager(configManager);
WiFiClient wifiClient;
MqttManager mqttManager(configManager, wifiClient);
SonarSensor* sonarSensor = nullptr;
FlowMeter* flowMeter = nullptr;
PumpController* pumpController = nullptr;
FactoryResetHandler* factoryResetHandler = nullptr;

// --- MQTT Message Handler ---
void handleMqttMessage(String& topic, String& payload);

// --- Status Publishing and State Globals ---
unsigned long lastStatusPublishTime = 0;
const unsigned long STATUS_PUBLISH_INTERVAL_MS = 15000; // 15 seconds
bool networkWasConnected = false;

// =================================================================
// SETUP
// =================================================================
void setup() {
    Serial.begin(115200);
    Serial.println("\n--- Water Tank Controller Booting Up ---");

    // 1. Initialize Configuration
    configManager.begin();
    AppConfig& config = configManager.getConfig();

    // 2. Initialize Hardware Abstractions
    sonarSensor = new SonarSensor(config.pinSonarTrig, config.pinSonarEcho);
    sonarSensor->setup();

    flowMeter = new FlowMeter(config.pinFlowMeter);
    flowMeter->setup();

    // 3. Initialize Core Services
    // Note: The order is important here.
    wifiManager.setup();
    mqttManager.setup(handleMqttMessage);

    // 4. Initialize Controllers
    pumpController = new PumpController(configManager, *sonarSensor, *flowMeter, mqttManager);
    pumpController->setup();

    factoryResetHandler = new FactoryResetHandler(configManager, config.pinResetButton);
    factoryResetHandler->setup();

    Serial.println("--- Setup Complete ---");
}

// =================================================================
// LOOP
// =================================================================
void loop() {
    // Run the loop function for all active components
    wifiManager.loop();
    mqttManager.loop();
    pumpController->loop();
    factoryResetHandler->loop();
    flowMeter->loop();

    // --- Network Connection Event Handling ---
    bool networkIsConnected = wifiManager.isConnected() && mqttManager.isConnected();
    if (networkWasConnected && !networkIsConnected) {
        Serial.println("Network connection LOST. Activating pump in manual mode as a fallback.");
        pumpController->startManual();
    } else if (!networkWasConnected && networkIsConnected) {
        Serial.println("Network connection RE-ESTABLISHED. Returning pump to IDLE state.");
        pumpController->stop(false); // false = not a command, just return to idle
    }
    networkWasConnected = networkIsConnected;

    // --- Periodic Status Publishing ---
    if (mqttManager.isConnected() && (millis() - lastStatusPublishTime > STATUS_PUBLISH_INTERVAL_MS)) {
        lastStatusPublishTime = millis();

        Serial.println("Publishing status updates...");

        AppConfig& config = configManager.getConfig();

        // Read sensor for water level
        // The sonar measures distance from the sensor to the water.
        // Water level is the height of the sensor minus this distance.
        float distance = sonarSensor->getDistance();
        float waterLevel = config.sensorMountHeight - distance;
        if (waterLevel < 0) waterLevel = 0;

        char buffer[20];
        dtostrf(waterLevel, 4, 3, buffer);
        mqttManager.publish("status/water_level_m", buffer);

        // Calculate and publish volume
        double volumeM3 = TankModel::calculateWaterVolume(waterLevel, config.tankRadius, config.tankLength);
        double volumeLiters = volumeM3 * 1000.0;

        dtostrf(volumeM3, 5, 3, buffer);
        mqttManager.publish("status/water_volume_m3", buffer);
        dtostrf(volumeLiters, 6, 2, buffer);
        mqttManager.publish("status/water_volume_liters", buffer, true); // Retain

        // Publish flow rate
        dtostrf(flowMeter->getFlowRate(), 4, 2, buffer);
        mqttManager.publish("status/flow_rate_lpm", buffer);

        // Publish IP address
        mqttManager.publish("status/ip_address", WiFi.localIP().toString().c_str());
    }
}

// =================================================================
// MQTT MESSAGE HANDLER
// =================================================================
void handleMqttMessage(String& topic, String& payload) {
    Serial.println("Handling MQTT Message...");

    String baseTopic = configManager.getConfig().mqttTopicBase;
    String commandTopic = topic.substring(baseTopic.length() + 1);

    // --- Pump Commands ---
    if (commandTopic == "command/pump") {
        if (payload.equalsIgnoreCase("ON")) pumpController->startManual();
        else if (payload.equalsIgnoreCase("OFF")) pumpController->stop(true);
        return;
    }
    if (commandTopic == "command/pump_duration_s") {
        long seconds = payload.toInt();
        if (seconds > 0) pumpController->startForDuration(seconds);
        return;
    }
    if (commandTopic == "command/pump_volume_l") {
        float liters = payload.toFloat();
        if (liters > 0) pumpController->startForVolume(liters);
        return;
    }

    // --- Configuration Updates ---
    if (commandTopic.startsWith("config/")) {
        String configKey = commandTopic.substring(7); // "config/".length()
        AppConfig& config = configManager.getConfig();
        bool configChanged = true; // Assume change until proven otherwise

        // Helper macro for safe string copy
        #define SET_CONFIG_STRING(field) strncpy(config.field, payload.c_str(), sizeof(config.field) - 1); config.field[sizeof(config.field) - 1] = '\0'

        if (configKey == "wifiSsid") { SET_CONFIG_STRING(wifiSsid); }
        else if (configKey == "wifiPassword") { SET_CONFIG_STRING(wifiPassword); }
        else if (configKey == "mqttServer") { SET_CONFIG_STRING(mqttServer); }
        else if (configKey == "mqttUser") { SET_CONFIG_STRING(mqttUser); }
        else if (configKey == "mqttPassword") { SET_CONFIG_STRING(mqttPassword); }
        else if (configKey == "mqttTopicBase") { SET_CONFIG_STRING(mqttTopicBase); }
        else if (configKey == "mqttPort") { config.mqttPort = payload.toInt(); }
        else if (configKey == "tankRadius") { config.tankRadius = payload.toFloat(); }
        else if (configKey == "tankLength") { config.tankLength = payload.toFloat(); }
        else if (configKey == "minWaterLevel") { config.minWaterLevel = payload.toFloat(); }
        else if (configKey == "restartWaterLevel") { config.restartWaterLevel = payload.toFloat(); }
        else if (configKey == "sensorMountHeight") { config.sensorMountHeight = payload.toFloat(); }
        else {
            configChanged = false;
            Serial.print("Unknown config key: ");
            Serial.println(configKey);
        }

        if (configChanged) {
            configManager.save();
            Serial.print("Configuration saved for: ");
            Serial.println(configKey);
            mqttManager.publish("status/config", "SAVED", false);
            // Note: Some changes like WiFi/MQTT details may require a restart to take effect.
            // A command like "config/reboot" could be added.
        }
    }
}
