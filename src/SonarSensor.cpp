#include "SonarSensor.h"

// The speed of sound in air in meters per microsecond.
// (343 m/s) / (1,000,000 µs/s) = 0.000343 m/µs
const float SOUND_SPEED_M_PER_US = 0.000343f;

SonarSensor::SonarSensor(uint8_t triggerPin, uint8_t echoPin)
    : _triggerPin(triggerPin), _echoPin(echoPin) {
}

void SonarSensor::setup() {
    pinMode(_triggerPin, OUTPUT);
    pinMode(_echoPin, INPUT);
}

float SonarSensor::getDistance(long timeout) {
    // Ensure the trigger pin is low before starting
    digitalWrite(_triggerPin, LOW);
    delayMicroseconds(2);

    // Send a 10-microsecond high pulse to trigger the measurement
    digitalWrite(_triggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(_triggerPin, LOW);

    // Measure the duration of the echo pulse (in microseconds).
    // pulseIn() waits for the pin to go HIGH, starts timing, then waits for the
    // pin to go LOW and stops timing. The timeout prevents it from blocking forever.
    long duration = pulseIn(_echoPin, HIGH, timeout);

    // If the duration is 0, it means the pulseIn function timed out.
    if (duration == 0) {
        return -1.0f; // Return an error value
    }

    // Calculate the distance:
    // Distance = (Time * Speed of Sound) / 2
    // We divide by 2 because the sound wave travels to the object and back.
    float distance = (duration * SOUND_SPEED_M_PER_US) / 2.0f;

    return distance;
}
