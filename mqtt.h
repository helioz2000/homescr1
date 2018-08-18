/**
 * @file mqtt.h
 *
 -----------------------------------------------------------------------------
  The MQTT class encapsulates the mosquitto connection used for publishing
  and receiving data via the MQTT protocol from a broker.

 -----------------------------------------------------------------------------
 */

#ifndef MQTT_H
#define MQTT_H

//#include <time.h>

#include <mosquitto.h>

class MQTT {
public:
    // Constructor
    MQTT ();

    // Destructor
    ~MQTT();

    /**
     * Connect to the MQTT broker
     */
    void connect(void);

    /**
     * register callback for connection status change
     */
    void registerConnectionCallback(void (*callback) (bool));

    /**
     * register callback for topic update
     */
    void registerTopicUpdateCallback(void (*callback) (const char*, const char*));

    /**
     * callback function for async connect
     * @param mosq: pointer to mosquitto structure
     * @param result: connection result
     */
    void connect_callback(struct mosquitto *mosq, int result);

    /**
     * callback function for disconnect
     * @param mosq: pointer to mosquitto structure
     * @param rc: result code
     */
    void disconnect_callback(struct mosquitto *mosq, int rc);

    /**
     * callback function for publish
     * @param mosq: pointer to mosquitto structure
     * @param mid: message id
     */
    void publish_callback(struct mosquitto *mosq, int mid);

    /**
     * callback function for message receive
     * @param mosq: pointer to mosquitto structure
     * @param message: the received mosquite message
     */
    void message_callback(struct mosquitto *m, const struct mosquitto_message *message);

     /**
      * callback function for logging
      * @param mosq: pointer to mosquitto structure
      * @param level: log level
      * @param str: log message
      */
    void log_callback(struct mosquitto *m, int level, const char *str);

    /**
     * callback function for subscribe
     * @param mosq: pointer to mosquitto structure
     * @param mid: message ID
     * @param qos_count: the number of granted subscriptions (size of granted_qos)
     * @param granted_qos: array of integers indicating the granted QoS for each of the subscriptions
     */
    void subscribe_callback(struct mosquitto *m, int mid, int qos_count, const int *granted_qos);

    /**
     * publish topic
     * @param topic: the topic name to be published
     * @param format: printf style format string
     * @param value: the numeric value to publish
     * @return: message ID, can be used for further tracking
     */
    int publish(const char* topic, const char* format, float value);

    /**
     * subscribe to a topic
     * @param topic: topic string
     */
    int subscribe(const char *topic);

    /**
     * unsubscribe from a topic
     * @param topic: topic string
     */
    int unsubscribe(const char *topic);

    /**
     * check connection status
     * @param buffer: text storage buffer
     * @returns: true if connection is established
     */
    bool isConnected(void);

    /**
     * Read Kernel name and version
     * @param buffer: text storage buffer
     * @param maxlen: length of text storage buffer
     */
    //int get_kernel_name(char *buffer, int maxlen);

    /**
     * Read active IP address
     * @param buffer: text storage buffer
     * @param maxlen: length of text storage buffer
     */
    //int get_ip_address(char *buffer, int maxlen);

private:
    void (*connectionStatusCallback) (bool);     // callback for connection status change
    void (*topicUpdateCallback) (const char *topic, const char *value);     // callback for topic update

    struct mosquitto *mosq;
    bool connected;
    char pub_buf[100];

    bool console_log_enable;    // for mosqitto logging

    int qos;        // quality of service [0..2]
    bool retain;    // reained message (last known value)
};

#endif /* HARDWARE_H */