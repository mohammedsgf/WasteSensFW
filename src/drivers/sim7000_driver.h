/**
 * @file sim7000_driver.h
 * @brief SIM7000G Modem Driver - Low-level AT command interface
 */

#ifndef SIM7000_DRIVER_H
#define SIM7000_DRIVER_H

#include <Arduino.h>
#include "config.h"

// TinyGSM configuration (may also be defined in build flags)
#ifndef TINY_GSM_MODEM_SIM7000SSL
#define TINY_GSM_MODEM_SIM7000SSL
#endif

#ifndef TINY_GSM_RX_BUFFER
#define TINY_GSM_RX_BUFFER 1024
#endif

#include <TinyGsmClient.h>

#if DUMP_AT_COMMANDS
#include <StreamDebugger.h>
#endif

namespace Drivers {

/**
 * @brief SIM7000G Modem Driver
 * 
 * Provides low-level modem control and AT command interface
 * using TinyGSM library
 */
class SIM7000Driver {
public:
    /**
     * @brief Constructor
     * @param serial Hardware serial for AT communication
     */
    SIM7000Driver(HardwareSerial& serial);

    /**
     * @brief Destructor
     */
    ~SIM7000Driver();

    /**
     * @brief Initialize modem hardware (power pins, serial)
     * @return true if hardware initialized successfully
     */
    bool initHardware();

    /**
     * @brief Power on the modem
     * @return true if modem powered on
     */
    bool powerOn();

    /**
     * @brief Power off the modem
     */
    void powerOff();

    /**
     * @brief Reset the modem
     */
    void reset();

    /**
     * @brief Initialize modem communication
     * @return true if modem responds to AT commands
     */
    bool initModem();

    /**
     * @brief Test if modem responds to AT command
     * @param timeout Timeout in milliseconds
     * @return true if modem responds
     */
    bool testAT(uint32_t timeout = 1000);

    /**
     * @brief Get TinyGSM modem instance
     * @return Reference to TinyGsm object
     */
    TinyGsm& getModem();

    /**
     * @brief Get modem name
     * @return Modem name string
     */
    String getModemName();

    /**
     * @brief Get modem info
     * @return Modem info string
     */
    String getModemInfo();

    /**
     * @brief Get SIM status
     * @return SIM status code
     */
    int getSimStatus();

    /**
     * @brief Unlock SIM with PIN
     * @param pin SIM PIN code
     * @return true if unlock successful
     */
    bool unlockSim(const char* pin);

    /**
     * @brief Send raw AT command
     * @param cmd AT command to send
     */
    void sendAT(const String& cmd);

    /**
     * @brief Wait for response
     * @param timeout Timeout in milliseconds
     * @return Response status
     */
    int8_t waitResponse(uint32_t timeout = 1000);

private:
    HardwareSerial& _serial;
    TinyGsm* _modem;
    
#if DUMP_AT_COMMANDS
    StreamDebugger* _debugger;
#endif

    bool _initialized;
};

} // namespace Drivers

#endif // SIM7000_DRIVER_H
