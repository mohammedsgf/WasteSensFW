/**
 * @file us100_driver.cpp
 * @brief US-100 Ultrasonic Sensor Driver implementation
 */

#include "us100_driver.h"

namespace Drivers {

US100Driver::US100Driver(uint8_t triggerPin, uint8_t echoPin)
    : _triggerPin(triggerPin), _echoPin(echoPin) {
}

void US100Driver::init() {
    GpioDriver::configurePin(_triggerPin, PinMode::OUTPUT_MODE);
    GpioDriver::configurePin(_echoPin, PinMode::INPUT_MODE);
    
    // Ensure trigger is LOW initially
    GpioDriver::writeDigital(_triggerPin, LOW);
    delay(50); // Let sensor settle
}

unsigned long US100Driver::measureEchoDuration(unsigned long timeoutUs) {
    // Clear trigger pin
    GpioDriver::writeDigital(_triggerPin, LOW);
    delayMicroseconds(2);
    
    // Generate 10us trigger pulse
    GpioDriver::writeDigital(_triggerPin, HIGH);
    delayMicroseconds(10);
    GpioDriver::writeDigital(_triggerPin, LOW);
    
    // Measure echo pulse duration
    return GpioDriver::measurePulse(_echoPin, HIGH, timeoutUs);
}

float US100Driver::measureDistanceCm(unsigned long timeoutUs) {
    unsigned long duration = measureEchoDuration(timeoutUs);
    
    if (duration == 0) {
        return -1.0f; // Timeout or no echo
    }
    
    // Calculate distance: duration * speed_of_sound / 2
    // Speed of sound = 343 m/s = 0.0343 cm/us
    float distance = (duration * SOUND_SPEED_CM_PER_US) / 2.0f;
    
    return distance;
}

float US100Driver::measureDistanceAvgCm(uint8_t samples, unsigned long timeoutUs) {
    if (samples == 0) samples = 1;
    
    float sum = 0;
    uint8_t validSamples = 0;
    
    for (uint8_t i = 0; i < samples; i++) {
        float distance = measureDistanceCm(timeoutUs);
        
        if (distance > 0) {
            sum += distance;
            validSamples++;
        }
        
        delay(60); // Minimum 60ms between measurements for US-100
    }
    
    if (validSamples == 0) {
        return -1.0f; // All measurements failed
    }
    
    return sum / validSamples;
}

} // namespace Drivers
