/**
 * @file power_hal.h
 * @brief Power/Battery Hardware Abstraction Layer
 */

#ifndef POWER_HAL_H
#define POWER_HAL_H

#include <Arduino.h>
#include "../drivers/adc_driver.h"

namespace HAL {

/**
 * @brief Battery status data structure
 */
struct BatteryStatus {
    bool valid;           // True if reading is valid
    uint32_t voltageMilliV; // Battery voltage in millivolts
    uint8_t percentage;   // Battery percentage (0-100)
    uint32_t timestamp;   // Reading timestamp (millis)
};

/**
 * @brief Power HAL class - abstracts battery/power operations
 */
class PowerHAL {
public:
    /**
     * @brief Constructor
     * @param adcPin ADC pin for battery voltage (-1 if not available)
     * @param voltageDivider Voltage divider ratio (typically 2.0)
     */
    PowerHAL(int8_t adcPin = -1, float voltageDivider = 2.0f);

    /**
     * @brief Initialize power monitoring
     * @return true if initialization successful
     */
    bool init();

    /**
     * @brief Check if battery monitoring is available
     * @return true if battery ADC is configured
     */
    bool isAvailable();

    /**
     * @brief Get battery status
     * @return Battery status data
     */
    BatteryStatus getBatteryStatus();

    /**
     * @brief Get battery voltage in millivolts
     * @return Voltage in mV, or 0 if not available
     */
    uint32_t getVoltageMilliV();

    /**
     * @brief Get battery percentage
     * @return Percentage (0-100), or -1 if not available
     */
    int8_t getPercentage();

    /**
     * @brief Set voltage range for percentage calculation
     * @param minV Minimum voltage (0%)
     * @param maxV Maximum voltage (100%)
     */
    void setVoltageRange(float minV, float maxV);

    /**
     * @brief Set simulated battery level (for testing or when ADC unavailable)
     * @param percentage Simulated percentage (0-100)
     */
    void setSimulatedLevel(uint8_t percentage);

private:
    int8_t _adcPin;
    float _voltageDivider;
    float _minVoltage;
    float _maxVoltage;
    bool _available;
    uint8_t _simulatedLevel;
    bool _useSimulated;

    /**
     * @brief Calculate percentage from voltage
     * @param voltageV Voltage in volts
     * @return Percentage (0-100)
     */
    uint8_t voltageToPercentage(float voltageV);
};

} // namespace HAL

#endif // POWER_HAL_H
