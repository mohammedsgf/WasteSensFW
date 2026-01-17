/**
 * @file modem_hal.h
 * @brief Modem Hardware Abstraction Layer
 */

#ifndef MODEM_HAL_H
#define MODEM_HAL_H

#include <Arduino.h>
#include "../drivers/sim7000_driver.h"

namespace HAL {

/**
 * @brief Modem status enumeration
 */
enum class ModemStatus {
    OFF,
    INITIALIZING,
    READY,
    ERROR
};

/**
 * @brief Modem HAL class - abstracts modem operations
 */
class ModemHAL {
public:
    /**
     * @brief Constructor
     * @param driver Reference to SIM7000 driver
     */
    ModemHAL(Drivers::SIM7000Driver& driver);

    /**
     * @brief Initialize modem
     * @return true if initialization successful
     */
    bool init();

    /**
     * @brief Check if modem is ready
     * @return true if modem is ready for operations
     */
    bool isReady();

    /**
     * @brief Get modem status
     * @return Current modem status
     */
    ModemStatus getStatus();

    /**
     * @brief Get underlying TinyGSM modem
     * @return Reference to TinyGsm object
     */
    TinyGsm& getModem();

    /**
     * @brief Get modem information string
     * @return Modem info
     */
    String getInfo();

    /**
     * @brief Check and unlock SIM if needed
     * @param pin SIM PIN (empty if not required)
     * @return true if SIM is ready
     */
    bool checkSim(const char* pin = "");

    /**
     * @brief Power cycle the modem
     */
    void restart();

    /**
     * @brief Put modem to sleep
     */
    void sleep();

    /**
     * @brief Wake modem from sleep
     */
    void wake();

private:
    Drivers::SIM7000Driver& _driver;
    ModemStatus _status;
};

} // namespace HAL

#endif // MODEM_HAL_H
