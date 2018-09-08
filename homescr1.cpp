/**
 * @file homescr1.cpp
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <sys/utsname.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <mosquitto.h>

#include "mqtt.h"

#include "lvgl/lvgl.h"

#include "hardware.h"
#include "screen.h"
#include "datatag.h"
#include "mcp9808/mcp9808.h"

#define VAR_PROCESS_INTERVAL 5      // seconds

//#define MQTT_CONNECT_TIMEOUT 5      // seconds

#define CPU_TEMP_TOPIC "binder/home/screen1/cpu/temp"
#define ENV_TEMP_TOPIC "binder/home/screen1/env/temp"

bool exitSignal = false;
time_t var_process_time = time(NULL) + VAR_PROCESS_INTERVAL;
//time_t mqtt_connection_timeout = 0;
time_t mqtt_connect_time = 0;   // time the connection was initiated
bool mqtt_connection_in_progress = false;

extern char *info_label_text;
extern void cpuTempUpdate(int x, Tag* t);
extern void roomTempUpdate(int x, Tag* t);

// Proto types
void subscribe_tags(void);
void mqtt_connection_status(bool status);
void mqtt_topic_update(const char *topic, const char *value);

Hardware hw;
TagStore ts;
MQTT mqtt;
Mcp9808 envTempSensor;    // Environment temperature sensor at rear of screen

/*
 * Handle system signals
 */
void sigHandler(int signum)
{
    char signame[10];
    switch (signum) {
        case SIGTERM:
            strcpy(signame, "SIGTERM");
            break;
        case SIGHUP:
            strcpy(signame, "SIGHUP");
            break;
        case SIGINT:
            strcpy(signame, "SIGINT");
            break;

        default:
            break;
    }

    printf("Received %s\n", signame);
    exitSignal = true;
}

/*
 * Process commands from screen user interface
 */
void cmd_process(void)
{
    switch (screen_getCmd()) {
        case SCR_CMD_SHUTDOWN:
            printf("Shutdown\n");
            hw.shutdown(false);
            exitSignal = true;
            break;
        case SCR_CMD_REBOOT:
            printf("Reboot\n");
            hw.shutdown(true);
            exitSignal = true;
            break;
        case SCR_CMD_BRIGHTNESS:
            hw.set_brightness(screen_brightness());
            break;
        default:
            break;
    }
    screen_clearCmd();
}

/*
 * Process local variables
 * Local variables get are process at a fixed time interval
 * The processing involves reading value from hardware and
 * publishing the value to MQTT broker
 */
void var_process(void) {
    float fValue;
    time_t now = time(NULL);
    if (now > var_process_time) {
        var_process_time = now + VAR_PROCESS_INTERVAL;

        // update CPU temperature
        Tag *tag = ts.getTag((char*) CPU_TEMP_TOPIC);
        if (tag != NULL) {
            tag->setValue(hw.read_cpu_temp());
            if (mqtt.isConnected()) {
              mqtt.publish(CPU_TEMP_TOPIC, "%.1f", tag->floatValue() );
            }
        }

        // update environment temperature
        tag = ts.getTag((const char*) ENV_TEMP_TOPIC);
        if (tag != NULL) {
            if (envTempSensor.readTempC(&fValue)) {
                tag->setValue(fValue);
                if (mqtt.isConnected()) {
                    mqtt.publish(ENV_TEMP_TOPIC, "%.1f", tag->floatValue() );
                }
            }
        }
    }

    // reconnect mqtt if required
    /*if (!mqtt.isConnected() && !mqtt_connection_in_progress) {
        mqtt_connect();
    }*/
}

void init_values(void)
{
    char info1[80], info2[80], info3[80], info4[80];
    // Initialise brightness
    int value = hw.get_brightness();
    if (value < 10) {
        value = 10; // ensure min value
        hw.set_brightness(value);
    }
    screen_set_brightness(value);   // write to screen brightness

    // get hardware info
    hw.get_model_name(info1, sizeof(info1));
    hw.get_os_name(info2, sizeof(info2));
    hw.get_kernel_name(info3, sizeof(info3));
    hw.get_ip_address(info4, sizeof(info4));
    info_label_text = (char *)malloc(strlen(info1) +strlen(info2) +strlen(info3) +strlen(info4) +5);
    sprintf(info_label_text, "%s\n%s\n%s\n%s", info1, info2, info3, info4);
    //printf(info_label_text);
}

/*
 * Initialise the tag database (tagstore)
 *
 */
