/**
 * @file adc_driver.h
 * @brief ADC Driver - Low-level analog-to-digital conversion operations
 */

#ifndef ADC_DRIVER_H
#define ADC_DRIVER_H

#include <Arduino.h>
#include <esp32-hal-adc.h>

namespace Drivers {

/**
 * @brief ADC attenuation levels
 */
enum class AdcAttenuation {
    DB_0,   // 0dB attenuation (100mV - 950mV)
    DB_2_5, // 2.5dB attenuation (100mV - 1250mV)
    DB_6,   // 6dB attenuation (150mV - 1750mV)
    DB_11   // 11dB attenuation (150mV - 2450mV)
};

/**
 * @brief ADC Driver class for analog reading operations
 */
class AdcDriver {
public:
    /**
     * @brief Initialize ADC with default settings
     */
    static void init();

    /**
     * @brief Set ADC resolution
     * @param bits Resolution in bits (9-12)
     */
    static void setResolution(uint8_t bits);

    /**
     * @brief Set ADC attenuation
     * @param attenuation Attenuation level
     */
    static void setAttenuation(AdcAttenuation attenuation);

    /**
     * @brief Read raw ADC value
     * @param pin Analog pin number
     * @return Raw ADC value
     */
    static uint16_t readRaw(uint8_t pin);

    /**
     * @brief Read voltage in millivolts
     * @param pin Analog pin number
     * @return Voltage in millivolts
     */
    static uint32_t readMilliVolts(uint8_t pin);

    /**
     * @brief Read averaged voltage in millivolts
     * @param pin Analog pin number
     * @param samples Number of samples to average
     * @return Averaged voltage in millivolts
     */
    static uint32_t readMilliVoltsAvg(uint8_t pin, uint8_t samples);
};

} // namespace Drivers

#endif // ADC_DRIVER_H
