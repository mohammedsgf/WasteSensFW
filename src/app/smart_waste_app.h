/**
 * @file smart_waste_app.h
 * @brief Smart Waste Monitoring Application
 */

#ifndef SMART_WASTE_APP_H
#define SMART_WASTE_APP_H

#include <Arduino.h>
#include "../hal/modem_hal.h"
#include "../hal/sensor_hal.h"
#include "../hal/gps_hal.h"
#include "../hal/power_hal.h"
#include "../network/gprs_manager.h"
#include "../network/mqtt_service.h"

namespace App {

/**
 * @brief Application state
 */
enum class AppState {
    INIT,
    IDLE,
    READING_SENSORS,
    PUBLISHING,
    ERROR,
    SLEEP
};

/**
 * @brief Sensor readings aggregated
 */
struct SensorReadings {
    float distanceCm;
    int8_t fillLevel;
    float latitude;
    float longitude;
    int8_t batteryLevel;
    bool gpsValid;
    uint32_t timestamp;
};

/**
 * @brief Smart Waste Application
 * 
 * Main application orchestrating all components:
 * - Reads ultrasonic sensor for fill level
 * - Gets GPS location
 * - Reads battery level
 * - Publishes data via MQTT
 */
class SmartWasteApp {
public:
    /**
     * @brief Constructor
     * @param modemHal Reference to modem HAL
     * @param sensorHal Reference to sensor HAL
     * @param gpsHal Reference to GPS HAL
     * @param powerHal Reference to power HAL
     * @param gprsManager Reference to GPRS manager
     * @param mqttService Reference to MQTT service
     */
    SmartWasteApp(HAL::ModemHAL& modemHal,
                  HAL::SensorHAL& sensorHal,
                  HAL::GpsHAL& gpsHal,
                  HAL::PowerHAL& powerHal,
                  Network::GprsManager& gprsManager,
                  Network::MqttService& mqttService);

    /**
     * @brief Initialize application
     * @return true if initialization successful
     */
    bool init();

    /**
     * @brief Run main application loop
     * Call this in Arduino loop()
     */
    void run();

    /**
     * @brief Get current application state
     * @return Application state
     */
    AppState getState();

    /**
     * @brief Get last sensor readings
     * @return Sensor readings structure
     */
    SensorReadings getLastReadings();

    /**
     * @brief Force immediate sensor reading and publish
     */
    void forcePublish();

    /**
     * @brief Set publish interval
     * @param intervalMs Interval in milliseconds
     */
    void setPublishInterval(uint32_t intervalMs);

    /**
     * @brief Set trash can height for fill level calculation
     * @param heightCm Height in centimeters
     */
    void setTrashCanHeight(float heightCm);

private:
    // Component references
    HAL::ModemHAL& _modemHal;
    HAL::SensorHAL& _sensorHal;
    HAL::GpsHAL& _gpsHal;
    HAL::PowerHAL& _powerHal;
    Network::GprsManager& _gprsManager;
    Network::MqttService& _mqttService;
    
    // State
    AppState _state;
    SensorReadings _lastReadings;
    uint32_t _lastPublishTime;
    uint32_t _publishInterval;
    float _trashCanHeight;
    bool _initialized;
    bool _firstRun;  // Flag to trigger immediate first publish

    /**
     * @brief Initialize all hardware components
     * @return true if successful
     */
    bool initHardware();

    /**
     * @brief Initialize network connection
     * @return true if successful
     */
    bool initNetwork();

    /**
     * @brief Read all sensors
     * @return Sensor readings
     */
    SensorReadings readSensors();

    /**
     * @brief Calculate fill level from distance
     * @param distanceCm Distance in centimeters
     * @return Fill level percentage (0-100)
     */
    int8_t calculateFillLevel(float distanceCm);

    /**
     * @brief Publish sensor data to MQTT
     * @param readings Sensor readings to publish
     * @return true if published successfully
     */
    bool publishData(const SensorReadings& readings);

    /**
     * @brief Check if it's time to publish
     * @return true if publish interval elapsed
     */
    bool shouldPublish();

    /**
     * @brief Handle error state
     */
    void handleError();

    /**
     * @brief Blink LED for status indication
     * @param times Number of blinks
     * @param onMs On duration in ms
     * @param offMs Off duration in ms
     */
    void blinkLed(int times, int onMs = 100, int offMs = 100);
};

} // namespace App

#endif // SMART_WASTE_APP_H
