/**
 * @file screen.h
 *
 */

#ifndef SCREEN_H
#define SCREEN_H

#ifdef __cplusplus
extern "C" {
#endif
    
/*********************
 *      INCLUDES
 *********************/
    
/*********************
 *      DEFINES
 *********************/
#define SCREEN_UPDATE    5

/**********************
 *      TYPEDEFS
 **********************/
    typedef enum
    {
        SCR_CMD_NONE = 0,
        SCR_CMD_SHUTDOWN,
        SCR_CMD_REBOOT,
        SCR_CMD_BRIGHTNESS,
    }scr_cmd_t;
    
/**********************
 *   GLOBAL PROTOTYPES
 **********************/

    void screen_init();
    void screen_create(void);
    void screen_exit(void);
    scr_cmd_t screen_getCmd(void);
    void screen_clearCmd();
    int16_t screen_brightness(void);
    void screen_set_brightness(int16_t value);
    
#ifdef __cplusplus
}
#endif

#endif /* SCREEN_H */
