#ifndef FLOW_METER_H
#define FLOW_METER_H

#include <Arduino.h>

class FlowMeter {
public:
    // Constructor takes the GPIO pin number for the sensor's signal.
    FlowMeter(uint8_t signalPin);

    // Configures the pin and attaches the interrupt.
    void setup();

    // Should be called periodically to calculate the flow rate.
    void loop();

    // Returns the current flow rate in Liters per Minute.
    float getFlowRate() const;

    // Returns the total volume of water that has passed in Liters.
    float getTotalVolume() const;

    // Resets the total volume counter to zero.
    void resetTotalVolume();

    // The public ISR handler. Must be marked with IRAM_ATTR for ESP8266.
    void IRAM_ATTR pulseCounter();

private:
    uint8_t _pin;

    // Volatile is crucial here because this variable is modified by an ISR
    // and read in the main loop. It prevents the compiler from making
    // optimizations that could lead to incorrect values.
    volatile unsigned long _pulseCount;

    unsigned long _lastReadTime;
    float _flowRate;
    float _totalVolume;

    // The YF-S201 sensor outputs a certain number of pulses per liter.
    // From the datasheet, it's typically around 450 pulses per liter.
    // (7.5 pulses/sec for 1 L/min => 7.5 * 60 = 450 pulses/L)
    const float PULSES_PER_LITER = 450.0f;
};

#endif // FLOW_METER_H
