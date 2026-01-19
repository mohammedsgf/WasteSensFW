/**
 * @file mqtt_service.cpp
 * @brief MQTT Service implementation
 */

#include "mqtt_service.h"
#include "config.h"

namespace Network {

MqttService::MqttService(GprsManager& gprsManager)
    : _gprsManager(gprsManager), _mqtt(nullptr), _state(MqttState::DISCONNECTED),
      _port(1883), _lastReconnectAttempt(0) {
}

bool MqttService::init(const char* broker, uint16_t port, const char* clientId,
                       const char* user, const char* pass) {
    DEBUG_PRINTLN("[MQTT] Initializing...");
    
    _broker = broker;
    _port = port;
    _clientId = clientId;
    _user = user;
    _pass = pass;
    
    // Create MQTT client
    if (!_mqtt) {
        _mqtt = new PubSubClient(_gprsManager.getClient());
    }
    
    // Set buffer size (default 256 is too small for JSON payload)
    _mqtt->setBufferSize(512);
    
    // Set keep-alive to 60 seconds
    _mqtt->setKeepAlive(60);
    
    // Set socket timeout (in seconds)
    _mqtt->setSocketTimeout(30);
    
    _mqtt->setServer(broker, port);
    
    DEBUG_PRINTF("[MQTT] Broker: %s:%d\n", broker, port);
    DEBUG_PRINTF("[MQTT] Client ID: %s\n", clientId);
    DEBUG_PRINTF("[MQTT] Buffer size: 512 bytes\n");
    
    return true;
}

bool MqttService::connect() {
    if (!_gprsManager.isConnected()) {
        DEBUG_PRINTLN("[MQTT] GPRS not connected");
        _state = MqttState::ERROR;
        return false;
    }
    
    _state = MqttState::CONNECTING;
    DEBUG_PRINTF("[MQTT] Connecting to %s...\n", _broker.c_str());
    
    bool connected = false;
    
    if (_user.length() > 0 && _pass.length() > 0) {
        connected = _mqtt->connect(_clientId.c_str(), _user.c_str(), _pass.c_str());
    } else {
        connected = _mqtt->connect(_clientId.c_str());
    }
    
    if (!connected) {
        DEBUG_PRINTF("[MQTT] Connection failed, state: %d\n", _mqtt->state());
        _state = MqttState::ERROR;
        return false;
    }
    
    _state = MqttState::CONNECTED;
    DEBUG_PRINTLN("[MQTT] Connected successfully");
    
    return true;
}

void MqttService::disconnect() {
    DEBUG_PRINTLN("[MQTT] Disconnecting...");
    
    if (_mqtt) {
        _mqtt->disconnect();
    }
    
    _state = MqttState::DISCONNECTED;
}

bool MqttService::isConnected() {
    if (!_mqtt) return false;
    
    bool connected = _mqtt->connected();
    
    if (!connected && _state == MqttState::CONNECTED) {
        _state = MqttState::DISCONNECTED;
    }
    
    return connected;
}

MqttState MqttService::getState() {
    return _state;
}

void MqttService::loop() {
    if (_mqtt && isConnected()) {
        _mqtt->loop();
    }
}

bool MqttService::publishSensorData(const SensorPayload& payload) {
    if (!ensureConnection()) {
        DEBUG_PRINTLN("[MQTT] Cannot publish - not connected");
        return false;
    }
    
    // Call loop() to process any pending messages and maintain connection
    _mqtt->loop();
    
    String topic = buildTopic(payload.deviceId);
    String jsonPayload = buildJsonPayload(payload);
    
    DEBUG_PRINTF("[MQTT] Publishing to: %s\n", topic.c_str());
    DEBUG_PRINTF("[MQTT] Payload: %s\n", jsonPayload.c_str());
    DEBUG_PRINTF("[MQTT] Payload length: %d bytes\n", jsonPayload.length());
    
    // Check connection state before publish
    if (!_mqtt->connected()) {
        DEBUG_PRINTF("[MQTT] Connection lost before publish! State: %d\n", _mqtt->state());
        _state = MqttState::DISCONNECTED;
        return false;
    }
    
    bool success = _mqtt->publish(topic.c_str(), jsonPayload.c_str());
    
    if (success) {
        DEBUG_PRINTLN("[MQTT] Publish successful");
    } else {
        // Get detailed error state
        int state = _mqtt->state();
        DEBUG_PRINTF("[MQTT] Publish failed! MQTT state: %d\n", state);
        DEBUG_PRINTLN("[MQTT] State codes: -4=timeout, -3=lost, -2=failed, -1=disconnected, 0=connected");
        
        // If disconnected, update our state
        if (state < 0) {
            _state = MqttState::DISCONNECTED;
        }
    }
    
    return success;
}

bool MqttService::publish(const char* topic, const char* payload, bool retained) {
    if (!ensureConnection()) {
        return false;
    }
    
    return _mqtt->publish(topic, payload, retained);
}

bool MqttService::ensureConnection() {
    if (isConnected()) {
        return true;
    }
    
    // Check if enough time passed since last attempt
    uint32_t now = millis();
    if (now - _lastReconnectAttempt < MQTT_RECONNECT_DELAY_MS) {
        return false;
    }
    _lastReconnectAttempt = now;
    
    DEBUG_PRINTLN("[MQTT] Connection lost, reconnecting...");
    
    // Ensure GPRS is connected first
    if (!_gprsManager.ensureConnection()) {
        DEBUG_PRINTLN("[MQTT] GPRS reconnection failed");
        return false;
    }
    
    // Reconnect MQTT
    return connect();
}

void MqttService::setCallback(MQTT_CALLBACK_SIGNATURE) {
    if (_mqtt) {
        _mqtt->setCallback(callback);
    }
}

bool MqttService::subscribe(const char* topic) {
    if (!isConnected()) {
        return false;
    }
    
    return _mqtt->subscribe(topic);
}

String MqttService::buildTopic(const char* deviceId) {
    // Format: smartwaste/{device_id}/data
    return String(MQTT_TOPIC_PREFIX) + "/" + deviceId + "/" + MQTT_TOPIC_SUFFIX;
}

String MqttService::buildJsonPayload(const SensorPayload& payload) {
    StaticJsonDocument<256> doc;
    
    doc["device_id"] = payload.deviceId;
    
    JsonObject location = doc.createNestedObject("location");
    location["latitude"] = payload.latitude;
    location["longitude"] = payload.longitude;
    
    doc["battery_level"] = payload.batteryLevel;
    doc["fill_level"] = payload.fillLevel;
    
    String output;
    serializeJson(doc, output);
    
    return output;
}

} // namespace Network
