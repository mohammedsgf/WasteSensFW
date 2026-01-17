/**
 * @file modem_hal.cpp
 * @brief Modem HAL implementation
 */

#include "modem_hal.h"
#include "config.h"

namespace HAL {

ModemHAL::ModemHAL(Drivers::SIM7000Driver& driver)
    : _driver(driver), _status(ModemStatus::OFF) {
}

bool ModemHAL::init() {
    _status = ModemStatus::INITIALIZING;
    DEBUG_PRINTLN("[ModemHAL] Initializing modem...");
    
    // Initialize hardware
    if (!_driver.initHardware()) {
        _status = ModemStatus::ERROR;
        DEBUG_PRINTLN("[ModemHAL] Hardware init failed");
        return false;
    }
    
    // Power on modem
    if (!_driver.powerOn()) {
        _status = ModemStatus::ERROR;
        DEBUG_PRINTLN("[ModemHAL] Power on failed");
        return false;
    }
    
    // Initialize modem communication
    if (!_driver.initModem()) {
        _status = ModemStatus::ERROR;
        DEBUG_PRINTLN("[ModemHAL] Modem init failed");
        return false;
    }
    
    _status = ModemStatus::READY;
    DEBUG_PRINTLN("[ModemHAL] Modem ready");
    DEBUG_PRINTF("[ModemHAL] Name: %s\n", _driver.getModemName().c_str());
    DEBUG_PRINTF("[ModemHAL] Info: %s\n", _driver.getModemInfo().c_str());
    
    return true;
}

bool ModemHAL::isReady() {
    return _status == ModemStatus::READY;
}

ModemStatus ModemHAL::getStatus() {
    return _status;
}

TinyGsm& ModemHAL::getModem() {
    return _driver.getModem();
}

String ModemHAL::getInfo() {
    return _driver.getModemName() + " - " + _driver.getModemInfo();
}

bool ModemHAL::checkSim(const char* pin) {
    DEBUG_PRINTLN("[ModemHAL] Checking SIM...");
    
    int simStatus = _driver.getSimStatus();
    DEBUG_PRINTF("[ModemHAL] SIM status: %d\n", simStatus);
    
    // TinyGSM SimStatus enum:
    // 0 = SIM_ERROR
    // 1 = SIM_READY
    // 2 = SIM_LOCKED (needs PIN)
    // 3 = SIM_ANTITHEFT_LOCKED
    
    if (simStatus == 1) {
        DEBUG_PRINTLN("[ModemHAL] SIM ready");
        return true;
    }
    
    // SIM is locked, try to unlock if PIN provided
    if (simStatus == 2 && pin && strlen(pin) > 0) {
        DEBUG_PRINTLN("[ModemHAL] SIM locked, unlocking...");
        if (_driver.unlockSim(pin)) {
            DEBUG_PRINTLN("[ModemHAL] SIM unlocked");
            return true;
        }
        DEBUG_PRINTLN("[ModemHAL] SIM unlock failed");
    }
    
    // Log specific error
    if (simStatus == 0) {
        DEBUG_PRINTLN("[ModemHAL] SIM error - check if SIM is inserted");
    } else if (simStatus == 2) {
        DEBUG_PRINTLN("[ModemHAL] SIM locked - PIN required, set SIM_PIN in config.h");
    } else if (simStatus == 3) {
        DEBUG_PRINTLN("[ModemHAL] SIM antitheft locked");
    }
    
    return false;
}

void ModemHAL::restart() {
    DEBUG_PRINTLN("[ModemHAL] Restarting modem...");
    _status = ModemStatus::INITIALIZING;
    _driver.reset();
    
    if (_driver.initModem()) {
        _status = ModemStatus::READY;
    } else {
        _status = ModemStatus::ERROR;
    }
}

void ModemHAL::sleep() {
    DEBUG_PRINTLN("[ModemHAL] Entering sleep mode...");
    _driver.getModem().sleepEnable(true);
}

void ModemHAL::wake() {
    DEBUG_PRINTLN("[ModemHAL] Waking from sleep...");
    _driver.getModem().sleepEnable(false);
}

} // namespace HAL