void init_tags(void)
{
    // CPU temp
    Tag* tp = ts.addTag((char*) CPU_TEMP_TOPIC);
    tp->setPublish();
    tp->registerCallback(&cpuTempUpdate, 15);   // update screen

    // Environment temperature is stored in index 0
    tp = ts.addTag((char*) ENV_TEMP_TOPIC);
    tp->setPublish();
    tp->registerCallback(&roomTempUpdate, 0);   // update screen

    // Shack Temp is stored in index 0
    tp = ts.addTag((char*) "binder/home/shack/room/temp");
    tp->registerCallback(&roomTempUpdate, 1);

    // Bedroom 1 Temp
    tp = ts.addTag((const char*) "binder/home/bed1/room/temp");
    tp->registerCallback(&roomTempUpdate, 2);

    // Testing only
    //ts.addTag((char*) "binder/home/screen1/room/temp");
    //ts.addTag((char*) "binder/home/screen1/room/hum");
    // = ts.getTag((char*) "binder/home/screen1/room/temp");
}

void mqtt_connect(void) {
    //printf("%s - attempting to connect to mqtt broker.\n", __func__);
    mqtt.connect();
    //mqtt_connection_timeout = time(NULL) + MQTT_CONNECT_TIMEOUT;
    mqtt_connection_in_progress = true;
    mqtt_connect_time = time(NULL);
}

/*
 * Initialise the MQTT broker and register callbacks
 */
void init_mqtt(void) {
    mqtt.registerConnectionCallback(mqtt_connection_status);
    mqtt.registerTopicUpdateCallback(mqtt_topic_update);
    mqtt_connect();
}

/*
 * Subscribe tags to MQTT broker
 * Iterate over tag store and process every "subscribe" tag
 */
void subscribe_tags(void) {
    //printf("%s\n", __func__);
    Tag* tp = ts.getFirstTag();
    while (tp != NULL) {
        if (tp->isSubscribe()) {
            //printf("%s: %s\n", __func__, tp->getTopic());
            mqtt.subscribe(tp->getTopic());
        }
        tp = ts.getNextTag();
    }
}

/*
 * callback function for MQTT
 * MQTT notifies a change in connection status by calling this function
 * This function is registered with MQTT during initialisation
 */
void mqtt_connection_status(bool status) {
    //printf("%s - %d\n", __func__, status);
    // subscribe tags when connection is online
    if (status) {
        printf("%s: connected to mqtt broker\n", __func__);
        mqtt_connection_in_progress = false;
        subscribe_tags();
    } else {
        if (mqtt_connection_in_progress) {
            mqtt.disconnect();
            // Note: the timeout is determined by OS network stack
            fprintf(stderr, "%s: mqtt connection timeout after %lds\n", __func__, time(NULL) - mqtt_connect_time);
            mqtt_connection_in_progress = false;
        }
    }
    //printf("%s - done\n", __func__);
}

/*
 * callback function for MQTT
 * MQTT notifies when a subscribed topic has received an update
 *
 * Note: do not store the pointers "topic" & "value", they will be
 * destroyed after this function returns
 */
void mqtt_topic_update(const char *topic, const char *value) {
    //printf("%s - %s %s\n", __func__, topic, value);
    Tag *tp = ts.getTag(topic);
    if (tp == NULL) {
        fprintf(stderr, "%s: <%s> not  in ts\n", __func__, topic);
        return;
    }
    tp->setValue(value);
}

/*
 * called on program exit
 */
void exit_loop(void)
{
    hw.set_brightness(screen_brightness());
    screen_exit();
    for (int i=0; i<=10; i++) {
        lv_tick_inc(SCREEN_UPDATE);
        lv_task_handler();
        usleep(SCREEN_UPDATE * 1000);
    }
}

void main_loop()
{
    clock_t start, end;
    double cpu_time_used;
    double min_time = 99999.0, max_time = 0.0;

    // first call takes a long time (10ms)
    lv_tick_inc(SCREEN_UPDATE);
    lv_task_handler();
    while (!exitSignal) {
        start = clock();
        lv_tick_inc(SCREEN_UPDATE);
        lv_task_handler();
        cmd_process();
        var_process();
        hw.process_screen_saver(screen_brightness());
        end = clock();
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
        if (cpu_time_used > max_time) {
            max_time = cpu_time_used;
        }
        if (cpu_time_used < min_time) {
            min_time = cpu_time_used;
        }
        usleep(SCREEN_UPDATE * 1000);
    }
    printf("CPU time %.3fms - %.3fms\n", min_time*1000, max_time*1000);
}


int main (int argc, char *argv[])
{
    signal (SIGINT, sigHandler);
    //signal (SIGHUP, sigHandler);
    //signal (SIGTERM, sigHandler);

    //mqtt.setConsoleLog(true);
    init_tags();
    init_mqtt();
    init_values();
    usleep(100000);
    screen_init();
    screen_create();
    main_loop();
    exit_loop();
}
