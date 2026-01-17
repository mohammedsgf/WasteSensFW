/**
 * @file gpio_driver.cpp
 * @brief GPIO Driver implementation
 */

#include "gpio_driver.h"

namespace Drivers {

void GpioDriver::configurePin(uint8_t pin, PinMode mode) {
    switch (mode) {
        case PinMode::INPUT_MODE:
            pinMode(pin, INPUT);
            break;
        case PinMode::OUTPUT_MODE:
            pinMode(pin, OUTPUT);
            break;
        case PinMode::INPUT_PULLUP_MODE:
            pinMode(pin, INPUT_PULLUP);
            break;
        case PinMode::INPUT_PULLDOWN_MODE:
            pinMode(pin, INPUT_PULLDOWN);
            break;
    }
}

void GpioDriver::writeDigital(uint8_t pin, uint8_t value) {
    digitalWrite(pin, value);
}

uint8_t GpioDriver::readDigital(uint8_t pin) {
    return digitalRead(pin);
}

unsigned long GpioDriver::measurePulse(uint8_t pin, uint8_t state, unsigned long timeout) {
    return pulseIn(pin, state, timeout);
}

void GpioDriver::generatePulse(uint8_t pin, uint8_t state, unsigned long durationUs) {
    digitalWrite(pin, !state);
    delayMicroseconds(2);
    digitalWrite(pin, state);
    delayMicroseconds(durationUs);
    digitalWrite(pin, !state);
}

} // namespace Drivers
