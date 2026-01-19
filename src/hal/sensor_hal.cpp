/**
 * @file sensor_hal.cpp
 * @brief Sensor HAL implementation
 */

#include "sensor_hal.h"
#include "config.h"

namespace HAL {

SensorHAL::SensorHAL(Drivers::US100Driver& driver)
    : _driver(driver), _timeoutUs(US100_TIMEOUT_US), _initialized(false) {
}

bool SensorHAL::init() {
    DEBUG_PRINTLN("[SensorHAL] Initializing sensor...");
    
    _driver.init();
    _initialized = true;
    
    // Test if sensor responds
    if (!isConnected()) {
        DEBUG_PRINTLN("[SensorHAL] Warning: Sensor not responding");
    } else {
        DEBUG_PRINTLN("[SensorHAL] Sensor initialized");
    }
    
    return true;
}

DistanceReading SensorHAL::getDistance() {
    DistanceReading reading;
    reading.timestamp = millis();
    
    float distance = _driver.measureDistanceCm(_timeoutUs);
    
    if (distance < 0 || distance < SENSOR_MIN_DISTANCE_CM || distance > SENSOR_MAX_DISTANCE_CM) {
        reading.valid = false;
        reading.distanceCm = -1;
        DEBUG_PRINTLN("[SensorHAL] Invalid distance reading");
    } else {
        reading.valid = true;
        reading.distanceCm = distance;
        DEBUG_PRINTF("[SensorHAL] Distance: %.2f cm\n", distance);
    }
    
    return reading;
}

DistanceReading SensorHAL::getDistanceAvg(uint8_t samples) {
    DistanceReading reading;
    reading.timestamp = millis();
    
    float distance = _driver.measureDistanceAvgCm(samples, _timeoutUs);
    
    if (distance < 0 || distance < SENSOR_MIN_DISTANCE_CM || distance > SENSOR_MAX_DISTANCE_CM) {
        reading.valid = false;
        reading.distanceCm = -1;
        DEBUG_PRINTF("[SensorHAL] Invalid averaged reading (%d samples)\n", samples);
    } else {
        reading.valid = true;
        reading.distanceCm = distance;
        DEBUG_PRINTF("[SensorHAL] Averaged distance: %.2f cm (%d samples)\n", distance, samples);
    }
    
    return reading;
}

bool SensorHAL::isConnected() {
    // Try to get a reading to verify sensor connection
    float distance = _driver.measureDistanceCm(_timeoutUs);
    return (distance > 0);
}

void SensorHAL::setTimeout(unsigned long timeoutUs) {
    _timeoutUs = timeoutUs;
}

} // namespace HAL
