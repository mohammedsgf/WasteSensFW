/**
 * @file us100_driver.h
 * @brief Ultrasonic Sensor Driver (HC-SR04 / US-100 GPIO Mode)
 */

#ifndef US100_DRIVER_H
#define US100_DRIVER_H

#include <Arduino.h>
#include "gpio_driver.h"

namespace Drivers {

/**
 * @brief Ultrasonic Sensor Driver (HC-SR04 compatible)
 * 
 * Works with HC-SR04, US-100 (GPIO mode), and similar sensors
 * - Trigger: 10µs HIGH pulse initiates measurement
 * - Echo: Returns HIGH pulse, width = round-trip time
 * - Speed of sound: 343 m/s at 20°C
 * - Distance = (Time × 0.0343) / 2 cm
 */
class US100Driver {
public:
    /**
     * @brief Constructor
     * @param triggerPin GPIO pin for trigger
     * @param echoPin GPIO pin for echo
     */
    US100Driver(uint8_t triggerPin, uint8_t echoPin);

    /**
     * @brief Initialize the sensor
     */
    void init();

    /**
     * @brief Measure distance in centimeters
     * @param timeoutUs Timeout in microseconds
     * @return Distance in centimeters, or -1 if measurement failed
     */
    float measureDistanceCm(unsigned long timeoutUs = 30000);

    /**
     * @brief Measure distance with averaging
     * @param samples Number of samples to average
     * @param timeoutUs Timeout in microseconds per sample
     * @return Averaged distance in centimeters, or -1 if all measurements failed
     */
    float measureDistanceAvgCm(uint8_t samples = 5, unsigned long timeoutUs = 30000);

    /**
     * @brief Get raw echo duration in microseconds
     * @param timeoutUs Timeout in microseconds
     * @return Echo duration in microseconds, or 0 if timeout
     */
    unsigned long measureEchoDuration(unsigned long timeoutUs = 30000);

private:
    uint8_t _triggerPin;
    uint8_t _echoPin;
    
    // Speed of sound in cm/us (343 m/s = 0.0343 cm/us)
    // Distance = duration * 0.0343 / 2 = duration * 0.01715
    static constexpr float SOUND_SPEED_CM_PER_US = 0.0343f;
};

} // namespace Drivers

#endif // US100_DRIVER_H
