/**
 * @file datatag.cpp
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
#include "datatag.h"

#include <stdexcept>
#include <iostream>

/*********************
 *      DEFINES
 *********************/
#define CRC16 0x8005
using namespace std;

/*********************
 * GLOBAL FUNCTIONS
 *********************/

/**
 * Generate CRC16 for data
 * @param data: sequence of bytes
 * @param size: number of bytes
 */
static uint16_t gen_crc16(const char *data, uint16_t size)
{
    uint16_t out = 0;
    int bits_read = 0, bit_flag;

    /* Sanity check: */
    if(data == NULL)
        return 0;

    while(size > 0)
    {
        bit_flag = out >> 15;

        /* Get next bit: */
        out <<= 1;
        out |= (*data >> bits_read) & 1; // item a) work from the least significant bits

        /* Increment bit counter: */
        bits_read++;
        if(bits_read > 7)
        {
            bits_read = 0;
            data++;
            size--;
        }

        /* Cycle check: */
        if(bit_flag)
            out ^= CRC16;

    }

    // item b) "push out" the last 16 bits
    int i;
    for (i = 0; i < 16; ++i) {
        bit_flag = out >> 15;
        out <<= 1;
        if(bit_flag)
            out ^= CRC16;
    }

    // item c) reverse the bits
    uint16_t crc = 0;
    i = 0x8000;
    int j = 0x0001;
    for (; i != 0; i >>=1, j <<= 1) {
        if (i & out) crc |= j;
    }

    return crc;
}

/*********************
 * MEMBER FUNCTIONS
 *********************/

//
// Class Tag
//

Tag::Tag() {
    printf("%s\n", __func__);
    throw runtime_error("Class Tag - forbidden constructor");
}

Tag::Tag(const char *topicStr) {

    if (topicStr == NULL) {
        throw invalid_argument("Class Tag - topic is NULL");
    }
    this->topic = topicStr;
    this->_format = "";
	this->_noreadStr = "";
	_noreadStatus = true;
	_noreadValue = 0.0;
    _valueUpdateCB = NULL;
    _valueUpdateID = -1;
    _publishTagCB = NULL;
    _publishTagID = -1;
    _publish = false;
    _subscribe = false;
    _retain = false;
    _type = TAG_TYPE_NUMERIC;
    //cout << topic << endl;
    topicCRC = gen_crc16(topic.data(), topic.length());
    //cout << topicCRC << endl;
}

Tag::~Tag() {
    //printf("%s - %s\n", __func__, topic.c_str());
}

const char* Tag::getTopic(void) {
    return topic.c_str();
}

uint16_t Tag::getTopicCrc(void) {
    return topicCRC;
}

void Tag::setFormat(const char *formatStr) {
    this->_format = formatStr;
}

const char* Tag::getFormat(void) {
    return _format.c_str();
}

void Tag::setNoreadStr(const char *noreadStr) {
	this->_noreadStr = noreadStr;
}

const char* Tag::getNoreadStr(void) {
	return _noreadStr.c_str();
}

void Tag::setNoreadValue(float newValue) {
	this->_noreadValue = newValue;
}

float Tag::getNoreadValue(void) {
	return _noreadValue;
}

bool Tag::getFormattedValueStr(char *buffer, int buflen, const char *formatStr) {
	int result = 0;
	if(_noreadStatus) {
		if(formatStr) {
			result = snprintf(buffer, buflen, formatStr, _noreadValue);
		} else {
			result = snprintf(buffer, buflen, _noreadStr.c_str());
		}
	} else {
		if(formatStr) {
			result = snprintf(buffer, buflen, formatStr, this->floatValue());
		} else {
			result = snprintf(buffer, buflen, _format.c_str(), this->floatValue());
		}
	}
	if (result > 0) return true;
	else return false;
}

void Tag::registerUpdateCallback(void (*updateCallback) (int, Tag*), int callBackID) {
    //printf("%s - 1\n", __func__);
    _valueUpdateCB = updateCallback;
    _valueUpdateID = callBackID;
    //printf("%s - 2\n", __func__);
}

void Tag::registerPublishCallback(void (*publishCallback) (int, Tag*), int callBackID) {
    _publishTagCB = publishCallback;
    _publishTagID = callBackID;
}

int Tag::valueUpdateID(void) {
    return _valueUpdateID;
}

int Tag::publishTagID(void) {
    return _publishTagID;
}

void Tag::testCallback() {
    if (_valueUpdateCB != NULL) {
        (*_valueUpdateCB) (_valueUpdateID, this);
    }
}

void Tag::setNoread(bool newNoread) {
	_noreadStatus = newNoread;
}

void Tag::performCallbacks(bool publishMe) {
	    // Publish new value if required 
    if (_publish && publishMe) {
        //printf("%s[%d] - publishing <%s>\n", __FILE__, __LINE__, topic.c_str());
        // call publishTag callback if it exists
        if (_publishTagCB != NULL) {
            (*_publishTagCB) (_publishTagID, this); }
    } else {    // otherwise perform local value update
        // call valueUpdate callback if it exists
        if (_valueUpdateCB != NULL) {
            (*_valueUpdateCB) (_valueUpdateID, this); }
    }
}

