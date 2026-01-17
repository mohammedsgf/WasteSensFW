/**
 * @file adc_driver.cpp
 * @brief ADC Driver implementation
 */

#include "adc_driver.h"

namespace Drivers {

void AdcDriver::init() {
    // Set default ADC settings
    analogSetAttenuation(ADC_11db);
    analogReadResolution(12);
    
#if CONFIG_IDF_TARGET_ESP32
    analogSetWidth(12);
#endif
}

void AdcDriver::setResolution(uint8_t bits) {
    analogReadResolution(bits);
#if CONFIG_IDF_TARGET_ESP32
    analogSetWidth(bits);
#endif
}

void AdcDriver::setAttenuation(AdcAttenuation attenuation) {
    adc_attenuation_t atten;
    switch (attenuation) {
        case AdcAttenuation::DB_0:
            atten = ADC_0db;
            break;
        case AdcAttenuation::DB_2_5:
            atten = ADC_2_5db;
            break;
        case AdcAttenuation::DB_6:
            atten = ADC_6db;
            break;
        case AdcAttenuation::DB_11:
        default:
            atten = ADC_11db;
            break;
    }
    analogSetAttenuation(atten);
}

uint16_t AdcDriver::readRaw(uint8_t pin) {
    return analogRead(pin);
}

uint32_t AdcDriver::readMilliVolts(uint8_t pin) {
    return analogReadMilliVolts(pin);
}

uint32_t AdcDriver::readMilliVoltsAvg(uint8_t pin, uint8_t samples) {
    if (samples == 0) samples = 1;
    
    uint32_t sum = 0;
    for (uint8_t i = 0; i < samples; i++) {
        sum += analogReadMilliVolts(pin);
        delay(1);
    }
    return sum / samples;
}

} // namespace Drivers
