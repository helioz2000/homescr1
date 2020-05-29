/**
 * @file screen.cpp
 *
 * Author: Erwin Bejsta
 * 
 * screen objects reference: https://docs.lvgl.io/v7/
 *
 * History:
 * 2018 Sept - First version using LVGL V6
 * 2020 May - Upgraded to use LVGL V7
 */

/*********************
 *      INCLUDES
 *********************/

#include <stddef.h>
#include <stdio.h>

#include "lvgl.h"
#include "fbdev.h"
#include "evdev.h"

#include "screen.h"
#include "datatag.h"
#include "topics.h"

extern TagStore ts;

/**********************
 *   PRIVATE VARIABLES
 **********************/

// display buffer size - not sure if this size is really needed
#define LV_BUF_SIZE 384000      // 800x480
// A static variable to store the display buffers
static lv_disp_buf_t disp_buf;
// Static buffer(s). The second buffer is optional
static lv_color_t lvbuf1[LV_BUF_SIZE];
static lv_color_t lvbuf2[LV_BUF_SIZE];
// touch screen driver
lv_indev_drv_t indev_drv;

// graphic content 
char *info_label_text;
int16_t brightness_value = 10;
scr_cmd_t scr_cmd = SCR_CMD_NONE;

lv_obj_t *tv;
lv_obj_t *tab1;
lv_obj_t *tab2;
lv_obj_t *tab3;
lv_obj_t *tab4;
lv_obj_t *chart;
lv_obj_t *lv_cpuTemp;
static lv_obj_t *shack_heater_led;
static lv_obj_t *shack_heater_switch;

// Common styles
static lv_style_t style_led_green;
static lv_style_t style_led_red;

#define ROOM_TEMPS_MAX 5
lv_obj_t *lv_roomTemp[ROOM_TEMPS_MAX];

const char* roomTempFormat[ROOM_TEMPS_MAX] = { "This %.1f°C", "Shack %.1f°C", "Bed1 %.1f°C", NULL, NULL };

/**********************
 *   PRIVATE PROTOTYPES
 **********************/
void coolheat_create(lv_obj_t *parent);
void heating_create(lv_obj_t *parent);
void temps_create(lv_obj_t *parent);
void settings_create(lv_obj_t *parent);

void slider_action(lv_obj_t *obj, lv_event_t event);
void brightness_action(lv_obj_t *obj, lv_event_t event);
void pi_btn_action(lv_obj_t *obj, lv_event_t event);
void shack_heater_switch_action(lv_obj_t *obj, lv_event_t event);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
 
/**
 * Init screen subsystem
 */
void screen_init() {
    lv_init();		// LittlecGL init
    fbdev_init();	// Frame Buffer device init (screen)
    evdev_init();	// Touch pointer device init

    // Initialize `disp_buf` with the display buffer(s)
    lv_disp_buf_init(&disp_buf, lvbuf1, lvbuf2, LV_BUF_SIZE);

    //Initialize and register  display driver
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = fbdev_flush;	// lvgl buffer to frame buffer
    disp_drv.buffer = &disp_buf;        // set display buffer reference
    lv_disp_drv_register(&disp_drv);

    /* Initialize and register touch pointer driver */
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = evdev_read;
    lv_indev_drv_register(&indev_drv);
    
    /* Set common styles for screen objects*/
    /* Green LED */
    lv_style_init(&style_led_green);
    lv_style_set_bg_color(&style_led_green, LV_STATE_DEFAULT, LV_COLOR_GREEN);
    lv_style_set_shadow_color(&style_led_green, LV_STATE_DEFAULT, LV_COLOR_GREEN);
    lv_style_set_border_color(&style_led_green, LV_STATE_DEFAULT, LV_COLOR_GREEN);
    /* Red LED */
    lv_style_init(&style_led_red);
    lv_style_set_bg_color(&style_led_red, LV_STATE_DEFAULT, LV_COLOR_RED);
    lv_style_set_shadow_color(&style_led_red, LV_STATE_DEFAULT, LV_COLOR_RED);
    lv_style_set_border_color(&style_led_red, LV_STATE_DEFAULT, LV_COLOR_RED);
}

/**
 * Create screen application
 */
