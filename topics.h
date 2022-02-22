/**
 * @file topics.h
 *
 * Author: Erwin Bejsta
 * May 2020
 *
 * define topics for MQTT broker
 */

#ifndef TOPICS_H
#define TOPICS_H


#define TOPIC_CPU_TEMP "binder/home/screen1pi/cpu/temp"
#define TOPIC_ENV_TEMP "binder/home/screen1/env/temp"
#define TOPIC_BED1_ROOM_TEMP "binder/home/bed1/room/temp"
#define TOPIC_BALCONY_TEMP "binder/home/balcony/temp"
#define TOPIC_BALCONY_HUMIDITY "binder/home/balcony/humidity"
#define TOPIC_SHACK_HEATER_ENABLE "binder/home/shack/heater/enable"
#define TOPIC_SHACK_ROOM_TEMP "binder/home/shack/room/temp"
#define TOPIC_SHACK_HEATER_TEMP_SP "binder/home/shack/heater/temp_sp"
#define TOPIC_SHACK_HEATER_CONTROL "binder/home/shack/heater/control"
#define TOPIC_SHACK_RADIO240_PWR1 "binder/home/shack/power240radio/pwr1oncommand"
#define TOPIC_SHACK_RADIO240_PWR2 "binder/home/shack/power240radio/pwr2oncommand"


static const char *Topic_Shack_Radio12_Pwr[] = {"binder/home/shack/power12radio/pwr1oncommand",
	"binder/home/shack/power12radio/pwr2oncommand",
	"binder/home/shack/power12radio/pwr3oncommand",
	"binder/home/shack/power12radio/pwr4oncommand",
	"binder/home/shack/power12radio/pwr5oncommand",
	"binder/home/shack/power12radio/pwr6oncommand",
	"binder/home/shack/power12radio/pwr7oncommand",
	"binder/home/shack/power12radio/pwr8oncommand" };

#endif /* TOPICS_H */
