/**
 * @file mqtt_service.h
 * @brief MQTT Service for publishing sensor data
 */

#ifndef MQTT_SERVICE_H
#define MQTT_SERVICE_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "gprs_manager.h"

namespace Network {

/**
 * @brief MQTT connection state
 */
enum class MqttState {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    ERROR
};

/**
 * @brief Smart waste sensor payload
 */
struct SensorPayload {
    const char* deviceId;
    float latitude;
    float longitude;
    int8_t batteryLevel;
    int8_t fillLevel;
};

/**
 * @brief MQTT Service for publishing data
 */
class MqttService {
public:
    /**
     * @brief Constructor
     * @param gprsManager Reference to GPRS manager
     */
    MqttService(GprsManager& gprsManager);

    /**
     * @brief Initialize MQTT service
     * @param broker MQTT broker address
     * @param port MQTT broker port
     * @param clientId Client ID
     * @param user Username (optional)
     * @param pass Password (optional)
     * @return true if initialization successful
     */
    bool init(const char* broker, uint16_t port, const char* clientId,
              const char* user = "", const char* pass = "");

    /**
     * @brief Connect to MQTT broker
     * @return true if connected
     */
    bool connect();

    /**
     * @brief Disconnect from MQTT broker
     */
    void disconnect();

    /**
     * @brief Check if connected to broker
     * @return true if connected
     */
    bool isConnected();

    /**
     * @brief Get connection state
     * @return Current MQTT state
     */
    MqttState getState();

    /**
     * @brief Process MQTT messages (call in loop)
     */
    void loop();

    /**
     * @brief Publish sensor data
     * @param payload Sensor payload data
     * @return true if published successfully
     */
    bool publishSensorData(const SensorPayload& payload);

    /**
     * @brief Publish raw message
     * @param topic MQTT topic
     * @param payload Message payload
     * @param retained Retain flag
     * @return true if published successfully
     */
    bool publish(const char* topic, const char* payload, bool retained = false);

    /**
     * @brief Ensure connection, reconnect if needed
     * @return true if connected
     */
    bool ensureConnection();

    /**
     * @brief Set callback for incoming messages
     * @param callback Callback function
     */
    void setCallback(MQTT_CALLBACK_SIGNATURE);

    /**
     * @brief Subscribe to topic
     * @param topic Topic to subscribe
     * @return true if subscription successful
     */
    bool subscribe(const char* topic);

private:
    GprsManager& _gprsManager;
    PubSubClient* _mqtt;
    MqttState _state;
    
    String _broker;
    uint16_t _port;
    String _clientId;
    String _user;
    String _pass;
    
    uint32_t _lastReconnectAttempt;

    /**
     * @brief Build topic string for sensor data
     * @param deviceId Device ID
     * @return Full topic string
     */
    String buildTopic(const char* deviceId);

    /**
     * @brief Build JSON payload from sensor data
     * @param payload Sensor payload
     * @return JSON string
     */
    String buildJsonPayload(const SensorPayload& payload);
};

} // namespace Network

#endif // MQTT_SERVICE_H
