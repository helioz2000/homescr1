/**
 * @file homescr1.cpp
 *
 * Author: Erwin Bejsta
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
#include <syslog.h>

#include <mosquitto.h>

#include "mqtt.h"
#include "topics.h"

#include "lvgl.h"

#include "hardware.h"
#include "screen.h"
#include "datatag.h"
#include "mcp9808.h"

#define VAR_PROCESS_INTERVAL 5      // seconds

//#define MQTT_CONNECT_TIMEOUT 5      // seconds

bool exitSignal = false;
bool debugEnabled = false;
bool runningAsDaemon = false;
time_t var_process_time = time(NULL) + VAR_PROCESS_INTERVAL;
//time_t mqtt_connection_timeout = 0;
time_t mqtt_connect_time = 0;   // time the connection was initiated
bool mqtt_connection_in_progress = false;
std::string processName;


extern char *info_label_text;

/* Callback functions to update the display value  */
extern void cpuTempUpdate(int x, Tag* t);
extern void roomTempUpdate(int x, Tag* t);
extern void shackHeaterSwitchUpdate(int x, Tag* t);
extern void shackHeaterSliderUpdate(int x, Tag* t);
extern void shackHeaterLedUpdate(int x, Tag* t);


// Proto types
void subscribe_tags(void);
void mqtt_connection_status(bool status);
void mqtt_topic_update(const char *topic, const char *value);
void mqtt_publish_tag(int x, Tag* t);

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
    syslog(LOG_INFO, "Received %s", signame);
    exitSignal = true;
}

/*
 * Process commands from screen user interface
 */