/**
 * this function is called in two separate and distinct cases:
 * 1) when an update is received from and external source (eg mqtt broker)
 * 2) when an update is received from internal source (eg local hardware or user interface)
 * external updates need to be propagated to internal screen elements or 
 * locally connected hardware.
 * internal updates need to be propagated to external infrastructure (eg mqtt broker)
 * NOTE: some tags can be publish and subscribe (i.e. read & write)
 */
void Tag::setValue(double doubleValue, bool publishMe) {
    topicDoubleValue = doubleValue;
    lastUpdateTime = time(NULL);
    setNoread(false);
	performCallbacks(publishMe);
}

void Tag::setValue(float floatValue, bool publishMe) {
    setValue( (double) floatValue, publishMe );
}

void Tag::setValue(int intValue, bool publishMe) {
    setValue( (double) intValue, publishMe );
}

void Tag::setValue(bool boolValue, bool publishMe) {
    setValue( (double) boolValue, publishMe );
    //printf("%s[%d] - setValue <%s>\n", __FILE__, __LINE__, topic.c_str());
}

/**
 * Note: strValue can be NULL when MQTT clears a retained message
 */

bool Tag::setValue(const char* strValue, bool publishMe) {
	float newValue = 0;
	int result = 0;
	// handle "noread" (or clear value) case?
	if (strValue == NULL) {
		newValue = _noreadValue;
		setNoread(true);
		result = 1;
	} else {
		switch (_type) {
			case TAG_TYPE_NUMERIC:
				result = sscanf(strValue, "%f", &newValue);
				break;
			case TAG_TYPE_BOOL:
				if ( (strValue[0] == 'f') || (strValue[0] == 'F') ) {
					newValue = 0; result = 1; }
				if ( (strValue[0] == 't') || (strValue[0] == 'T') ) {
					newValue = 1; result = 1; }
				break;
		}
		_noreadStatus = false;
	}
	if (result != 1) {
		fprintf(stderr, "%s - failed to setValue <%s> for topic %s\n", __func__, strValue, topic.c_str());
		return false;
	}
	if (!_noreadStatus) {
		setValue(newValue, publishMe);
	} else {
		performCallbacks(publishMe);
	}
    return true;
}

double Tag::doubleValue(void) {
    return topicDoubleValue;
}

float Tag::floatValue(void) {
    return (float) topicDoubleValue;
}

int Tag::intValue(void) {
    return (int) topicDoubleValue;
}

bool Tag::boolValue(void) {
    if (topicDoubleValue != 0) {
        return true; }
    else {
        return false; }
}

void Tag::setNoreadStatus(bool newStatus) {
	_noreadStatus = newStatus;
}

bool Tag::getNoreadStatus(void) {
	return _noreadStatus;
}

bool Tag::isPublish() {
    return _publish;
}

bool Tag::isSubscribe() {
    return _subscribe;
}

void Tag::setPublish(void) {
    _publish = true;
}

void Tag::setSubscribe(void) {
    _subscribe = true;
}

void Tag::setRetain(bool newRetain) {
    _retain = newRetain;
}

bool Tag::getRetain(void) {
    return _retain;
}

void Tag::setType(tag_type_t newType) {
    _type = newType;
}

tag_type_t Tag::type(void) {
    return _type;
}
//
// Class TagStore
//

TagStore::TagStore() {
    // mark tag list entries as empty
    for (int i = 0; i < MAX_TAG_NUM; i++) {
        tagList[i] = NULL;
    }
    iterateIndex = -1;
}

TagStore::~TagStore() {
    deleteAll();
}

void TagStore::deleteAll(void) {
    // delete every tag
    for (int i = 0; i < MAX_TAG_NUM; i++) {
        delete(tagList[i]);
        tagList[i] = NULL;
    }
}

Tag *TagStore::getTag(const char* tagTopic) {

    string topic (tagTopic);
    uint16_t tagCrc = gen_crc16(topic.data(), topic.length());
    Tag *retTag =  NULL;

    for (int index = 0; index < MAX_TAG_NUM; index++) {
        if (tagCrc == tagList[index]->getTopicCrc()) {
            retTag = tagList[index];
            break;
        }
    }
    return retTag;
}

Tag* TagStore::getFirstTag(void) {
    int index;
    // find first free entry in tag list
    for (index = 0; index < MAX_TAG_NUM; index++) {
        if (tagList[index] != NULL) {
            iterateIndex = index;
            return tagList[index];
        }
    }
    return NULL;    // no tags found
}

Tag* TagStore::getNextTag(void) {
    int index;
    // check if getFirstTag has been called
    if (iterateIndex < 0) return NULL;
    // find first free entry in tag list
    for (index = iterateIndex+1; index < MAX_TAG_NUM; index++) {
        if (tagList[index] != NULL) {
            iterateIndex = index;
            return tagList[index];
        }
    }
    // No more tags found
    iterateIndex = -1; // reset iterateIndex
    return NULL;
}

Tag* TagStore::addTag(const char* tagTopic) {
    int index, freeIndex = -1;
    // find first free entry in tag list
    for (index = 0; index < MAX_TAG_NUM; index++) {
        if (tagList[index] == NULL) {
            freeIndex = index;
            break;
        }
    }
    // abort if tagList is full
    if (freeIndex < 0) return NULL;
    // create new tag and store in list
    Tag *tPtr = new Tag(tagTopic);
    tagList[index] = tPtr;
    //printf("%s - [%d] - %s\n", __func__, index, tPtr->getTopic());
    return tPtr;
}
