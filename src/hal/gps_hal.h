/**
 * @file gps_hal.h
 * @brief GPS Hardware Abstraction Layer
 */

#ifndef GPS_HAL_H
#define GPS_HAL_H

#include <Arduino.h>
#include "../drivers/sim7000_driver.h"

namespace HAL {

/**
 * @brief GPS location data structure
 */
struct GpsLocation {
    bool valid;         // True if fix is valid
    float latitude;     // Latitude in degrees
    float longitude;    // Longitude in degrees
    float altitude;     // Altitude in meters
    float speed;        // Speed in km/h
    float accuracy;     // Accuracy in meters
    int satellites;     // Number of visible satellites
    uint32_t timestamp; // Reading timestamp (millis)
};

/**
 * @brief GPS HAL class - abstracts GPS operations
 */
class GpsHAL {
public:
    /**
     * @brief Constructor
     * @param driver Reference to SIM7000 driver
     */
    GpsHAL(Drivers::SIM7000Driver& driver);

    /**
     * @brief Initialize and enable GPS
     * @param timeout Timeout for GPS fix in milliseconds
     * @return true if GPS enabled successfully
     */
    bool init(uint32_t timeout = 120000);

    /**
     * @brief Enable GPS module
     * @return true if enabled successfully
     */
    bool enable();

    /**
     * @brief Disable GPS module
     */
    void disable();

    /**
     * @brief Check if GPS is enabled
     * @return true if GPS is enabled
     */
    bool isEnabled();

    /**
     * @brief Get current GPS location
     * @param timeout Timeout in milliseconds
     * @return GPS location data
     */
    GpsLocation getLocation(uint32_t timeout = 30000);

    /**
     * @brief Wait for GPS fix
     * @param timeout Timeout in milliseconds
     * @return true if fix obtained
     */
    bool waitForFix(uint32_t timeout = 120000);

    /**
     * @brief Get raw GPS string from modem
     * @return Raw NMEA/GPS string
     */
    String getRawGps();

    /**
     * @brief Set default location (used when GPS unavailable)
     * @param lat Default latitude
     * @param lon Default longitude
     */
    void setDefaultLocation(float lat, float lon);

    /**
     * @brief Get default location
     * @return GPS location with default coordinates
     */
    GpsLocation getDefaultLocation();

private:
    Drivers::SIM7000Driver& _driver;
    bool _enabled;
    float _defaultLat;
    float _defaultLon;
};

} // namespace HAL

#endif // GPS_HAL_H
