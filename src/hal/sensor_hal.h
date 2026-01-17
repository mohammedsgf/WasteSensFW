/**
 * @file sensor_hal.h
 * @brief Sensor Hardware Abstraction Layer
 */

#ifndef SENSOR_HAL_H
#define SENSOR_HAL_H

#include <Arduino.h>
#include "../drivers/us100_driver.h"

namespace HAL {

/**
 * @brief Distance sensor reading result
 */
struct DistanceReading {
    bool valid;         // True if reading is valid
    float distanceCm;   // Distance in centimeters
    uint32_t timestamp; // Reading timestamp (millis)
};

/**
 * @brief Sensor HAL class - abstracts distance sensor operations
 */
class SensorHAL {
public:
    /**
     * @brief Constructor
     * @param driver Reference to US100 driver
     */
    SensorHAL(Drivers::US100Driver& driver);

    /**
     * @brief Initialize sensor
     * @return true if initialization successful
     */
    bool init();

    /**
     * @brief Get single distance reading
     * @return Distance reading result
     */
    DistanceReading getDistance();

    /**
     * @brief Get averaged distance reading
     * @param samples Number of samples to average
     * @return Averaged distance reading result
     */
    DistanceReading getDistanceAvg(uint8_t samples = 5);

    /**
     * @brief Check if sensor is connected and responding
     * @return true if sensor responds
     */
    bool isConnected();

    /**
     * @brief Set measurement timeout
     * @param timeoutUs Timeout in microseconds
     */
    void setTimeout(unsigned long timeoutUs);

private:
    Drivers::US100Driver& _driver;
    unsigned long _timeoutUs;
    bool _initialized;
};

} // namespace HAL

#endif // SENSOR_HAL_H
