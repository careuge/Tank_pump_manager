#ifndef SONAR_SENSOR_H
#define SONAR_SENSOR_H

#include <Arduino.h>

class SonarSensor {
public:
    // Constructor takes the trigger and echo pin numbers.
    SonarSensor(uint8_t triggerPin, uint8_t echoPin);

    // Sets up the GPIO pins for the sensor.
    void setup();

    /**
     * @brief Measures the distance to an object.
     * @param timeout The maximum time to wait for an echo pulse in microseconds.
     * @return The measured distance in meters. Returns a negative value on timeout.
     */
    float getDistance(long timeout = 25000);

private:
    uint8_t _triggerPin;
    uint8_t _echoPin;
};

#endif // SONAR_SENSOR_H
