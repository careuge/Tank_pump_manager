#include "WifiManager.h"
#include <ESP8266WiFi.h>

// How often to retry the connection in milliseconds
#define WIFI_RETRY_DELAY_MS 10000

WifiManager::WifiManager(ConfigManager& configManager)
    : _configManager(configManager), _lastConnectionAttempt(0) {
}

void WifiManager::setup() {
    WiFi.mode(WIFI_STA);
    WiFi.persistent(false); // Do not save WiFi config in SDK memory
    connect();
}

void WifiManager::loop() {
    // If we are not connected, and enough time has passed since the last attempt, try again.
    if (!isConnected() && (millis() - _lastConnectionAttempt > WIFI_RETRY_DELAY_MS)) {
        connect();
    }
}

bool WifiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

void WifiManager::connect() {
    _lastConnectionAttempt = millis();
    AppConfig& config = _configManager.getConfig();

    // Check if the SSID is valid before attempting to connect.
    if (strlen(config.wifiSsid) == 0 || strcmp(config.wifiSsid, "CHANGEME_SSID") == 0) {
        Serial.println("WiFi not configured. Cannot connect.");
        return;
    }

    Serial.print("Connecting to WiFi network: ");
    Serial.println(config.wifiSsid);

    // Start the connection attempt.
    WiFi.begin(config.wifiSsid, config.wifiPassword);

    // This part is blocking, but it's typically only run once on startup
    // or after a long disconnect.
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) { // ~10 second timeout
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (isConnected()) {
        Serial.println("\nWiFi connected successfully!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nFailed to connect to WiFi. Will retry later.");
        WiFi.disconnect(); // Go to a known disconnected state
    }
}
