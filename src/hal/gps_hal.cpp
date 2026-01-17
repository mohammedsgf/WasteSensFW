/**
 * @file gps_hal.cpp
 * @brief GPS HAL implementation
 */

#include "gps_hal.h"
#include "config.h"

namespace HAL {

GpsHAL::GpsHAL(Drivers::SIM7000Driver& driver)
    : _driver(driver), _enabled(false), 
      _defaultLat(DEFAULT_LATITUDE), _defaultLon(DEFAULT_LONGITUDE) {
}

bool GpsHAL::init(uint32_t timeout) {
    DEBUG_PRINTLN("[GpsHAL] Initializing GPS...");
    DEBUG_PRINTLN("[GpsHAL] Make sure GPS antenna is connected!");
    
    if (!enable()) {
        DEBUG_PRINTLN("[GpsHAL] Failed to enable GPS");
        return false;
    }
    
    DEBUG_PRINTLN("[GpsHAL] GPS initialized - ready for positioning");
    DEBUG_PRINTLN("[GpsHAL] Note: First fix may take 1-3 minutes outdoors");
    
    return true;
}

bool GpsHAL::enable() {
    DEBUG_PRINTLN("[GpsHAL] Enabling GPS...");
    
    TinyGsm& modem = _driver.getModem();
    
    // SIM7000G specific: Power on GPS module via GPIO48
    // This AT command is essential for SIM7000G GPS to work!
    DEBUG_PRINTLN("[GpsHAL] Powering on GPS module (AT+CGPIO)...");
    modem.sendAT("+CGPIO=0,48,1,1");
    if (modem.waitResponse(10000L) != 1) {
        DEBUG_PRINTLN("[GpsHAL] Warning: GPS power command failed");
        // Continue anyway, might still work
    }
    
    // Enable GPS via standard TinyGSM command
    DEBUG_PRINTLN("[GpsHAL] Enabling GPS (AT+CGNSPWR)...");
    if (!modem.enableGPS()) {
        DEBUG_PRINTLN("[GpsHAL] Failed to enable GPS");
        return false;
    }
    
    _enabled = true;
    DEBUG_PRINTLN("[GpsHAL] GPS enabled successfully");
    return true;
}

void GpsHAL::disable() {
    DEBUG_PRINTLN("[GpsHAL] Disabling GPS...");
    
    TinyGsm& modem = _driver.getModem();
    
    // SIM7000G specific: Power off GPS module via GPIO48
    modem.sendAT("+CGPIO=0,48,1,0");
    modem.waitResponse(10000L);
    
    // Disable GPS via standard TinyGSM command
    modem.disableGPS();
    _enabled = false;
    
    DEBUG_PRINTLN("[GpsHAL] GPS disabled");
}

bool GpsHAL::isEnabled() {
    return _enabled;
}

GpsLocation GpsHAL::getLocation(uint32_t timeout) {
    GpsLocation location;
    location.timestamp = millis();
    location.valid = false;
    location.latitude = _defaultLat;
    location.longitude = _defaultLon;
    location.altitude = 0;
    location.speed = 0;
    location.accuracy = 0;
    location.satellites = 0;
    
    if (!_enabled) {
        DEBUG_PRINTLN("[GpsHAL] GPS not enabled");
        return location;
    }
    
    TinyGsm& modem = _driver.getModem();
    
    float lat = 0, lon = 0, speed = 0, alt = 0, accuracy = 0;
    int vsat = 0, usat = 0;
    int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
    uint8_t status = 0;
    
    uint32_t startTime = millis();
    int attempts = 0;
    
    DEBUG_PRINTLN("[GpsHAL] Waiting for GPS fix (blue LED will blink)...");
    
    while (millis() - startTime < timeout) {
        // Use full getGPS call with all parameters (TinyGSM fork requirement)
        if (modem.getGPS(&status, &lat, &lon, &speed, &alt, &vsat, &usat, &accuracy,
                         &year, &month, &day, &hour, &minute, &second)) {
            location.valid = true;
            location.latitude = lat;
            location.longitude = lon;
            location.altitude = alt;
            location.speed = speed;
            location.accuracy = accuracy;
            location.satellites = vsat;
            
            DEBUG_PRINTF("[GpsHAL] Fix obtained: %.6f, %.6f (sats: %d)\n", lat, lon, vsat);
            return location;
        }
        
        attempts++;
        if (attempts % 5 == 0) {
            DEBUG_PRINTF("[GpsHAL] Still waiting for fix... (%lu sec)\n", 
                        (millis() - startTime) / 1000);
        }
        
        delay(2000);  // Match working example's 2 second delay
    }
    
    DEBUG_PRINTLN("[GpsHAL] GPS fix timeout, using default location");
    return location;
}

bool GpsHAL::waitForFix(uint32_t timeout) {
    GpsLocation loc = getLocation(timeout);
    return loc.valid;
}

String GpsHAL::getRawGps() {
    if (!_enabled) return "";
    return _driver.getModem().getGPSraw();
}

void GpsHAL::setDefaultLocation(float lat, float lon) {
    _defaultLat = lat;
    _defaultLon = lon;
}

GpsLocation GpsHAL::getDefaultLocation() {
    GpsLocation location;
    location.valid = false;
    location.latitude = _defaultLat;
    location.longitude = _defaultLon;
    location.altitude = 0;
    location.speed = 0;
    location.accuracy = 0;
    location.satellites = 0;
    location.timestamp = millis();
    return location;
}

} // namespace HAL
