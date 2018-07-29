/**
 * @file screen.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include <stddef.h>
#include <stdio.h>

#include "lvgl/lvgl.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"

#include "screen.h"

/**********************
 *   PRIVATE VARIABLES
 **********************/
char *info_label_text;
int16_t brightness_current_value;
scr_cmd_t scr_cmd;

lv_indev_drv_t indev_drv;
lv_res_t slider_action(lv_obj_t *slider);
lv_res_t brightness_action(lv_obj_t *slider);
lv_res_t pi_btn_action(lv_obj_t * btn);

lv_obj_t *tv;
lv_obj_t *tab1;
lv_obj_t *tab2;
lv_obj_t *tab3;
lv_obj_t *tab4;
lv_obj_t *chart;

/**********************
 *   PRIVATE PROTOTYPES
 **********************/
void aircon_create(lv_obj_t *parent);
void heating_create(lv_obj_t *parent);
void temps_create(lv_obj_t *parent);
void settings_create(lv_obj_t *parent);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void screen_init()
{
    brightness_current_value = 10;
    scr_cmd = SCR_CMD_NONE;
    
    lv_init();
    
    evdev_init();               // initialise event device
    //lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read = evdev_read;    // defined in lv_drivers/indev/evdev.h
    lv_indev_drv_register(&indev_drv);

    fbdev_init();
    
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.disp_flush = fbdev_flush;      // It flushes the internal graphical buffer to the frame buffer
    lv_disp_drv_register(&disp_drv);
}

/**
 * Create a screen application
 */
void screen_create(void)
{
    /*
    lv_obj_t *wp = lv_img_create(lv_scr_act(), NULL);
    lv_img_set_src(wp, &img_bubble_pattern);
    lv_obj_set_width(wp, LV_HOR_RES * 4);
    lv_obj_set_protect(wp, LV_PROTECT_POS);
    */
    
    static lv_style_t style_tv_btn_bg;
    lv_style_copy(&style_tv_btn_bg, &lv_style_plain);
    style_tv_btn_bg.body.main_color = LV_COLOR_HEX(0x487fb7);
    style_tv_btn_bg.body.grad_color = LV_COLOR_HEX(0x487fb7);
    style_tv_btn_bg.body.padding.ver = 0;
    
    static lv_style_t style_tv_btn_rel;
    lv_style_copy(&style_tv_btn_rel, &lv_style_btn_rel);
    style_tv_btn_rel.body.empty = 1;
    style_tv_btn_rel.body.border.width = 0;
    
    static lv_style_t style_tv_btn_pr;
    lv_style_copy(&style_tv_btn_pr, &lv_style_btn_pr);
    style_tv_btn_pr.body.radius = 0;
    style_tv_btn_pr.body.opa = LV_OPA_50;
    style_tv_btn_pr.body.main_color = LV_COLOR_WHITE;
    style_tv_btn_pr.body.grad_color = LV_COLOR_WHITE;
    style_tv_btn_pr.body.border.width = 0;
    style_tv_btn_pr.text.color = LV_COLOR_GRAY;
    
    tv = lv_tabview_create(lv_scr_act(), NULL);
    
    tab1 = lv_tabview_add_tab(tv, "AirCon");
    tab2 = lv_tabview_add_tab(tv, "Heating");
    tab3 = lv_tabview_add_tab(tv, "Temps");
    tab4 = lv_tabview_add_tab(tv, "Settings");
 
    lv_tabview_set_style(tv, LV_TABVIEW_STYLE_BTN_BG, &style_tv_btn_bg);
    lv_tabview_set_style(tv, LV_TABVIEW_STYLE_INDIC, &lv_style_plain);
    lv_tabview_set_style(tv, LV_TABVIEW_STYLE_BTN_REL, &style_tv_btn_rel);
    lv_tabview_set_style(tv, LV_TABVIEW_STYLE_BTN_PR, &style_tv_btn_pr);
    lv_tabview_set_style(tv, LV_TABVIEW_STYLE_BTN_TGL_REL, &style_tv_btn_rel);
    lv_tabview_set_style(tv, LV_TABVIEW_STYLE_BTN_TGL_PR, &style_tv_btn_pr);
    
    aircon_create(tab1);
    heating_create(tab2);
    temps_create(tab3);
    settings_create(tab4);
    /*
     #if LV_DEMO_SLIDE_SHOW
     lv_task_create(tab_switcher, 3000, LV_TASK_PRIO_MID, tv);
     #endif
     */
}

scr_cmd_t screen_getCmd(void) {
    return scr_cmd;
}

void screen_clearCmd(void) {
    scr_cmd = SCR_CMD_NONE;
}

int16_t screen_current_brightness(void) {
    return brightness_current_value;
}

