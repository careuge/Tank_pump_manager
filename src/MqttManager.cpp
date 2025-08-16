#include "MqttManager.h"

// How often to retry MQTT connection in milliseconds
#define MQTT_RECONNECT_DELAY_MS 5000

MqttManager::MqttManager(ConfigManager& configManager, WiFiClient& wifiClient)
    : _configManager(configManager),
      _mqttClient(wifiClient),
      _messageHandler(nullptr),
      _lastReconnectAttempt(0) {
}

void MqttManager::setup(MqttMessageHandler messageHandler) {
    _messageHandler = messageHandler;
    AppConfig& config = _configManager.getConfig();

    // Configure the MQTT client with the server address and port from config
    _mqttClient.setServer(config.mqttServer, config.mqttPort);

    // Set the callback function for incoming messages.
    // We use a lambda to capture 'this' and call a member function,
    // which is a standard C++ pattern for C-style callbacks.
    _mqttClient.setCallback([this](char* topic, byte* payload, unsigned int length) {
        this->internalCallback(topic, payload, length);
    });
}

void MqttManager::loop() {
    // If not connected, try to reconnect periodically.
    if (!_mqttClient.connected()) {
        long now = millis();
        if (now - _lastReconnectAttempt > MQTT_RECONNECT_DELAY_MS) {
            _lastReconnectAttempt = now;
            connect();
        }
    } else {
        // If connected, process incoming messages and maintain the connection.
        _mqttClient.loop();
    }
}

void MqttManager::publish(const char* subTopic, const char* payload, bool retained) {
    if (!isConnected()) {
        Serial.println("Cannot publish, MQTT not connected.");
        return;
    }

    // Construct the full topic path, e.g., "home/watertank/status/level"
    char fullTopic[128];
    snprintf(fullTopic, sizeof(fullTopic), "%s/%s", _configManager.getConfig().mqttTopicBase, subTopic);

    _mqttClient.publish(fullTopic, payload, retained);
}

bool MqttManager::isConnected() {
    return _mqttClient.connected();
}

void MqttManager::connect() {
    Serial.println("Attempting MQTT connection...");
    AppConfig& config = _configManager.getConfig();

    // Generate a unique client ID
    String clientId = "ESP-WaterTank-" + String(ESP.getChipId());

    // Attempt to connect
    if (_mqttClient.connect(clientId.c_str(), config.mqttUser, config.mqttPassword)) {
        Serial.println("MQTT connected successfully!");
        // Once connected, resubscribe to topics
        subscribeToTopics();
    } else {
        Serial.print("MQTT connection failed, rc=");
        Serial.print(_mqttClient.state());
        Serial.println(" | Retrying in 5 seconds...");
    }
}

void MqttManager::subscribeToTopics() {
    char topic[128];
    AppConfig& config = _configManager.getConfig();

    // Subscribe to all topics under the command hierarchy
    // e.g., home/watertank/command/pump, home/watertank/command/reset
    snprintf(topic, sizeof(topic), "%s/command/#", config.mqttTopicBase);
    _mqttClient.subscribe(topic);

    Serial.print("Subscribed to MQTT topic: ");
    Serial.println(topic);

    // We could also subscribe to a separate config topic tree if desired
    // snprintf(topic, sizeof(topic), "%s/config/#", config.mqttTopicBase);
    // _mqttClient.subscribe(topic);
    // Serial.print("Subscribed to MQTT topic: ");
    // Serial.println(topic);
}

void MqttManager::internalCallback(char* c_topic, byte* payload, unsigned int length) {
    // Convert the C-style topic and payload to C++ Strings
    String topic(c_topic);

    // Create a null-terminated string from the payload
    String payloadStr;
    payloadStr.reserve(length + 1);
    for (unsigned int i = 0; i < length; i++) {
        payloadStr += (char)payload[i];
    }

    Serial.print("MQTT Message Received [");
    Serial.print(topic);
    Serial.print("]: ");
    Serial.println(payloadStr);

    // If a message handler is registered, call it.
    if (_messageHandler) {
        _messageHandler(topic, payloadStr);
    }
}
