#include "FlowMeter.h"
#include <functional>

// --- Interrupt Service Routine (ISR) Handling ---

// This is a global pointer to the FlowMeter instance. It's a common pattern
// in Arduino/C++ when a C-style ISR needs to call a class method.
// This design assumes you will only have ONE FlowMeter object.
static FlowMeter* globalFlowMeterInstance = nullptr;

// This is the function that will be attached to the interrupt pin.
// It must be a standalone function, not a class method.
// IRAM_ATTR ensures this code is placed in the ESP8266's RAM for faster execution.
void IRAM_ATTR staticFlowMeterIsr() {
    if (globalFlowMeterInstance) {
        globalFlowMeterInstance->pulseCounter();
    }
}

// --- Class Implementation ---

FlowMeter::FlowMeter(uint8_t signalPin)
    : _pin(signalPin),
      _pulseCount(0),
      _lastReadTime(0),
      _flowRate(0.0f),
      _totalVolume(0.0f) {
    // Set the global instance pointer to this object.
    globalFlowMeterInstance = this;
}

void FlowMeter::setup() {
    // Configure the sensor pin as an input with an internal pull-up resistor.
    // This is common for digital sensors to prevent floating states.
    pinMode(_pin, INPUT_PULLUP);

    // Attach the ISR to the pin. It will trigger on the RISING edge of the signal.
    attachInterrupt(digitalPinToInterrupt(_pin), staticFlowMeterIsr, RISING);
}

void FlowMeter::loop() {
    unsigned long currentTime = millis();
    // Every second, calculate the flow rate based on the pulses counted.
    if (currentTime - _lastReadTime >= 1000) {
        // Temporarily disable interrupts to safely read the volatile variable _pulseCount.
        // This prevents the ISR from modifying the value while we are reading it.
        noInterrupts();
        unsigned long currentPulses = _pulseCount;
        interrupts();

        // Reset the pulse count for the next interval.
        _pulseCount -= currentPulses;

        // Calculate flow rate:
        // Flow Rate (Liters/Minute) = (Pulses / Time Elapsed in Sec) * (60 Sec / 1 Min) / (Pulses / 1 Liter)
        unsigned long timeElapsed = currentTime - _lastReadTime;
        _flowRate = ((1000.0f / timeElapsed) * currentPulses) * 60.0f / PULSES_PER_LITER;

        _lastReadTime = currentTime;
    }
}

// This is the method called by the ISR. Keep it as short and fast as possible.
void IRAM_ATTR FlowMeter::pulseCounter() {
    // Increment the pulse count.
    _pulseCount++;
    // Add the volume of one pulse to the total.
    _totalVolume += (1.0f / PULSES_PER_LITER);
}

float FlowMeter::getFlowRate() const {
    return _flowRate;
}

float FlowMeter::getTotalVolume() const {
    return _totalVolume;
}

void FlowMeter::resetTotalVolume() {
    // Safely reset the volume counter.
    noInterrupts();
    _totalVolume = 0.0f;
    interrupts();
}