void screen_set_current_brightness(int16_t value) {
    brightness_current_value = value;
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
void aircon_create(lv_obj_t *parent)
{
}

void heating_create(lv_obj_t *parent)
{
}

void settings_create(lv_obj_t *parent)
{
    lv_page_set_style(parent, LV_PAGE_STYLE_BG, &lv_style_transp_fit);
    lv_page_set_style(parent, LV_PAGE_STYLE_SCRL, &lv_style_transp_fit);
    
    lv_page_set_scrl_fit(parent, false, false);
    lv_page_set_scrl_height(parent, lv_obj_get_height(parent));
    lv_page_set_sb_mode(parent, LV_SB_MODE_OFF);
    

    // Create brightness slider
    lv_obj_t *slider = lv_slider_create(parent, NULL);
    lv_obj_set_size(slider, LV_DPI / 3, lv_obj_get_height(parent) * 0.8);
    lv_obj_align(slider, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -40, -20);
    lv_slider_set_action(slider, brightness_action);
    lv_slider_set_range(slider, 10, 200);
    lv_slider_set_value(slider, brightness_current_value);
    // Create a label top of brightness slider
    lv_obj_t * bightness_slider_label = lv_label_create(parent, NULL);
    lv_label_set_text(bightness_slider_label, "Brightness");
    lv_obj_align(bightness_slider_label, slider, LV_ALIGN_OUT_TOP_MID, 0, -20);
    
    // Reboot & Shutdown Button List
    lv_obj_t *list1 = lv_list_create(parent, NULL);
    lv_obj_set_size(list1, 130, 170);
    lv_obj_align(list1, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -120, 00);
    
    // Add list elements
    lv_obj_t *btn = lv_list_add(list1, NULL, "Reboot", pi_btn_action);
    lv_obj_set_free_num(btn, SCR_CMD_REBOOT);
    btn = lv_list_add(list1, NULL, "Stop", pi_btn_action);
    lv_obj_set_free_num(btn, SCR_CMD_SHUTDOWN);
    
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

void temps_create(lv_obj_t *parent)
{
    lv_page_set_style(parent, LV_PAGE_STYLE_BG, &lv_style_transp_fit);
    lv_page_set_style(parent, LV_PAGE_STYLE_SCRL, &lv_style_transp_fit);
    
    lv_page_set_scrl_fit(parent, false, false);
    lv_page_set_scrl_height(parent, lv_obj_get_height(parent));
    lv_page_set_sb_mode(parent, LV_SB_MODE_OFF);
    
    static lv_style_t style_chart;
    lv_style_copy(&style_chart, &lv_style_pretty);
    style_chart.body.opa = LV_OPA_60;
    style_chart.body.radius = 0;
    style_chart.line.color = LV_COLOR_GRAY;
    
    chart = lv_chart_create(parent, NULL);
    lv_obj_set_size(chart, 2 * lv_obj_get_width(parent) / 3, lv_obj_get_height(parent) / 2);
    lv_obj_align(chart, NULL,  LV_ALIGN_IN_TOP_MID, 0, LV_DPI / 4);
    lv_chart_set_type(chart, LV_CHART_TYPE_COLUMN);
    lv_chart_set_style(chart, &style_chart);
    lv_chart_set_series_opa(chart, LV_OPA_70);
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
    
    /*Create a bar, an indicator and a knob style*/
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
    
    /*Create a second slider*/
    lv_obj_t *slider = lv_slider_create(parent, NULL);
    lv_slider_set_style(slider, LV_SLIDER_STYLE_BG, &style_bar);
    lv_slider_set_style(slider, LV_SLIDER_STYLE_INDIC, &style_indic);
    lv_slider_set_style(slider, LV_SLIDER_STYLE_KNOB, &style_knob);
    lv_obj_set_size(slider, lv_obj_get_width(chart), LV_DPI / 3);
    lv_obj_align(slider, chart, LV_ALIGN_OUT_BOTTOM_MID, 0, (LV_VER_RES - chart->coords.y2 - lv_obj_get_height(slider)) / 2); /*Align to below the chart*/
    lv_slider_set_action(slider, slider_action);
    lv_slider_set_range(slider, 10, 1000);
    lv_slider_set_value(slider, 700);
    slider_action(slider);          /*Simulate a user value set the refresh the chart*/
}

/**
 * Called when a new value on the slider on the Chart tab is set
 * @param slider pointer to the slider
 * @return LV_RES_OK because the slider is not deleted in the function
 */
lv_res_t slider_action(lv_obj_t *slider)
{
    int16_t v = lv_slider_get_value(slider);
    v = 1000 * 100 / v; /*Convert to range modify values linearly*/
    lv_chart_set_range(chart, 0, v);
    
    return LV_RES_OK;
}

/**
 * Called when the brightness slider on the Settings tab is adjusted
 * @param slider pointer to the slider
 * @return LV_RES_OK because the slider is not deleted in the function
 */
lv_res_t brightness_action(lv_obj_t *slider)
{
    int16_t v = lv_slider_get_value(slider);
    brightness_current_value = v;
    scr_cmd = SCR_CMD_BRIGHTNESS;
    return LV_RES_OK;
}

lv_res_t pi_btn_action(lv_obj_t * btn)
{
    scr_cmd = lv_obj_get_free_num(btn); // button number = screen command
    return LV_RES_OK; // Return OK if the button is not deleted
}

