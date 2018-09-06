/**
 * @file mqtt.cpp
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <sys/utsname.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <stdexcept>
#include <iostream>

#include "mqtt.h"

/*********************
 *      DEFINES
 *********************/

#define CLIENT_ID "homescr1"        // our ID for broker connection
#define MQTT_BROKER_ADDRESS "192.168.0.124"
#define MQTT_BROKER_PORT 1883
#define MQTT_BROKER_KEEPALIVE 60

using namespace std;

/*********************
 *  GLOBAL FUNCTIONS
 *********************
 *
 * These functions are used for mosquitto callback implementation
 * the parameter obj contains a pointer to a MQTT class instance
 */

// Callback function for mosquitto connect async
static void on_connect(struct mosquitto *mosq, void *obj, int result) {
    // callback function of the relevant instance
    ((MQTT*)obj)->connect_callback(mosq, result);
}

// Callback function for mosquitto disconnect async
static void on_disconnect(struct mosquitto *mosq, void *obj, int rc) {
    // callback function of the relevant instance
    ((MQTT*)obj)->disconnect_callback(mosq, rc);
}

// Callback function for mosquitto publish
static void on_publish(struct mosquitto *mosq, void *obj, int mid) {
    ((MQTT*)obj)->publish_callback(mosq, mid);
}

// Callback function for mosquitto message receive
static void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message) {
    ((MQTT*)obj)->message_callback(mosq, message);
}

// Callback function for mosquitto logging
static void on_log(struct mosquitto *mosq, void *obj, int level, const char *str) {
	((MQTT*)obj)->log_callback(mosq, level, str);
}

// Callback function for mosquitto subscribe
static void on_subscribe(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos) {
    ((MQTT*)obj)->subscribe_callback(mosq, mid, qos_count, granted_qos);
}

/*********************
 * GLOBAL FUNCTIONS
 *********************/

 /*********************
  * MEMBER FUNCTIONS
  *********************/

 //
 // Class MQTT
 //

 MQTT::MQTT() {
     connected = false;
     console_log_enable = false;
     qos = 0;
     retain = true;
     connectionStatusCallback = NULL;
     topicUpdateCallback = NULL;

     // initialise library
     mosquitto_lib_init();
     int major, minor, revision, result;
     result = mosquitto_lib_version(&major, &minor, &revision);
     printf("%s - mosqitto V%d.%d.%d (%d)\n", __func__, major, minor, revision, result);

     // create new mqtt
     mosq = mosquitto_new(CLIENT_ID, false, this);  // "this" provides a link from calllback to class instance
     if (mosq == NULL) {
         throw runtime_error("Class MQTT - mosquitto_new returned NULL");
     }

     // start mqtt processing loop in own thread
     result = mosquitto_loop_start(mosq);
     if (result != MOSQ_ERR_SUCCESS) {
         throw runtime_error("Class MQTT - mosquitto_loop_start failed");
     }

     // set callback functions
     mosquitto_connect_callback_set(mosq, on_connect);
     mosquitto_disconnect_callback_set(mosq, on_disconnect);
     mosquitto_publish_callback_set(mosq, on_publish);
     mosquitto_message_callback_set(mosq, on_message);
     mosquitto_log_callback_set(mosq, on_log);
     mosquitto_subscribe_callback_set(mosq, on_subscribe);
 }

 MQTT::~MQTT() {
     //printf("%s - Connected: %d\n", __func__, connected);
     if (connected) mosquitto_disconnect(mosq) ;
     mosquitto_loop_stop(mosq, true); // Note: must be true or this will block
     if (mosq != NULL) {
         mosquitto_destroy(mosq);
         mosq = NULL;
     }
     mosquitto_lib_cleanup();
 }

void MQTT::setConsoleLog(bool enable) {
    console_log_enable = enable;
}

