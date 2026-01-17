/**
 * @file gpio_driver.h
 * @brief GPIO Driver - Low-level GPIO operations
 */

#ifndef GPIO_DRIVER_H
#define GPIO_DRIVER_H

#include <Arduino.h>

namespace Drivers {

/**
 * @brief GPIO pin modes
 */
enum class PinMode {
    INPUT_MODE,
    OUTPUT_MODE,
    INPUT_PULLUP_MODE,
    INPUT_PULLDOWN_MODE
};

/**
 * @brief GPIO Driver class for low-level pin operations
 */
class GpioDriver {
public:
    /**
     * @brief Configure a pin mode
     * @param pin GPIO pin number
     * @param mode Pin mode to set
     */
    static void configurePin(uint8_t pin, PinMode mode);

    /**
     * @brief Write digital value to a pin
     * @param pin GPIO pin number
     * @param value HIGH or LOW
     */
    static void writeDigital(uint8_t pin, uint8_t value);

    /**
     * @brief Read digital value from a pin
     * @param pin GPIO pin number
     * @return HIGH or LOW
     */
    static uint8_t readDigital(uint8_t pin);

    /**
     * @brief Measure pulse duration on a pin
     * @param pin GPIO pin number
     * @param state State to measure (HIGH or LOW)
     * @param timeout Timeout in microseconds
     * @return Pulse duration in microseconds, or 0 if timeout
     */
    static unsigned long measurePulse(uint8_t pin, uint8_t state, unsigned long timeout);

    /**
     * @brief Generate a pulse on a pin
     * @param pin GPIO pin number
     * @param state Pulse state (HIGH or LOW)
     * @param durationUs Pulse duration in microseconds
     */
    static void generatePulse(uint8_t pin, uint8_t state, unsigned long durationUs);
};

} // namespace Drivers

#endif // GPIO_DRIVER_H
