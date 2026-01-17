/**
 * @file gprs_manager.h
 * @brief GPRS Connection Manager
 */

#ifndef GPRS_MANAGER_H
#define GPRS_MANAGER_H

#include <Arduino.h>
#include "../hal/modem_hal.h"

namespace Network {

/**
 * @brief GPRS connection state
 */
enum class GprsState {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    ERROR
};

/**
 * @brief Network information
 */
struct NetworkInfo {
    bool registered;
    String operatorName;
    int signalQuality;  // 0-31, or 99 if unknown
    String ipAddress;
};

/**
 * @brief GPRS Connection Manager
 */
class GprsManager {
public:
    /**
     * @brief Constructor
     * @param modemHal Reference to modem HAL
     */
    GprsManager(HAL::ModemHAL& modemHal);

    /**
     * @brief Initialize GPRS manager
     * @param apn Access Point Name
     * @param user Username (optional)
     * @param pass Password (optional)
     * @return true if initialization successful
     */
    bool init(const char* apn, const char* user = "", const char* pass = "");

    /**
     * @brief Connect to GPRS network
     * @param timeout Connection timeout in milliseconds
     * @return true if connected successfully
     */
    bool connect(uint32_t timeout = 180000);

    /**
     * @brief Disconnect from GPRS network
     */
    void disconnect();

    /**
     * @brief Check if connected to network
     * @return true if connected
     */
    bool isConnected();

    /**
     * @brief Get connection state
     * @return Current GPRS state
     */
    GprsState getState();

    /**
     * @brief Wait for network registration
     * @param timeout Timeout in milliseconds
     * @return true if registered
     */
    bool waitForNetwork(uint32_t timeout = 180000);

    /**
     * @brief Get network information
     * @return Network info structure
     */
    NetworkInfo getNetworkInfo();

    /**
     * @brief Get signal quality (0-31)
     * @return Signal quality, or -1 if error
     */
    int getSignalQuality();

    /**
     * @brief Ensure connection is active, reconnect if needed
     * @return true if connection is active
     */
    bool ensureConnection();

    /**
     * @brief Get TinyGSM client for TCP/IP
     * @return TinyGsmClient reference
     */
    TinyGsmClient& getClient();

private:
    HAL::ModemHAL& _modemHal;
    TinyGsmClient* _client;
    GprsState _state;
    String _apn;
    String _user;
    String _pass;
};

} // namespace Network

#endif // GPRS_MANAGER_H
