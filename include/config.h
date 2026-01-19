/**
 * @file config.h
 * @brief Global configuration for Smart Waste Monitoring Firmware
 */

#ifndef CONFIG_H
#define CONFIG_H

// =============================================================================
// DEVICE CONFIGURATION
// =============================================================================
#define DEVICE_ID               "smartwaste_001"
#define FIRMWARE_VERSION        "1.0.0"

// =============================================================================
// MODEM PIN CONFIGURATION (T-SIM7000G)
// =============================================================================
#define MODEM_TX_PIN            27
#define MODEM_RX_PIN            26
#define MODEM_PWRKEY_PIN        4
#define MODEM_DTR_PIN           25
#define MODEM_RING_PIN          33
#define MODEM_BAUDRATE          115200

// LED Configuration
#define BOARD_LED_PIN           12
#define LED_ON                  LOW
#define LED_OFF                 HIGH

// =============================================================================
// ULTRASONIC SENSOR CONFIGURATION (US-100 GPIO Mode)
// =============================================================================
#define US100_TRIGGER_PIN       32
#define US100_ECHO_PIN          35  // Using input-only pin for echo

// Trash can dimensions (in centimeters)
#define TRASH_CAN_HEIGHT_CM     120.0f
#define SENSOR_MIN_DISTANCE_CM  2.0f
#define SENSOR_MAX_DISTANCE_CM  400.0f

// Measurement settings
#define US100_TIMEOUT_US        30000   // 30ms timeout for echo
#define US100_NUM_SAMPLES       5       // Number of samples for averaging

// =============================================================================
// GPS CONFIGURATION
// =============================================================================
// Set to 0 to disable GPS completely (use fixed coordinates)
// Set to 1 to enable GPS (requires clear sky view, may take 2-3 min for first fix)
//#define GPS_ENABLED             1       // Disabled by default - set to 1 to enable

#define GPS_ENABLE_GPIO         4
#define GPS_ENABLE_LEVEL        0
#define GPS_TIMEOUT_MS          30000   // 30 seconds timeout (reduced from 2 min)

// Default/Fixed location (used when GPS disabled or fails)
// Set your actual coordinates here
#define DEFAULT_LATITUDE        24.7136f    // Example: Riyadh, Saudi Arabia
#define DEFAULT_LONGITUDE       46.6753f

// =============================================================================
// NETWORK CONFIGURATION
// =============================================================================
// APN Settings - Modify according to your carrier
#define GPRS_APN                "jawalnet.com.sa"
#define GPRS_USER               ""
#define GPRS_PASS               ""

// SIM PIN (leave empty if not required)
#define SIM_PIN                 ""

// =============================================================================
// MQTT CONFIGURATION
// =============================================================================
#define MQTT_BROKER             "test.mosquitto.org"
#define MQTT_PORT               1883
#define MQTT_CLIENT_ID          DEVICE_ID
#define MQTT_USER               ""
#define MQTT_PASS               ""

// Topic format: smartwaste/{device_id}/data
#define MQTT_TOPIC_PREFIX       "smartwaste"
#define MQTT_TOPIC_SUFFIX       "data"

// =============================================================================
// TIMING CONFIGURATION
// =============================================================================
#define PUBLISH_INTERVAL_MS     1000
#define MQTT_RECONNECT_DELAY_MS 10000   // 10 seconds
#define NETWORK_TIMEOUT_MS      180000  // 3 minutes
#define MODEM_INIT_DELAY_MS     3000    // 3 seconds

// =============================================================================
// BATTERY CONFIGURATION
// =============================================================================
// Note: T-SIM7000G doesn't have dedicated battery ADC pin on standard version
// If using a voltage divider, configure these values
#define BATTERY_ADC_PIN         -1      // Set to actual pin if available
#define BATTERY_VOLTAGE_DIVIDER 2.0f    // Voltage divider ratio
#define BATTERY_MIN_VOLTAGE     3.3f    // Minimum voltage (0%)
#define BATTERY_MAX_VOLTAGE     4.2f    // Maximum voltage (100%)

// =============================================================================
// DEBUG CONFIGURATION
// =============================================================================
#define SERIAL_DEBUG            1
#define DUMP_AT_COMMANDS        0       // Set to 1 to see AT commands

#if SERIAL_DEBUG
    #define DEBUG_PRINT(x)      Serial.print(x)
    #define DEBUG_PRINTLN(x)    Serial.println(x)
    #define DEBUG_PRINTF(...)   Serial.printf(__VA_ARGS__)
#else
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINTLN(x)
    #define DEBUG_PRINTF(...)
#endif

#endif // CONFIG_H
