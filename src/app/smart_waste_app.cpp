/**
 * @file smart_waste_app.cpp
 * @brief Smart Waste Application implementation
 */

#include "smart_waste_app.h"
#include "config.h"
#include "../drivers/gpio_driver.h"

namespace App {

SmartWasteApp::SmartWasteApp(HAL::ModemHAL& modemHal,
                             HAL::SensorHAL& sensorHal,
                             HAL::GpsHAL& gpsHal,
                             HAL::PowerHAL& powerHal,
                             Network::GprsManager& gprsManager,
                             Network::MqttService& mqttService)
    : _modemHal(modemHal),
      _sensorHal(sensorHal),
      _gpsHal(gpsHal),
      _powerHal(powerHal),
      _gprsManager(gprsManager),
      _mqttService(mqttService),
      _state(AppState::INIT),
      _lastPublishTime(0),
      _publishInterval(PUBLISH_INTERVAL_MS),
      _trashCanHeight(TRASH_CAN_HEIGHT_CM),
      _initialized(false),
      _firstRun(true) {
    
    // Initialize last readings
    memset(&_lastReadings, 0, sizeof(_lastReadings));
}

bool SmartWasteApp::init() {
    DEBUG_PRINTLN("========================================");
    DEBUG_PRINTLN("  Smart Waste Monitoring System");
    DEBUG_PRINTF("  Firmware: %s\n", FIRMWARE_VERSION);
    DEBUG_PRINTF("  Device ID: %s\n", DEVICE_ID);
    DEBUG_PRINTLN("========================================");
    
    _state = AppState::INIT;
    
    // Initialize hardware
    if (!initHardware()) {
        DEBUG_PRINTLN("[App] Hardware initialization failed");
        _state = AppState::ERROR;
        return false;
    }
    
    // Initialize network
    if (!initNetwork()) {
        DEBUG_PRINTLN("[App] Network initialization failed");
        _state = AppState::ERROR;
        return false;
    }
    
    _initialized = true;
    _state = AppState::IDLE;
    _lastPublishTime = millis();  // Will be overwritten after first publish
    // _firstRun flag (set in constructor) ensures immediate first publish
    
    DEBUG_PRINTLN("[App] Initialization complete");
    blinkLed(3, 200, 200); // Success indication
    
    return true;
}

bool SmartWasteApp::initHardware() {
    DEBUG_PRINTLN("[App] Initializing hardware...");
    
    // Initialize modem
    if (!_modemHal.init()) {
        DEBUG_PRINTLN("[App] Modem init failed");
        return false;
    }
    
    // Check SIM
    if (!_modemHal.checkSim(SIM_PIN)) {
        DEBUG_PRINTLN("[App] SIM check failed");
        return false;
    }
    
    // Initialize ultrasonic sensor
    if (!_sensorHal.init()) {
        DEBUG_PRINTLN("[App] Sensor init failed");
        // Continue anyway - sensor might work later
    }
    
    // Initialize power monitoring
    if (!_powerHal.init()) {
        DEBUG_PRINTLN("[App] Power init failed");
        // Continue - will use simulated values
    }
    
    DEBUG_PRINTLN("[App] Hardware initialized");
    return true;
}

bool SmartWasteApp::initNetwork() {
    DEBUG_PRINTLN("[App] Initializing network...");
    
    // Initialize GPRS
    if (!_gprsManager.init(GPRS_APN, GPRS_USER, GPRS_PASS)) {
        DEBUG_PRINTLN("[App] GPRS init failed");
        return false;
    }
    
    // Connect to GPRS
    if (!_gprsManager.connect(NETWORK_TIMEOUT_MS)) {
        DEBUG_PRINTLN("[App] GPRS connection failed");
        return false;
    }
    
    // Initialize GPS (if enabled)
#if GPS_ENABLED
    if (!_gpsHal.init(GPS_TIMEOUT_MS)) {
        DEBUG_PRINTLN("[App] GPS init failed - will use default location");
        // Continue - will use default coordinates
    }
#else
    DEBUG_PRINTLN("[App] GPS disabled in config - using fixed coordinates");
    DEBUG_PRINTF("[App] Location: %.6f, %.6f\n", DEFAULT_LATITUDE, DEFAULT_LONGITUDE);
#endif
    
    // Initialize MQTT
    if (!_mqttService.init(MQTT_BROKER, MQTT_PORT, MQTT_CLIENT_ID, 
                           MQTT_USER, MQTT_PASS)) {
        DEBUG_PRINTLN("[App] MQTT init failed");
        return false;
    }
    
    // Connect to MQTT broker
    if (!_mqttService.connect()) {
        DEBUG_PRINTLN("[App] MQTT connection failed");
        return false;
    }
    
    DEBUG_PRINTLN("[App] Network initialized");
    return true;
}

void SmartWasteApp::run() {
    static uint32_t lastDebugPrint = 0;
    
    if (!_initialized) {
        DEBUG_PRINTLN("[App] Cannot run - not initialized");
        return;
    }
    
    // Process MQTT messages
    _mqttService.loop();
    
    switch (_state) {
        case AppState::IDLE:
            // Print debug info every 10 seconds while idle
            if (millis() - lastDebugPrint >= 10000) {
                uint32_t elapsed = millis() - _lastPublishTime;
                uint32_t remaining = (_publishInterval > elapsed) ? (_publishInterval - elapsed) : 0;
                DEBUG_PRINTF("[App] IDLE - Next publish in %lu seconds\n", remaining / 1000);
                lastDebugPrint = millis();
            }
            
            if (shouldPublish()) {
                DEBUG_PRINTLN("[App] Time to publish!");
                _state = AppState::READING_SENSORS;
            }
            break;
            
        case AppState::READING_SENSORS:
            DEBUG_PRINTLN("[App] Reading sensors...");
            _lastReadings = readSensors();
            _state = AppState::PUBLISHING;
            break;
            
        case AppState::PUBLISHING:
            DEBUG_PRINTLN("[App] Publishing data...");
            if (publishData(_lastReadings)) {
                _lastPublishTime = millis();
                _firstRun = false;  // Clear first run flag after successful publish
                DEBUG_PRINTLN("[App] Publish successful!");
                blinkLed(1, 100, 0); // Success blink
            } else {
                DEBUG_PRINTLN("[App] Publish failed!");
                blinkLed(5, 50, 50); // Error blink
            }
            _state = AppState::IDLE;
            break;
            
        case AppState::ERROR:
            handleError();
            break;
            
        case AppState::SLEEP:
            // Not implemented - for future deep sleep support
            break;
            
        default:
            _state = AppState::IDLE;
            break;
    }
}

AppState SmartWasteApp::getState() {
    return _state;
}

SensorReadings SmartWasteApp::getLastReadings() {
    return _lastReadings;
}

void SmartWasteApp::forcePublish() {
    _lastPublishTime = 0; // Force immediate publish
}

void SmartWasteApp::setPublishInterval(uint32_t intervalMs) {
    _publishInterval = intervalMs;
}

void SmartWasteApp::setTrashCanHeight(float heightCm) {
    _trashCanHeight = heightCm;
}

SensorReadings SmartWasteApp::readSensors() {
    SensorReadings readings;
    readings.timestamp = millis();
    
    // Read distance sensor
    DEBUG_PRINTLN("[App] Reading ultrasonic sensor...");
    HAL::DistanceReading distReading = _sensorHal.getDistanceAvg(ULTRASONIC_NUM_SAMPLES);
    
    if (distReading.valid) {
        readings.distanceCm = distReading.distanceCm;
        readings.fillLevel = calculateFillLevel(distReading.distanceCm);
        DEBUG_PRINTF("[App] Sensor OK: %.2f cm, %d%% full\n", readings.distanceCm, readings.fillLevel);
    } else {
        // Sensor broken/disconnected - still publish with -1 to indicate failure
        readings.distanceCm = -1;
        readings.fillLevel = -1;
        DEBUG_PRINTLN("[App] WARNING: Sensor FAILED - publishing fill_level=-1 to indicate broken sensor");
    }
    
    // Read GPS location (if enabled)
#if GPS_ENABLED
    DEBUG_PRINTLN("[App] Reading GPS location...");
    HAL::GpsLocation gpsLoc = _gpsHal.getLocation(GPS_TIMEOUT_MS);
    readings.latitude = gpsLoc.latitude;
    readings.longitude = gpsLoc.longitude;
    readings.gpsValid = gpsLoc.valid;
    
    if (!gpsLoc.valid) {
        DEBUG_PRINTLN("[App] GPS timeout - using default coordinates");
    }
#else
    // GPS disabled - use fixed coordinates from config
    DEBUG_PRINTLN("[App] GPS disabled - using fixed coordinates");
    readings.latitude = DEFAULT_LATITUDE;
    readings.longitude = DEFAULT_LONGITUDE;
    readings.gpsValid = false;
#endif
    
    // Read battery level
    DEBUG_PRINTLN("[App] Reading battery level...");
    HAL::BatteryStatus battery = _powerHal.getBatteryStatus();
    readings.batteryLevel = battery.percentage;
    
    // Log final readings summary
    DEBUG_PRINTLN("[App] === SENSOR READINGS COMPLETE ===");
    DEBUG_PRINTF("  Distance: %.2f cm %s\n", readings.distanceCm, 
                 readings.distanceCm < 0 ? "(SENSOR ERROR)" : "");
    DEBUG_PRINTF("  Fill Level: %d%% %s\n", readings.fillLevel,
                 readings.fillLevel < 0 ? "(SENSOR ERROR)" : "");
    DEBUG_PRINTF("  Location: %.6f, %.6f (%s)\n", 
                 readings.latitude, readings.longitude,
                 readings.gpsValid ? "GPS" : "default");
    DEBUG_PRINTF("  Battery: %d%%\n", readings.batteryLevel);
    DEBUG_PRINTLN("[App] ================================");
    
    return readings;
}

int8_t SmartWasteApp::calculateFillLevel(float distanceCm) {
    if (distanceCm < 0) {
        return -1; // Invalid reading
    }
    
    // Fill level = (max_height - measured_distance) / max_height * 100
    // When trash can is empty, distance is large (near max height)
    // When trash can is full, distance is small (near sensor)
    
    float fillLevel = ((_trashCanHeight - distanceCm) / _trashCanHeight) * 100.0f;
    
    // Clamp to 0-100 range
    if (fillLevel < 0) fillLevel = 0;
    if (fillLevel > 100) fillLevel = 100;
    
    return (int8_t)fillLevel;
}

bool SmartWasteApp::publishData(const SensorReadings& readings) {
    // Ensure network connection
    if (!_gprsManager.ensureConnection()) {
        DEBUG_PRINTLN("[App] Network connection lost");
        return false;
    }
    
    if (!_mqttService.ensureConnection()) {
        DEBUG_PRINTLN("[App] MQTT connection lost");
        return false;
    }
    
    // Build payload
    Network::SensorPayload payload;
    payload.deviceId = DEVICE_ID;
    payload.latitude = readings.latitude;
    payload.longitude = readings.longitude;
    payload.batteryLevel = readings.batteryLevel;
    payload.fillLevel = readings.fillLevel;
    
    // Publish
    return _mqttService.publishSensorData(payload);
}

bool SmartWasteApp::shouldPublish() {
    // Always publish on first run
    if (_firstRun) {
        DEBUG_PRINTLN("[App] First run - triggering immediate publish");
        return true;
    }
    
    return (millis() - _lastPublishTime >= _publishInterval);
}

void SmartWasteApp::handleError() {
    DEBUG_PRINTLN("[App] Handling error state...");
    
    blinkLed(10, 50, 50); // Error indication
    
    // Try to recover
    if (!_modemHal.isReady()) {
        DEBUG_PRINTLN("[App] Attempting modem recovery...");
        _modemHal.restart();
    }
    
    if (!_gprsManager.isConnected()) {
        DEBUG_PRINTLN("[App] Attempting network recovery...");
        _gprsManager.connect(NETWORK_TIMEOUT_MS);
    }
    
    if (!_mqttService.isConnected()) {
        DEBUG_PRINTLN("[App] Attempting MQTT recovery...");
        _mqttService.connect();
    }
    
    // If all recovered, go back to IDLE
    if (_modemHal.isReady() && _gprsManager.isConnected()) {
        _state = AppState::IDLE;
        DEBUG_PRINTLN("[App] Recovery successful");
    } else {
        // Wait before retry
        delay(10000);
    }
}

void SmartWasteApp::blinkLed(int times, int onMs, int offMs) {
    for (int i = 0; i < times; i++) {
        Drivers::GpioDriver::writeDigital(BOARD_LED_PIN, LED_ON);
        delay(onMs);
        Drivers::GpioDriver::writeDigital(BOARD_LED_PIN, LED_OFF);
        if (i < times - 1 && offMs > 0) {
            delay(offMs);
        }
    }
}

} // namespace App
