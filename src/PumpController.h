#ifndef PUMP_CONTROLLER_H
#define PUMP_CONTROLLER_H

#include "Config.h"
#include "SonarSensor.h"
#include "FlowMeter.h"
#include "MqttManager.h"

// Defines the possible states of the pump for the state machine.
enum class PumpState {
    IDLE,
    PUMPING_MANUAL,
    PUMPING_FOR_TIME,
    PUMPING_FOR_VOLUME,
    STOPPED_LOW_LEVEL,
    STOPPED_COMMAND
};

class PumpController {
public:
    // The constructor takes references to all the modules it needs to interact with.
    PumpController(ConfigManager& config, SonarSensor& sonar, FlowMeter& flow, MqttManager& mqtt);

    // Initializes the pump controller, sets pin modes.
    void setup();

    // The main logic loop for the controller, runs the state machine.
    void loop();

    // --- Public Commands ---
    // These methods are called to change the pump's state.
    void startManual();
    void startForDuration(unsigned long seconds);
    void startForVolume(float liters);
    void stop(bool command = false);

    // --- Status Reporting ---
    PumpState getState() const;
    const char* getStateName() const;

private:
    // References to other application modules
    ConfigManager& _config;
    SonarSensor& _sonar;
    FlowMeter& _flow;
    MqttManager& _mqtt;

    PumpState _state;
    uint8_t _pumpPin;
    float _currentWaterLevel;

    // State-specific variables
    unsigned long _pumpStartTimeMs;
    unsigned long _targetDurationMs;
    float _targetVolumeLiters;
    float _volumeAtStart;

    // Internal state management
    void setState(PumpState newState);
    void publishState();
    void physicalPumpOn();
    void physicalPumpOff();

    // Safety and precondition checks
    bool canPumpStart();
};

#endif // PUMP_CONTROLLER_H