void MQTT::connect(void) {
    char strbuf[255];
    // connect to mqtt server
    int result = mosquitto_connect_async(mosq, MQTT_BROKER_ADDRESS, MQTT_BROKER_PORT, MQTT_BROKER_KEEPALIVE);
    if (result != MOSQ_ERR_SUCCESS) {
        sprintf(strbuf, "%s - mosquitto_connect failed: %s [%d]\n", __func__, strerror(result), result);
        throw runtime_error(strbuf);
    }
    //printf ("%s\n", __func__);
}

void MQTT::disconnect(void) {
    if (connected) mosquitto_disconnect(mosq) ;
}

void MQTT::registerConnectionCallback(void (*callback) (bool)) {
    connectionStatusCallback = callback;
}

void MQTT::registerTopicUpdateCallback(void (*callback) (const char*, const char*)) {
    topicUpdateCallback = callback;
}

int MQTT::publish(const char* topic, const char* format, float value) {
    int messageid = 0;
    if (!connected) {
        fprintf(stderr, "%s: Not Connected!\n", __func__);
        return -1;
    } else {
        //printf ("%s: %s\n", __func__, topic);
    }
    sprintf(pub_buf, format, value);
    //printf ("%s: %s %s\n", __func__, topic, pub_buf);
    int result = mosquitto_publish(mosq, &messageid, topic, strlen(pub_buf), (const char *) pub_buf,qos,retain);
    if (result != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "%s: %s [%s]\n", __func__, mosquitto_strerror(result), topic);
    }
    return messageid;
}

int MQTT::subscribe(const char *topic) {
    int messageid = 0;
    int result = mosquitto_subscribe(mosq, &messageid, topic, qos);
    if (result != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "%s: %s [%s]\n", __func__, mosquitto_strerror(result), topic);
    } else {
        //printf ("%s: %s\n", __func__, topic);
    }
    return messageid;
}

int MQTT::unsubscribe(const char *topic) {
    int messageid = 0;
    int result = mosquitto_unsubscribe(mosq, &messageid, topic);
    if (result != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "%s: %s [%s]\n", __func__, mosquitto_strerror(result), topic);
    }
    return messageid;
}

bool MQTT::isConnected(void) {
    return connected;
}

void MQTT::message_callback(struct mosquitto *m, const struct mosquitto_message *message) {
    //fprintf(stderr, "%s:\n", __func__);
    /*
    if(message->payloadlen){
		fprintf(stderr, "%s %s\n", message->topic, (const char *)message->payload);
	}else{
		fprintf(stderr, "%s (null)\n", message->topic);
	}
    */
    if (topicUpdateCallback != NULL) {
        (*topicUpdateCallback) (message->topic, (const char *)message->payload);
    }
}

void MQTT::log_callback(struct mosquitto *m, int level, const char *str) {
    /* Print all log messages regardless of level. */
    if (console_log_enable) {
        fprintf(stderr, "%s [%d]: %s\n",__func__ , level, str);
    }
}

void MQTT::subscribe_callback(struct mosquitto *m, int mid, int qos_count, const int *granted_qos) {
    //printf("%s: mid:%d qos_count:%d\n", __func__, mid, qos_count);
}

void MQTT::publish_callback(struct mosquitto *m, int mid) {
    //fprintf(stderr, "%s: %d\n", __func__, mid );
}

void MQTT::connect_callback(struct mosquitto *m, int result) {
     //printf("%s: %s\n", __func__ , mosquitto_connack_string(result) );
     if (result == MOSQ_ERR_SUCCESS) {
         connected = true;
     } else {
         fprintf(stderr, "%s: %s\n", __func__ , mosquitto_connack_string(result) );
     }
     if (connectionStatusCallback != NULL) {
         (*connectionStatusCallback) (connected);
     }
}

void MQTT::disconnect_callback(struct mosquitto *m, int rc) {
     //fprintf(stderr, "%s: %s\n", __func__, mosquitto_strerror(rc) );
     connected = false;
     if (connectionStatusCallback != NULL) {
         (*connectionStatusCallback) (connected);
     }
 }

 /*********************
  * PRIVATE FUNCTIONS
  *********************/
