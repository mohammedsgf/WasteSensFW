/**
 * @file main.cpp
 * @brief Smart Waste Monitoring Firmware - Entry Point
 * 
 * Target: LilyGo T-SIM7000G (ESP32 + SIM7000G)
 * 
 * This firmware monitors trash bin fill levels using an ultrasonic sensor,
 * retrieves GPS location, and publishes telemetry data to an MQTT broker
 * over cellular network.
 * 
 * Architecture:
 * - Device Drivers: Low-level hardware access
 * - HAL: Hardware abstraction
 * - Network: GPRS and MQTT communication
 * - App: Main application logic
 */

#include <Arduino.h>
#include "config.h"

// Drivers
#include "drivers/gpio_driver.h"
#include "drivers/adc_driver.h"
#include "drivers/us100_driver.h"
#include "drivers/sim7000_driver.h"

// HAL
#include "hal/modem_hal.h"
#include "hal/sensor_hal.h"
#include "hal/gps_hal.h"
#include "hal/power_hal.h"

// Network
#include "network/gprs_manager.h"
#include "network/mqtt_service.h"

// App
#include "app/smart_waste_app.h"

// =============================================================================
// Global Objects
// =============================================================================

// Hardware Serial for modem communication
HardwareSerial SerialAT(1);

// Device Drivers
Drivers::US100Driver us100Driver(US100_TRIGGER_PIN, US100_ECHO_PIN);
Drivers::SIM7000Driver sim7000Driver(SerialAT);

// Hardware Abstraction Layer
HAL::ModemHAL modemHal(sim7000Driver);
HAL::SensorHAL sensorHal(us100Driver);
HAL::GpsHAL gpsHal(sim7000Driver);
HAL::PowerHAL powerHal(BATTERY_ADC_PIN, BATTERY_VOLTAGE_DIVIDER);

// Network Layer
Network::GprsManager gprsManager(modemHal);
Network::MqttService mqttService(gprsManager);

// Application Layer
App::SmartWasteApp app(modemHal, sensorHal, gpsHal, powerHal, gprsManager, mqttService);

// =============================================================================
// Setup
// =============================================================================

void setup() {
    // Initialize debug serial
    Serial.begin(115200);
    delay(100);
    
    Serial.println();
    Serial.println("========================================");
    Serial.println("  Smart Waste Monitoring System");
    Serial.println("  Starting up...");
    Serial.println("========================================");
    
    // Initialize application
    if (!app.init()) {
        Serial.println("ERROR: Application initialization failed!");
        Serial.println("System will attempt recovery...");
    }
}

// =============================================================================
// Main Loop
// =============================================================================

void loop() {
    // Run application
    app.run();
    
    // Small delay to prevent watchdog issues
    delay(10);
}

// =============================================================================
// TinyGSM Fork Library Check
// =============================================================================

#ifndef TINY_GSM_FORK_LIBRARY
#error "TinyGSM fork library not detected. Please ensure you have copied the lib/TinyGSM folder from the LilyGo-Modem-Series repository."
#endif
