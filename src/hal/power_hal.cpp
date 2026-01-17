/**
 * @file power_hal.cpp
 * @brief Power HAL implementation
 */

#include "power_hal.h"
#include "config.h"

namespace HAL {

PowerHAL::PowerHAL(int8_t adcPin, float voltageDivider)
    : _adcPin(adcPin), _voltageDivider(voltageDivider),
      _minVoltage(BATTERY_MIN_VOLTAGE), _maxVoltage(BATTERY_MAX_VOLTAGE),
      _available(false), _simulatedLevel(100), _useSimulated(false) {
}

bool PowerHAL::init() {
    DEBUG_PRINTLN("[PowerHAL] Initializing power monitoring...");
    
    if (_adcPin < 0) {
        DEBUG_PRINTLN("[PowerHAL] No battery ADC pin configured, using simulated values");
        _available = false;
        _useSimulated = true;
        return true;
    }
    
    // Initialize ADC
    Drivers::AdcDriver::init();
    _available = true;
    
    DEBUG_PRINTF("[PowerHAL] Battery monitoring on pin %d\n", _adcPin);
    return true;
}

bool PowerHAL::isAvailable() {
    return _available;
}

BatteryStatus PowerHAL::getBatteryStatus() {
    BatteryStatus status;
    status.timestamp = millis();
    
    if (_useSimulated || !_available) {
        // Return simulated values
        status.valid = true;
        status.percentage = _simulatedLevel;
        // Calculate simulated voltage from percentage
        float voltageRange = _maxVoltage - _minVoltage;
        float voltage = _minVoltage + (voltageRange * _simulatedLevel / 100.0f);
        status.voltageMilliV = (uint32_t)(voltage * 1000);
        
        DEBUG_PRINTF("[PowerHAL] Simulated battery: %d%% (%.2fV)\n", 
                     status.percentage, voltage);
        return status;
    }
    
    // Read actual battery voltage
    uint32_t adcMilliV = Drivers::AdcDriver::readMilliVoltsAvg(_adcPin, 10);
    status.voltageMilliV = (uint32_t)(adcMilliV * _voltageDivider);
    
    float voltageV = status.voltageMilliV / 1000.0f;
    status.percentage = voltageToPercentage(voltageV);
    status.valid = true;
    
    DEBUG_PRINTF("[PowerHAL] Battery: %d%% (%lumV)\n", 
                 status.percentage, status.voltageMilliV);
    
    return status;
}

uint32_t PowerHAL::getVoltageMilliV() {
    BatteryStatus status = getBatteryStatus();
    return status.voltageMilliV;
}

int8_t PowerHAL::getPercentage() {
    BatteryStatus status = getBatteryStatus();
    return status.valid ? status.percentage : -1;
}

void PowerHAL::setVoltageRange(float minV, float maxV) {
    _minVoltage = minV;
    _maxVoltage = maxV;
}

void PowerHAL::setSimulatedLevel(uint8_t percentage) {
    _simulatedLevel = (percentage > 100) ? 100 : percentage;
    _useSimulated = true;
}

uint8_t PowerHAL::voltageToPercentage(float voltageV) {
    if (voltageV <= _minVoltage) return 0;
    if (voltageV >= _maxVoltage) return 100;
    
    float range = _maxVoltage - _minVoltage;
    float percentage = ((voltageV - _minVoltage) / range) * 100.0f;
    
    return (uint8_t)percentage;
}

} // namespace HAL
