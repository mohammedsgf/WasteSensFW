/**
 * @file sim7000_driver.cpp
 * @brief SIM7000G Modem Driver implementation
 */

#include "sim7000_driver.h"
#include "gpio_driver.h"

namespace Drivers {

SIM7000Driver::SIM7000Driver(HardwareSerial& serial)
    : _serial(serial), _modem(nullptr), _initialized(false) {
#if DUMP_AT_COMMANDS
    _debugger = nullptr;
#endif
}

SIM7000Driver::~SIM7000Driver() {
    if (_modem) {
        delete _modem;
        _modem = nullptr;
    }
#if DUMP_AT_COMMANDS
    if (_debugger) {
        delete _debugger;
        _debugger = nullptr;
    }
#endif
}

bool SIM7000Driver::initHardware() {
    DEBUG_PRINTLN("[SIM7000] Initializing hardware...");
    
    // Configure power key pin
    GpioDriver::configurePin(MODEM_PWRKEY_PIN, PinMode::OUTPUT_MODE);
    GpioDriver::writeDigital(MODEM_PWRKEY_PIN, LOW);
    
    // Configure LED pin
    GpioDriver::configurePin(BOARD_LED_PIN, PinMode::OUTPUT_MODE);
    GpioDriver::writeDigital(BOARD_LED_PIN, LED_OFF);
    
    // Initialize serial communication
    _serial.begin(MODEM_BAUDRATE, SERIAL_8N1, MODEM_RX_PIN, MODEM_TX_PIN);
    
    DEBUG_PRINTLN("[SIM7000] Hardware initialized");
    return true;
}

bool SIM7000Driver::powerOn() {
    DEBUG_PRINTLN("[SIM7000] Powering on modem...");
    
    // Power on sequence for SIM7000G
    GpioDriver::writeDigital(MODEM_PWRKEY_PIN, LOW);
    delay(100);
    GpioDriver::writeDigital(MODEM_PWRKEY_PIN, HIGH);
    delay(1000);
    GpioDriver::writeDigital(MODEM_PWRKEY_PIN, LOW);
    
    // Wait for modem to boot
    delay(MODEM_INIT_DELAY_MS);
    
    // Turn on LED to indicate modem power
    GpioDriver::writeDigital(BOARD_LED_PIN, LED_ON);
    
    DEBUG_PRINTLN("[SIM7000] Power on sequence complete");
    return true;
}

void SIM7000Driver::powerOff() {
    DEBUG_PRINTLN("[SIM7000] Powering off modem...");
    
    if (_modem) {
        _modem->poweroff();
    }
    
    // Hardware power off
    GpioDriver::writeDigital(MODEM_PWRKEY_PIN, HIGH);
    delay(1500);
    GpioDriver::writeDigital(MODEM_PWRKEY_PIN, LOW);
    
    GpioDriver::writeDigital(BOARD_LED_PIN, LED_OFF);
    
    DEBUG_PRINTLN("[SIM7000] Modem powered off");
}

void SIM7000Driver::reset() {
    DEBUG_PRINTLN("[SIM7000] Resetting modem...");
    
    powerOff();
    delay(1000);
    powerOn();
}

bool SIM7000Driver::initModem() {
    DEBUG_PRINTLN("[SIM7000] Initializing modem communication...");
    
    // Create TinyGSM instance
#if DUMP_AT_COMMANDS
    if (!_debugger) {
        _debugger = new StreamDebugger(_serial, Serial);
    }
    if (!_modem) {
        _modem = new TinyGsm(*_debugger);
    }
#else
    if (!_modem) {
        _modem = new TinyGsm(_serial);
    }
#endif

    // Try to initialize modem
    int retry = 0;
    while (!testAT(1000)) {
        DEBUG_PRINT(".");
        if (retry++ > 10) {
            DEBUG_PRINTLN("\n[SIM7000] Modem not responding, power cycling...");
            reset();
            retry = 0;
        }
    }
    DEBUG_PRINTLN();
    
    // Initialize modem
    if (!_modem->init()) {
        DEBUG_PRINTLN("[SIM7000] Failed to initialize modem");
        return false;
    }
    
    _initialized = true;
    DEBUG_PRINTLN("[SIM7000] Modem initialized successfully");
    
    return true;
}

bool SIM7000Driver::testAT(uint32_t timeout) {
    if (!_modem) return false;
    return _modem->testAT(timeout);
}

TinyGsm& SIM7000Driver::getModem() {
    return *_modem;
}

String SIM7000Driver::getModemName() {
    if (!_modem) return "UNKNOWN";
    return _modem->getModemName();
}

String SIM7000Driver::getModemInfo() {
    if (!_modem) return "UNKNOWN";
    return _modem->getModemInfo();
}

int SIM7000Driver::getSimStatus() {
    if (!_modem) return -1;
    return _modem->getSimStatus();
}

bool SIM7000Driver::unlockSim(const char* pin) {
    if (!_modem || !pin || strlen(pin) == 0) return true;
    return _modem->simUnlock(pin);
}

void SIM7000Driver::sendAT(const String& cmd) {
    if (_modem) {
        _modem->sendAT(cmd.c_str());
    }
}

int8_t SIM7000Driver::waitResponse(uint32_t timeout) {
    if (!_modem) return -1;
    return _modem->waitResponse(timeout);
}

} // namespace Drivers
