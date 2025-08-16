#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <functional>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include "Config.h"

// Define a C++ standard function type for our message callback.
// This makes the MqttManager highly reusable, as it doesn't need to know
// about the details of the components that process the messages.
using MqttMessageHandler = std::function<void(String& topic, String& payload)>;

class MqttManager {
public:
    // Constructor requires the configuration and a WiFiClient instance.
    MqttManager(ConfigManager& configManager, WiFiClient& wifiClient);

    // Sets up the MQTT client and registers the callback for incoming messages.
    void setup(MqttMessageHandler messageHandler);

    // Keeps the MQTT client running and handles reconnections. Call in main loop.
    void loop();

    // Publishes a message to a specific sub-topic.
    void publish(const char* subTopic, const char* payload, bool retained = false);

    // Returns true if the client is connected to the MQTT broker.
    bool isConnected();

private:
    ConfigManager& _configManager;
    PubSubClient _mqttClient;
    MqttMessageHandler _messageHandler;
    unsigned long _lastReconnectAttempt;

    // The core connection logic.
    void connect();

    // Subscribes to all the necessary command topics.
    void subscribeToTopics();

    // The internal callback that receives messages from PubSubClient.
    void internalCallback(char* topic, byte* payload, unsigned int length);
};

#endif // MQTT_MANAGER_H