void cmd_process(void)
{
    std::string cmdStr;
    switch (screen_getCmd()) {
        case SCR_CMD_SHUTDOWN:
            cmdStr = "Shutdown";
            hw.shutdown(false);
            exitSignal = true;
            break;
        case SCR_CMD_REBOOT:
            cmdStr = "Reboot";
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
    if (exitSignal) {
        printf("%s - %s\n", __func__, cmdStr.c_str());
        syslog(LOG_INFO, "User selected %s", cmdStr.c_str());
    }
}

/*
 * Process local variables
 * Local variables get are processed at a fixed time interval
 * The processing involves reading value from hardware and
 * publishing the value to MQTT broker
 */
void var_process(void) {
    time_t now = time(NULL);
    if (now > var_process_time) {
        var_process_time = now + VAR_PROCESS_INTERVAL;

        // update CPU temperature
        Tag *tag = ts.getTag((char*) TOPIC_CPU_TEMP);
        if (tag != NULL) {
            tag->setValue(hw.read_cpu_temp(), true);
            cpuTempUpdate(0, tag);      // update on screen
        }

/* disabled - to check if it causes interference with touch screen pointer device
        // update environment temperature
        float fValue;
        tag = ts.getTag((const char*) TOPIC_ENV_TEMP);
        if (tag != NULL) {
            if (envTempSensor.readTempC(&fValue)) {
                tag->setValue(fValue);
                if (mqtt.isConnected()) {
                    mqtt.publish(TOPIC_ENV_TEMP, "%.1f", tag->floatValue() );
                }
            } else {
                syslog(LOG_ERR, "Failed to read Mcp9808 temp sensor");
            }
        }
        */
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
    Tag* tp = ts.addTag((char*) TOPIC_CPU_TEMP);
    tp->setPublish();
    tp->setFormat("%.1f");
    tp->registerUpdateCallback(&cpuTempUpdate, 15);   // update screen
    tp->registerPublishCallback(&mqtt_publish_tag, 0);

    // Environment temperature is stored in index 0
    tp = ts.addTag((char*) TOPIC_ENV_TEMP);
    tp->setPublish();
    tp->registerUpdateCallback(&roomTempUpdate, 0);   // update screen

    // Shack Temp is stored in index 0
    tp = ts.addTag((const char*) TOPIC_SHACK_ROOM_TEMP);
    tp->setSubscribe();
    tp->setFormat("%.1f");
    tp->registerUpdateCallback(&roomTempUpdate, 1);

    // Bedroom 1 Temp
    tp = ts.addTag((const char*) TOPIC_BED1_ROOM_TEMP);
    tp->setSubscribe();
    tp->registerUpdateCallback(&roomTempUpdate, 2);

    // Shack heater on/off
    tp = ts.addTag((const char*) TOPIC_SHACK_HEATER_ENABLE);
    tp->setSubscribe();
    tp->setPublish();
    tp->setRetain(true);    // needs to be retained during broker reboot
    tp->setType(TAG_TYPE_BOOL);
    tp->registerUpdateCallback(&shackHeaterSwitchUpdate, 0);
    tp->registerPublishCallback(&mqtt_publish_tag, 0);

    // Shack heater temp setpoint
    tp = ts.addTag((const char*) TOPIC_SHACK_HEATER_TEMP_SP);
    tp->setSubscribe();
    tp->setPublish();
    tp->setRetain(true);    // needs to be retained during broker reboot
    tp->setFormat("%.1f");
    tp->registerUpdateCallback(&shackHeaterSliderUpdate, 0);
    tp->registerPublishCallback(&mqtt_publish_tag, 0);

    // Shack heater on/off control
    tp = ts.addTag((const char*) TOPIC_SHACK_HEATER_CONTROL);
    tp->setSubscribe();
    tp->setType(TAG_TYPE_BOOL);
    tp->registerUpdateCallback(&shackHeaterLedUpdate, 0);


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
        syslog(LOG_INFO, "Connected to MQTT broker [%s]", mqtt.server());
        printf("%s: Connected to mqtt broker [%s]\n", __func__, mqtt.server());
        mqtt_connection_in_progress = false;
        subscribe_tags();
    } else {
        if (mqtt_connection_in_progress) {
            mqtt.disconnect();
            // Note: the timeout is determined by OS network stack
            unsigned long timeout = time(NULL) - mqtt_connect_time;
            syslog(LOG_INFO, "mqtt connection timeout after %lds", timeout);
            fprintf(stderr, "%s: mqtt connection timeout after %lds\n", __func__, timeout);
            mqtt_connection_in_progress = false;
        } else {
            syslog(LOG_WARNING, "Disconnected from MQTT broker [%s]", mqtt.server());
            fprintf(stderr, "%s: Disconnected from MQTT broker [%s]\n", __func__, mqtt.server());
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
 * value can be NULL (clear stored value)
 */
void mqtt_topic_update(const char *topic, const char *value) {
    //printf("%s - %s %s\n", __func__, topic, value);
    Tag *tp = ts.getTag(topic);
    if (tp == NULL) {
        fprintf(stderr, "%s: <%s> not  in ts\n", __func__, topic);
        return;
    }
	if (value == NULL) {
		//fprintf(stderr, "%s: <%s> received NULL value\n", __func__, topic);
		// should set to "noread" value
	} else {
    	tp->setValue(value);
	}
}

/*
 * callback function for data tags
 * data tags notify when their value has been updated from an internal source
 * and the updated value requires publishing to MQTT
 */
void mqtt_publish_tag(int x, Tag *t) {
    if (mqtt.isConnected()) {
        //printf("%s[%d] - publishing %s\n", __FILE__,__LINE__, t->getTopic());
        if (t->type() == TAG_TYPE_BOOL) {
            //printf("%s - bool detected <%s>\n", __func__, t->getTopic());
            mqtt.publish(t->getTopic(), t->boolValue() ? MQTT_TRUE : MQTT_FALSE, 0, t->getRetain() );
        } else {
            mqtt.publish(t->getTopic(), t->getFormat(), t->floatValue(), t->getRetain() );
        }
    }
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

void argument(const char *arg) {
    if (arg[0] == '-') {
        switch (arg[1]) {
            case 'd':
                debugEnabled = true;
                printf("Debug enabled\n");
                break;
            default:
                fprintf(stderr, "unknown argument: %s\n", arg);
                syslog(LOG_NOTICE, "unknown argument: %s", arg);
        }
    }
}

int main (int argc, char *argv[])
{
    int i;

    if ( getppid() == 1) {
        runningAsDaemon = true;
    }

    processName =  argv[0];
    for (i = 1; i < argc; i++) {
        argument( argv[i] );
    }

    syslog(LOG_INFO,"[%s] PID: %d PPID: %d", argv[0], getpid(), getppid());
    
    signal (SIGINT, sigHandler);
    //signal (SIGHUP, sigHandler);

    // catch SIGTERM only if running as daemon (started via systemctl)
    // when run from command line SIGTERM provides a last resort method
    // of killing the process regardless of any programming errors.
    if (runningAsDaemon) {
        signal (SIGTERM, sigHandler);
    }

    //mqtt.setConsoleLog(true);
    usleep(100000);
    // sequence is very important, functions rely on initialised data
    screen_init();
    init_tags();
    init_values();
    screen_create();
    init_mqtt();
    main_loop();
    exit_loop();
    syslog(LOG_INFO, "exiting");
}