void screen_create(void) {
    /*
    lv_obj_t *wp = lv_img_create(lv_scr_act(), NULL);
    lv_img_set_src(wp, &img_bubble_pattern);
    lv_obj_set_width(wp, LV_HOR_RES * 4);
    lv_obj_set_protect(wp, LV_PROTECT_POS);
    */

    /* Button style */
    static lv_style_t style_tv_btn;
    //lv_style_copy(&style_tv_btn_bg, &lv_style_plain);
    lv_style_init(&style_tv_btn);
    
    /* TAB buttons on top of screen */
    lv_style_set_bg_color(&style_tv_btn,LV_STATE_DEFAULT,LV_COLOR_MAKE(0x48, 0x7F, 0xB7));
    lv_style_set_bg_color(&style_tv_btn, LV_BTN_STATE_PRESSED, LV_COLOR_BLUE);
    lv_style_set_text_color(&style_tv_btn, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_text_color(&style_tv_btn, LV_BTN_STATE_PRESSED, LV_COLOR_YELLOW);
    /* opacity is required to see different color when pressed */
    lv_style_set_bg_opa(&style_tv_btn, LV_BTN_STATE_PRESSED, LV_OPA_50);
    /* define the border around the tab buttons */
    lv_style_set_border_width(&style_tv_btn, LV_STATE_DEFAULT, 1);
    lv_style_set_border_side(&style_tv_btn, LV_STATE_DEFAULT, LV_BORDER_SIDE_LEFT + LV_BORDER_SIDE_RIGHT + LV_BORDER_SIDE_TOP);
    lv_style_set_radius(&style_tv_btn, LV_STATE_DEFAULT, 10);
    

    // coloured bar on each tab button
    static lv_style_t style_tv_indic;
    lv_style_init(&style_tv_indic);
    lv_style_set_size(&style_tv_indic, LV_STATE_DEFAULT, 5);
    lv_style_set_bg_color(&style_tv_indic, LV_STATE_DEFAULT, LV_COLOR_AQUA);


    static lv_style_t style_tv_bg;
    lv_style_init(&style_tv_bg);
    /* disable padding to put buttons on edge of screen */
    lv_style_set_pad_bottom(&style_tv_bg, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_top(&style_tv_bg, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_left(&style_tv_bg, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_right(&style_tv_bg, LV_STATE_DEFAULT, 0);

    /* create the table view */
    tv = lv_tabview_create(lv_scr_act(), NULL);
    /* add tabs */
    tab1 = lv_tabview_add_tab(tv, "Cool-Heat");
    tab2 = lv_tabview_add_tab(tv, "- - - - ");
    tab3 = lv_tabview_add_tab(tv, "Temps");
    tab4 = lv_tabview_add_tab(tv, "Settings");

    /* set styles for table view parts */
    lv_obj_add_style(tv, LV_TABVIEW_PART_TAB_BG, &style_tv_bg);
    lv_obj_add_style(tv, LV_TABVIEW_PART_TAB_BG, &style_tv_btn);
    lv_obj_add_style(tv, LV_TABVIEW_PART_INDIC, &style_tv_indic);
    lv_obj_add_style(tv, LV_TABVIEW_PART_TAB_BTN, &style_tv_btn);

	/* create the individual screens in their respective tabs */
    coolheat_create(tab1);
    heating_create(tab2);
    temps_create(tab3);
    settings_create(tab4);

}

scr_cmd_t screen_getCmd(void) {
    return scr_cmd;
}

void screen_clearCmd(void) {
    scr_cmd = SCR_CMD_NONE;
}

int16_t screen_brightness(void) {
    return brightness_value;
}

void screen_set_brightness(int16_t value) {
    brightness_value = value;
}

// exit screen
void screen_exit(void) {
    lv_obj_del(tab1);
    lv_obj_del(tab2);
    lv_obj_del(tab3);
    lv_obj_del(tab4);
    lv_obj_del(tv);

    lv_obj_t * label = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(label, "Application Has Been Terminated");
    lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);
}

/**********************
 *   PRIVATE FUNCTIONS
 **********************/

void cpuTempUpdate(int x, Tag* t) {
    char buffer[20];
    //printf("%s - [%s] %f\n", __func__, t->getTopic(), t->floatValue());
    snprintf(buffer, sizeof(buffer), "CPU %.1f°C", t->floatValue());
    lv_label_set_text(lv_cpuTemp, buffer);
}

void roomTempUpdate(int x, Tag* t) {
    char buffer[20];
    snprintf(buffer, sizeof(buffer), roomTempFormat[x], t->floatValue());
    // Note: due to multi threading it is possible that this function
    // is called before the lv_roomTemp array is valid
    if (lv_roomTemp[t->valueUpdateID()] != NULL) {
        lv_label_set_text(lv_roomTemp[t->valueUpdateID()], buffer);
        //printf("%s - [%s] %f\n", __func__, t->getTopic(), t->floatValue());
    }
}

void shackHeaterStatusUpdate(int x, Tag *t) {
    //printf("%s - [%s] %f\n", __func__, t->getTopic(), t->floatValue());
    if (t->boolValue()) {
        lv_led_on(shack_heater_led);
        lv_switch_on(shack_heater_switch, LV_ANIM_ON);
    } else {
        lv_led_off(shack_heater_led);
        lv_switch_off(shack_heater_switch, LV_ANIM_ON);
    }
}

void coolheat_create(lv_obj_t *parent) {
/*
    lv_page_set_style(parent, LV_PAGE_STYLE_BG, &lv_style_transp_fit);
    lv_page_set_style(parent, LV_PAGE_STYLE_SCRL, &lv_style_transp_fit);
*/

// page setup
    lv_page_set_scrllable_fit(parent, LV_FIT_PARENT);	// defined in lv_cont.h
    lv_page_set_scrl_height(parent, lv_obj_get_height(parent));
    lv_page_set_scrlbar_mode(parent, LV_SCRLBAR_MODE_OFF);

    lv_obj_t *label = lv_label_create(parent, NULL);
    lv_label_set_text(label, "Shack");
    lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_LEFT, 70, 70);

    shack_heater_led = lv_led_create(parent, NULL);
    lv_obj_set_size(shack_heater_led, 30,30);
    lv_obj_align(shack_heater_led, NULL, LV_ALIGN_IN_TOP_LEFT, 130, 25);
    lv_obj_add_style(shack_heater_led, LV_LED_PART_MAIN, &style_led_green);
    lv_led_off(shack_heater_led);

    shack_heater_switch = lv_switch_create(parent, NULL);
    lv_obj_set_size(shack_heater_switch, 80, 40);
    lv_obj_align(shack_heater_switch, NULL, LV_ALIGN_IN_TOP_LEFT, 20, 20);
    lv_obj_set_event_cb(shack_heater_switch, shack_heater_switch_action);
    Tag *t = ts.getTag(TOPIC_SHACK_HEATER_ENABLE);
    lv_obj_set_user_data(shack_heater_switch, { 0, t } );

    // Draw building outline
    static lv_point_t line_points[] = {{0, 0}, {799, 0}, {799, 426}, {0, 426}, {0, 0}};
    lv_obj_t *line1;
    line1 = lv_line_create(parent, NULL);
    lv_line_set_points(line1, line_points, 5);
    lv_obj_set_pos(line1, 0, 0);
    //lv_obj_set_y(line1, 100);
    //lv_obj_align(line1, NULL, LV_ALIGN_CENTER, 0, 0);

}

void heating_create(lv_obj_t *parent) {
/*
    lv_page_set_style(parent, LV_PAGE_STYLE_BG, &lv_style_transp_fit);
    lv_page_set_style(parent, LV_PAGE_STYLE_SCRL, &lv_style_transp_fit);
*/

    /* page setup */
    lv_page_set_scrllable_fit(parent, LV_FIT_PARENT);	// defined in lv_cont.h
    lv_page_set_scrl_height(parent, lv_obj_get_height(parent));
    lv_page_set_scrlbar_mode(parent, LV_SCRLBAR_MODE_OFF);

}

void temps_create(lv_obj_t *parent) {
/*
    lv_page_set_style(parent, LV_PAGE_STYLE_BG, &lv_style_transp_fit);
    lv_page_set_style(parent, LV_PAGE_STYLE_SCRL, &lv_style_transp_fit);
*/
    /* page setup */
    lv_page_set_scrllable_fit(parent, LV_FIT_PARENT);	// defined in lv_cont.h
    lv_page_set_scrl_height(parent, lv_obj_get_height(parent));
    lv_page_set_scrlbar_mode(parent, LV_SCRLBAR_MODE_OFF);

/*
    // Draw building outline
    static lv_point_t line_points[] = {{0, 0}, {750, 0}, {750, 300}, {0, 300}, {0, 0}};
    lv_obj_t *line1;
    line1 = lv_line_create(parent, NULL);
    lv_line_set_points(line1, line_points, 5);     // Set the points
    lv_obj_set_pos(line1, 0, 0);
    //lv_obj_align(line1, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
*/

    // Screen Room Temperature
    lv_obj_t *obj0 = lv_obj_create(parent, NULL);
    lv_obj_set_size(obj0, 150, 40);
    //lv_obj_set_style(obj0, &lv_style_plain_color);
    lv_obj_align(obj0, NULL, LV_ALIGN_IN_TOP_LEFT, 20, 20);
    lv_roomTemp[0] = lv_label_create(obj0, NULL);
    lv_label_set_text(lv_roomTemp[0], roomTempFormat[0]);
    lv_obj_align(lv_roomTemp[0], NULL, LV_ALIGN_CENTER, 0, 0);


    // Shack Temperature
    lv_obj_t *obj1 = lv_obj_create(parent, NULL);
    lv_obj_set_size(obj1, 150, 40);
    //lv_obj_set_style(obj1, &lv_style_plain_color);
    lv_obj_align(obj1, NULL, LV_ALIGN_IN_TOP_LEFT, 20, 80);
    lv_roomTemp[1] = lv_label_create(obj1, NULL);
    lv_label_set_text(lv_roomTemp[1], roomTempFormat[1]);
    lv_obj_align(lv_roomTemp[1], NULL, LV_ALIGN_CENTER, 0, 0);

    // Bedroom 1 Temperature
    lv_obj_t *obj2 = lv_obj_create(parent, NULL);
    lv_obj_set_size(obj2, 150, 40);
    //lv_obj_set_style(obj2, &lv_style_plain_color);
    lv_obj_align(obj2, parent, LV_ALIGN_IN_TOP_LEFT, 20, 140);
    lv_roomTemp[2] = lv_label_create(obj2, NULL);
    lv_label_set_text(lv_roomTemp[2], roomTempFormat[2]);
    lv_obj_align(lv_roomTemp[2], NULL, LV_ALIGN_CENTER, 0, 0);

}

void settings_create(lv_obj_t *parent) {
/*
    lv_page_set_style(parent, LV_PAGE_STYLE_BG, &lv_style_transp_fit);
    lv_page_set_style(parent, LV_PAGE_STYLE_SCRL, &lv_style_transp_fit);
*/

	/* page setup */
	lv_page_set_scrllable_fit(parent, LV_FIT_PARENT);	// defined in lv_cont.h
	lv_page_set_scrl_height(parent, lv_obj_get_height(parent));
	lv_page_set_scrlbar_mode(parent, LV_SCRLBAR_MODE_OFF);

    // Create brightness slider
    lv_obj_t *slider = lv_slider_create(parent, NULL);
    lv_obj_set_size(slider, LV_DPI / 3, lv_obj_get_height(parent) * 0.8);
    lv_obj_align(slider, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -40, -20);
    lv_obj_set_event_cb(slider, brightness_action);
    lv_slider_set_range(slider, 10, 200);
    lv_slider_set_value(slider, brightness_value, LV_ANIM_OFF);
    // Create a label top of brightness slider
    lv_obj_t * bightness_slider_label = lv_label_create(parent, NULL);
    lv_label_set_text(bightness_slider_label, "Brightness");
    lv_obj_align(bightness_slider_label, slider, LV_ALIGN_OUT_TOP_MID, 0, -20);

    // Reboot & Shutdown Button List
    lv_obj_t *list1 = lv_list_create(parent, NULL);
    lv_obj_set_size(list1, 150, 170);
    lv_obj_align(list1, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -120, 00);

    // Add list elements
    lv_obj_t *btn = lv_list_add_btn(list1, LV_SYMBOL_LOOP, " Reboot");
	lv_obj_set_event_cb(btn, pi_btn_action);
    lv_obj_set_user_data(btn, { SCR_CMD_REBOOT, NULL } );
    btn = lv_list_add_btn(list1, LV_SYMBOL_POWER, " Stop");
	lv_obj_set_event_cb(btn, pi_btn_action);
    lv_obj_set_user_data(btn, { SCR_CMD_SHUTDOWN, NULL } );

    // CPU Temperature
    lv_obj_t *obj1 = lv_obj_create(parent, NULL);
    lv_obj_set_size(obj1, 150, 40);
    //lv_obj_set_style(obj1, &lv_style_plain_color);
    lv_obj_align(obj1, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_cpuTemp = lv_label_create(obj1, NULL);
    lv_label_set_text(lv_cpuTemp, "CPU --.-°C");
    lv_obj_align(lv_cpuTemp, NULL, LV_ALIGN_CENTER, 0, 0);

    // Info Label
    lv_obj_t *info_label = lv_label_create(parent, NULL);
    lv_label_set_text(info_label, info_label_text);
    lv_obj_align(info_label, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 10, -10);

    /*
    // Create a label above the list
    lv_obj_t * label;
    label = lv_label_create(parent, NULL);
    lv_label_set_text(label, "Screen");
    lv_obj_align(label, list1, LV_ALIGN_OUT_TOP_MID, 0, -10);
    */
    /*
    // Create exit button
    lv_obj_t * btn1 = lv_btn_create(parent, NULL);
    lv_cont_set_fit(btn1, true, true); // Enable resizing horizontally and vertically
    lv_obj_align(btn1, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_free_num(btn1, 1);   // Set a unique number for the button
    lv_btn_set_action(btn1, LV_BTN_ACTION_CLICK, shutdown_btn_action);

    //Add a label to the button
    lv_obj_t * label = lv_label_create(btn1, NULL);
    lv_label_set_text(label, "Shutdown");

    // Create reboot button
    lv_obj_t * btn2 = lv_btn_create(parent, NULL);
    lv_cont_set_fit(btn2, true, true); //Enable resizing horizontally and vertically
    lv_obj_align(btn2, btn1, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    lv_obj_set_free_num(btn2, 2);   //Set a unique number for the button
    lv_btn_set_action(btn2, LV_BTN_ACTION_CLICK, reboot_btn_action);

    //Add a label to the button
    label = lv_label_create(btn2, NULL);
    lv_label_set_text(label, "Reboot");
    */
}


// Not Used
void __temps_create(lv_obj_t *parent) {
/*
    lv_page_set_style(parent, LV_PAGE_STYLE_BG, &lv_style_transp_fit);
    lv_page_set_style(parent, LV_PAGE_STYLE_SCRL, &lv_style_transp_fit);
*/
	/* page setup */
	lv_page_set_scrllable_fit(parent, LV_FIT_PARENT);	// defined in lv_cont.h
	lv_page_set_scrl_height(parent, lv_obj_get_height(parent));
	lv_page_set_scrlbar_mode(parent, LV_SCRLBAR_MODE_OFF);

/*
    static lv_style_t style_chart;
    lv_style_copy(&style_chart, &lv_style_pretty);
    style_chart.body.opa = LV_OPA_60;
    style_chart.body.radius = 0;
    style_chart.line.color = LV_COLOR_GRAY;
*/
    chart = lv_chart_create(parent, NULL);
    lv_obj_set_size(chart, 2 * lv_obj_get_width(parent) / 3, lv_obj_get_height(parent) / 2);
    lv_obj_align(chart, NULL,  LV_ALIGN_IN_TOP_MID, 0, LV_DPI / 4);
    lv_chart_set_type(chart, LV_CHART_TYPE_COLUMN);
    //lv_chart_set_style(chart, &style_chart);
    //lv_chart_set_series_opa(chart, LV_OPA_70);
    lv_chart_series_t *ser1;
    ser1 = lv_chart_add_series(chart, LV_COLOR_RED);
    lv_chart_set_next(chart, ser1, 40);
    lv_chart_set_next(chart, ser1, 30);
    lv_chart_set_next(chart, ser1, 47);
    lv_chart_set_next(chart, ser1, 59);
    lv_chart_set_next(chart, ser1, 59);
    lv_chart_set_next(chart, ser1, 31);
    lv_chart_set_next(chart, ser1, 55);
    lv_chart_set_next(chart, ser1, 70);
    lv_chart_set_next(chart, ser1, 82);

    // Create a bar, an indicator and a knob style
/*
    static lv_style_t style_bar;
    static lv_style_t style_indic;
    static lv_style_t style_knob;

    lv_style_copy(&style_bar, &lv_style_pretty);
    style_bar.body.main_color =  LV_COLOR_BLACK;
    style_bar.body.grad_color =  LV_COLOR_GRAY;
    style_bar.body.radius = LV_RADIUS_CIRCLE;
    style_bar.body.border.color = LV_COLOR_WHITE;
    style_bar.body.opa = LV_OPA_60;
    style_bar.body.padding.hor = 0;
    style_bar.body.padding.ver = LV_DPI / 10;

    lv_style_copy(&style_indic, &lv_style_pretty);
    style_indic.body.grad_color =  LV_COLOR_MARRON;
    style_indic.body.main_color =  LV_COLOR_RED;
    style_indic.body.radius = LV_RADIUS_CIRCLE;
    style_indic.body.shadow.width = LV_DPI / 10;
    style_indic.body.shadow.color = LV_COLOR_RED;
    style_indic.body.padding.hor = LV_DPI / 30;
    style_indic.body.padding.ver = LV_DPI / 30;

    lv_style_copy(&style_knob, &lv_style_pretty);
    style_knob.body.radius = LV_RADIUS_CIRCLE;
    style_knob.body.opa = LV_OPA_70;
*/
    // Create a second slider
    lv_obj_t *slider = lv_slider_create(parent, NULL);
    //lv_slider_set_style(slider, LV_SLIDER_STYLE_BG, &style_bar);
    //lv_slider_set_style(slider, LV_SLIDER_STYLE_INDIC, &style_indic);
    //lv_slider_set_style(slider, LV_SLIDER_STYLE_KNOB, &style_knob);
    lv_obj_set_size(slider, lv_obj_get_width(chart), LV_DPI / 3);
    lv_obj_align(slider, chart, LV_ALIGN_OUT_BOTTOM_MID, 0, (LV_VER_RES - chart->coords.y2 - lv_obj_get_height(slider)) / 2); /*Align to below the chart*/
    lv_obj_set_event_cb(slider, slider_action);
    lv_slider_set_range(slider, 10, 1000);
    lv_slider_set_value(slider, 700, LV_ANIM_OFF);
    slider_action(slider, LV_EVENT_VALUE_CHANGED);	/*Simulate a user value set the refresh the chart*/
}

/**
 * Called when the shack heater on/off switch is operated
 * @param obj: pointer to the switch object
 * @param event: switch event
 */
void shack_heater_switch_action(lv_obj_t *obj, lv_event_t event) {
    if(event == LV_EVENT_VALUE_CHANGED) {
        bool newState = lv_switch_get_state(obj);
        //printf("Shack heater switch State: %s\n", newState ? "On" : "Off");
        Tag *t = (Tag*)lv_obj_get_user_data(obj).pVal;
        if (t == NULL) {
            fprintf(stderr, "%s[%d] - failed to retrieve tag\n", __FILE__, __LINE__);
        } else {
            t->setValue(newState, true);
        }
    }
}

/**
 * Called when a new value on the slider on the Chart tab is set
 * @param obj: pointer to the slider object
 * @param event: slider event
 */
void slider_action(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_VALUE_CHANGED) {
        int16_t v = lv_slider_get_value(obj);
        v = 1000 * 100 / v; /* Convert to range modify values linearly */
        lv_chart_set_range(chart, 0, v);
    }
}

/**
 * Called when the brightness slider on the Settings tab is adjusted
 * @param obj: pointer to slider object
 * @param event: slider event
 */
void brightness_action(lv_obj_t *obj, lv_event_t event) {
    if(event == LV_EVENT_VALUE_CHANGED) {
        int16_t v = lv_slider_get_value(obj);
        brightness_value = v;
        scr_cmd = SCR_CMD_BRIGHTNESS;
    }
}

/**
 * Called when the brightness slider on the Settings tab is adjusted
 * @param obj: pointer to button object
 * @param event: button event
 */
void pi_btn_action(lv_obj_t *obj, lv_event_t event) {
    if(event == LV_EVENT_CLICKED) {
        scr_cmd = (scr_cmd_t)lv_obj_get_user_data(obj).iVal; // button number = screen command
        //printf("Button pressed: %d \n", lv_obj_get_user_data(obj).iVal);
    }
}
