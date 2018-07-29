/**
 * @file hardware.c
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

#include "lvgl/lvgl.h"

#include "hardware.h"
#include "screen.h"
#include "datatag.h"

#define VAR_PROCESS_INTERVAL 5      // seconds

#define CPU_TEMP_TOPIC "binder/home/screen1/cpu/temp"

bool exitSignal = false;
time_t var_process_time = time(NULL) + VAR_PROCESS_INTERVAL;
extern char *info_label_text;

Hardware hw;
TagStore ts;

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
            hw.set_brightness(screen_current_brightness());
            break;
        default:
            break;
    }
    screen_clearCmd();
}

void var_process(void) {
    time_t now = time(NULL);
    if (now > var_process_time) {
        var_process_time = now + VAR_PROCESS_INTERVAL;
        // update CPU temperature
        Tag *tag = ts.getTag((char*) CPU_TEMP_TOPIC);
        if (tag != NULL) {
            tag->setValue(hw.read_cpu_temp());
        }
    }
}

void init_values(void)
{
    char info1[80], info2[80], info3[80], info4[80];
    char buffer[240];
    // Initialise brightness
    int value = hw.get_brightness();
    if (value < 10) {
        value = 10; // ensure min value
        hw.set_brightness(value);
    }
    screen_set_current_brightness(value);   // write to screen brightness
    
    // get hardware info
    hw.get_model_name(info1, sizeof(info1));
    hw.get_os_name(info2, sizeof(info2));
    hw.get_kernel_name(info3, sizeof(info3));
    hw.get_ip_address(info4, sizeof(info4));
    info_label_text = (char *)malloc(strlen(info1) +strlen(info2) +strlen(info3) +strlen(info4) +5);
    sprintf(info_label_text, "%s\n%s\n%s\n%s", info1, info2, info3, info4);
    //printf(info_label_text);
}

void cb_test(int x, Tag* t)
{
    printf("%s - [%s] %f\n", __func__, t->getTopic(), t->floatValue());
}


void init_tags(void)
{
    Tag* tp = ts.addTag((char*) CPU_TEMP_TOPIC);
    ts.addTag((char*) "binder/home/screen1/room/temp");
    ts.addTag((char*) "binder/home/screen1/room/hum");
    // = ts.getTag((char*) "binder/home/screen1/room/temp");
    
    tp->registerCallback(&cb_test, 15);

}

void exit_loop(void)
{
    hw.set_brightness(screen_current_brightness());
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
        hw.process_screen_saver(screen_current_brightness());
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
    signal (SIGHUP, sigHandler);
    signal (SIGTERM, sigHandler);
    
    init_values();
    init_tags();
    screen_init();
    screen_create();
    main_loop();
    exit_loop();
}
