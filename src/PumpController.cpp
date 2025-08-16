#include "PumpController.h"
#include "Arduino.h"

// How often to publish the pump status (in milliseconds)
#define PUMP_STATUS_PUBLISH_INTERVAL_MS 5000

PumpController::PumpController(ConfigManager& config, SonarSensor& sonar, FlowMeter& flow, MqttManager& mqtt)
    : _config(config),
      _sonar(sonar),
      _flow(flow),
      _mqtt(mqtt),
      _state(PumpState::IDLE),
      _pumpPin(0),
      _currentWaterLevel(0.0f),
      _pumpStartTimeMs(0),
      _targetDurationMs(0),
      _targetVolumeLiters(0.0f),
      _volumeAtStart(0.0f) {}

void PumpController::setup() {
    _pumpPin = _config.getConfig().pinPumpRelay;
    pinMode(_pumpPin, OUTPUT);
    physicalPumpOff(); // Ensure pump is off at startup
    setState(PumpState::IDLE);
}

void PumpController::loop() {
    // --- High Priority Safety Check ---
    // This runs regardless of the current state.
    _currentWaterLevel = _sonar.getDistance(); // NOTE: This needs to be converted to water level!
    // For now, let's assume getDistance IS the water level for simplicity.
    // TODO: Convert distance from sensor (at top) to water level from bottom.

    AppConfig& cfg = _config.getConfig();
    if (_currentWaterLevel < cfg.minWaterLevel && _state != PumpState::STOPPED_LOW_LEVEL) {
        if (_state != PumpState::IDLE) { // Don't show scary message if idle
            Serial.println("CRITICAL: Water level too low! Forcing pump OFF.");
            setState(PumpState::STOPPED_LOW_LEVEL);
        }
    }

    // --- State Machine Logic ---
    switch (_state) {
        case PumpState::STOPPED_LOW_LEVEL:
            if (_currentWaterLevel >= cfg.restartWaterLevel) {
                Serial.println("Water level has recovered. Pump is now idle.");
                setState(PumpState::IDLE);
            }
            break;

        case PumpState::PUMPING_FOR_TIME:
            if (millis() - _pumpStartTimeMs >= _targetDurationMs) {
                Serial.println("Pump time finished.");
                stop();
            }
            break;

        case PumpState::PUMPING_FOR_VOLUME:
            if (_flow.getTotalVolume() - _volumeAtStart >= _targetVolumeLiters) {
                Serial.println("Pump volume reached.");
                stop();
            }
            break;

        case PumpState::IDLE:
        case PumpState::PUMPING_MANUAL:
        case PumpState::STOPPED_COMMAND:
            // No time/volume-based logic needed for these states.
            break;
    }
}

// --- Commands ---

bool PumpController::canPumpStart() {
    if (_state == PumpState::STOPPED_LOW_LEVEL) {
        Serial.println("Cannot start pump: water level is critically low.");
        return false;
    }
    return true;
}

void PumpController::startManual() {
    if (!canPumpStart()) return;
    setState(PumpState::PUMPING_MANUAL);
}

void PumpController::startForDuration(unsigned long seconds) {
    if (!canPumpStart()) return;
    _targetDurationMs = seconds * 1000;
    _pumpStartTimeMs = millis();
    setState(PumpState::PUMPING_FOR_TIME);
}

void PumpController::startForVolume(float liters) {
    if (!canPumpStart()) return;
    _targetVolumeLiters = liters;
    _flow.resetTotalVolume(); // Or use _volumeAtStart = _flow.getTotalVolume();
    _volumeAtStart = _flow.getTotalVolume();
    setState(PumpState::PUMPING_FOR_VOLUME);
}

void PumpController::stop(bool command) {
    if (command) {
        setState(PumpState::STOPPED_COMMAND);
    } else {
        setState(PumpState::IDLE);
    }
}

// --- Internal Methods ---

void PumpController::setState(PumpState newState) {
    if (_state == newState) return; // No change

    _state = newState;
    Serial.print("Pump state changed to: ");
    Serial.println(getStateName());

    if (_state == PumpState::PUMPING_MANUAL ||
        _state == PumpState::PUMPING_FOR_TIME ||
        _state == PumpState::PUMPING_FOR_VOLUME) {
        physicalPumpOn();
    } else {
        physicalPumpOff();
    }

    publishState();
}

void PumpController::physicalPumpOn() {
    digitalWrite(_pumpPin, HIGH); // Assuming HIGH turns the relay ON
}

void PumpController::physicalPumpOff() {
    digitalWrite(_pumpPin, LOW); // Assuming LOW turns the relay OFF
}

void PumpController::publishState() {
    _mqtt.publish("status/pump", getStateName(), true);
}

PumpState PumpController::getState() const {
    return _state;
}

const char* PumpController::getStateName() const {
    switch (_state) {
        case PumpState::IDLE: return "IDLE";
        case PumpState::PUMPING_MANUAL: return "PUMPING_MANUAL";
        case PumpState::PUMPING_FOR_TIME: return "PUMPING_FOR_TIME";
        case PumpState::PUMPING_FOR_VOLUME: return "PUMPING_FOR_VOLUME";
        case PumpState::STOPPED_LOW_LEVEL: return "STOPPED_LOW_LEVEL";
        case PumpState::STOPPED_COMMAND: return "STOPPED_COMMAND";
        default: return "UNKNOWN";
    }
}
