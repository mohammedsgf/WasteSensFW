/**
 * @file gprs_manager.cpp
 * @brief GPRS Connection Manager implementation
 */

#include "gprs_manager.h"
#include "config.h"

namespace Network {

GprsManager::GprsManager(HAL::ModemHAL& modemHal)
    : _modemHal(modemHal), _client(nullptr), _state(GprsState::DISCONNECTED) {
}

bool GprsManager::init(const char* apn, const char* user, const char* pass) {
    DEBUG_PRINTLN("[GPRS] Initializing...");
    
    _apn = apn;
    _user = user;
    _pass = pass;
    
    // Create client
    if (!_client) {
        _client = new TinyGsmClient(_modemHal.getModem());
    }
    
    DEBUG_PRINTF("[GPRS] APN: %s\n", apn);
    return true;
}

bool GprsManager::connect(uint32_t timeout) {
    if (!_modemHal.isReady()) {
        DEBUG_PRINTLN("[GPRS] Modem not ready");
        _state = GprsState::ERROR;
        return false;
    }
    
    _state = GprsState::CONNECTING;
    DEBUG_PRINTLN("[GPRS] Connecting to network...");
    
    TinyGsm& modem = _modemHal.getModem();
    
    // Wait for network registration
    if (!waitForNetwork(timeout)) {
        DEBUG_PRINTLN("[GPRS] Network registration failed");
        _state = GprsState::ERROR;
        return false;
    }
    
    // Connect to GPRS
    DEBUG_PRINTF("[GPRS] Connecting to APN: %s\n", _apn.c_str());
    
    if (!modem.gprsConnect(_apn.c_str(), _user.c_str(), _pass.c_str())) {
        DEBUG_PRINTLN("[GPRS] GPRS connection failed");
        _state = GprsState::ERROR;
        return false;
    }
    
    if (!modem.isGprsConnected()) {
        DEBUG_PRINTLN("[GPRS] GPRS not connected after connect call");
        _state = GprsState::ERROR;
        return false;
    }
    
    _state = GprsState::CONNECTED;
    DEBUG_PRINTLN("[GPRS] Connected successfully");
    
    // Print connection info
    NetworkInfo info = getNetworkInfo();
    DEBUG_PRINTF("[GPRS] Operator: %s\n", info.operatorName.c_str());
    DEBUG_PRINTF("[GPRS] Signal: %d\n", info.signalQuality);
    DEBUG_PRINTF("[GPRS] IP: %s\n", info.ipAddress.c_str());
    
    return true;
}

void GprsManager::disconnect() {
    DEBUG_PRINTLN("[GPRS] Disconnecting...");
    
    TinyGsm& modem = _modemHal.getModem();
    modem.gprsDisconnect();
    
    _state = GprsState::DISCONNECTED;
    DEBUG_PRINTLN("[GPRS] Disconnected");
}

bool GprsManager::isConnected() {
    if (_state != GprsState::CONNECTED) {
        return false;
    }
    
    // Verify actual connection state
    TinyGsm& modem = _modemHal.getModem();
    bool connected = modem.isGprsConnected();
    
    if (!connected) {
        _state = GprsState::DISCONNECTED;
    }
    
    return connected;
}

GprsState GprsManager::getState() {
    return _state;
}

bool GprsManager::waitForNetwork(uint32_t timeout) {
    DEBUG_PRINTLN("[GPRS] Waiting for network registration...");
    
    TinyGsm& modem = _modemHal.getModem();
    
    if (!modem.waitForNetwork(timeout, true)) {
        DEBUG_PRINTLN("[GPRS] Network registration timeout");
        return false;
    }
    
    if (!modem.isNetworkConnected()) {
        DEBUG_PRINTLN("[GPRS] Network not connected");
        return false;
    }
    
    DEBUG_PRINTLN("[GPRS] Network registered");
    return true;
}

NetworkInfo GprsManager::getNetworkInfo() {
    NetworkInfo info;
    TinyGsm& modem = _modemHal.getModem();
    
    info.registered = modem.isNetworkConnected();
    info.operatorName = modem.getOperator();
    info.signalQuality = modem.getSignalQuality();
    info.ipAddress = modem.localIP().toString();
    
    return info;
}

int GprsManager::getSignalQuality() {
    TinyGsm& modem = _modemHal.getModem();
    return modem.getSignalQuality();
}

bool GprsManager::ensureConnection() {
    if (isConnected()) {
        return true;
    }
    
    DEBUG_PRINTLN("[GPRS] Connection lost, reconnecting...");
    
    // Check network first
    TinyGsm& modem = _modemHal.getModem();
    
    if (!modem.isNetworkConnected()) {
        if (!waitForNetwork(NETWORK_TIMEOUT_MS)) {
            return false;
        }
    }
    
    // Reconnect GPRS
    if (!modem.isGprsConnected()) {
        if (!modem.gprsConnect(_apn.c_str(), _user.c_str(), _pass.c_str())) {
            DEBUG_PRINTLN("[GPRS] Reconnection failed");
            _state = GprsState::ERROR;
            return false;
        }
    }
    
    _state = GprsState::CONNECTED;
    DEBUG_PRINTLN("[GPRS] Reconnected");
    return true;
}

TinyGsmClient& GprsManager::getClient() {
    return *_client;
}

} // namespace Network
