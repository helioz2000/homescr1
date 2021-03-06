/**
 * @file datatag.h

-----------------------------------------------------------------------------
 Two classes provide encapsulation for typical use of a data tag in an
 automation oriented user interface.
 This implementation is targeted data which is based on the MQTT protocol
 and stores the data access information as a topic path (see MQTT details)

 Class "Tag" encapsulates a single data unit
 Class "TagList" provides a facility to manage a list of tags

 The Tag class provides for a callback interface which is intended to update
 a user interface element (e.g. display value) only when data changes

 The "publish" member defines if a tag's value is published (written)
 to an mqtt broker or if it is subscribed (read from MQTT broker).
 This information is used outside this class.
-----------------------------------------------------------------------------
*/

#ifndef _DATATAG_H_
#define _DATATAG_H_

/*********************
 *      INCLUDES
 *********************/
#include <stdint.h>

#include <iostream>
#include <string>

/*********************
 *      DEFINES
 *********************/
#define MAX_TAG_NUM 100         // The mximum number of tags which can be stored in TagList

/**********************
 *      TYPEDEFS
 **********************/
    typedef enum
    {
        TAG_TYPE_NUMERIC = 0,
        TAG_TYPE_BOOL = 1
    }tag_type_t;


class Tag {
public:
    /**
     * Invalid - Empty constructor throws runtime error
     */
    Tag();

    /**
     * Constructor
     * @param topic: tag topic
     */
    Tag(const char* topicStr);

    /**
     * Destructor
     */
    ~Tag();

    /**
     * Get topic CRC
     * @return the CRC for the topic string
     */
    uint16_t getTopicCrc(void);

    /**
     * Get the topic string
     * @return the topic string
     */
    const char* getTopic(void);

    /**
     * Set the format string
     * @param formatStr: the format string
     */
    void setFormat(const char *formatStr);

    /**
     * Get the format string
     * @return the format string
     */
    const char* getFormat(void);

	/**
 	* Set the noread string
 	* @param noreadStr: the format string
 	*/
 	void setNoreadStr(const char *noreadStr);
 
 	/**
 	* Get the noread string
 	* @return the noread string
 	*/
 	const char* getNoreadStr(void);

	/**
	 * Set noread value
	 * @param newValue: the new noread value
	 */
	void setNoreadValue(float newValue);

	/**
	 * get noread value
	 * @returns: the noread value float
	 */
	float getNoreadValue(void);

	/**
	 * get formatted value string
	 * @param buffer: char buffer for formatted string
	 * @param buflen: length of buffer in bytes
	 * @param formatStr: printf format string, if NULL the object's format string is used
	 * @returns: true on success
	 */
	bool getFormattedValueStr(char *buffer, int buflen, const char *formatStr);

    /**
     * Register a callback function to notify value changes
     * @param function ptr: a pointer to the update function
     */
    void registerUpdateCallback(void (*updateCallback) (int,Tag*), int callBackID );

    /**
     * Register a callback function to to publish tag
     * @param function ptr: a pointer to the publish function
     */
    void registerPublishCallback(void (*publishCallback) (int,Tag*), int callBackID );

    /**
     * request value update callback ID
     * @return the current value update callback ID set with registerCallback
     */
    int valueUpdateID();
    
    /**
     * request publish tag callback ID
     * @return the current publishTag ID
     */
    int publishTagID(void);

    void testCallback();

    /**
     * Set the value
     * @param doubleValue: the new value
     */
    void setValue(double doubleValue, bool publishMe = false);

    /**
     * Set the value
     * @param floatValue: the new value
     */
    void setValue(float floatValue, bool publishMe = false);

    /**
     * Set the value
     * @param intValue: the new value
     */
    void setValue(int intValue, bool publishMe = false);

    /**
     * Set the value
     * @param boolValue: the new value
     */
    void setValue(bool boolValue, bool publishMe = false);


    /**
     * Set the value
     * @param strValue: new value as a string
     * @returns true on success
     */
    bool setValue(const char* strValue, bool publishMe = false);

    /**
     * Get value
     * @return value as double
     */
    double doubleValue(void);

    /**
     * Get value
     * @return value as float
     */
    float floatValue(void);

    /**
     * Get value
     * @return value as int
     */
    int intValue(void);

    /**
     * Get value
     * @return value as bool
     */
    bool boolValue(void);

    /**
     * Get formatted value
     * @return value as char *
     */
    //const char * formattedValue(void);

	/**
	 * set noread status
	 * @para newStatus: the new noread status
	 */
	void setNoreadStatus(bool newStatus);

	/**
	 * get noread status
	 * returns: teh current noread status
	 */
	bool getNoreadStatus(void);

    /**
     * is tag "publish"
     * @return true if publish 
     */
    bool isPublish();

    /**
     * is tag "subscribe"
     * @return true if subscribe
     */
    bool isSubscribe();

    /**
     * Mark tag as "publish" (can be publish + subscribe)
     */
    void setPublish(void);

    /**
     * Mark tag as "subscribe" (can be publish + subscribe)
     */
    void setSubscribe(void);

    /**
     * Set tag retain for publishing
     */
    void setRetain(bool newRetain);

    /**
     * Get tag retain setting
     */
    bool getRetain(void);

    /**
     * Set tag type (see tag_type_t)
     */
    void setType(tag_type_t newType);
    
    /**
     * Get tag type (see tag_type_t)
     */
    tag_type_t type(void);

private:

	void performCallbacks(bool publishMe);
	void setNoread(bool newNoread);

    /**
     * All properties of this class are private
     * Use setters & getters to access class members
     */
    std::string topic;                  // storage for topic path
    std::string _format;                 // publishing format (eg %.1f)
	std::string _noreadStr;				// display this when value is not available
    uint16_t topicCRC;                  // CRC on topic path
	float _noreadValue;					// Value to be used when noread is active
    double topicDoubleValue;            // storage numeric value
    time_t lastUpdateTime;              // last update time (change of value)
    void (*_valueUpdateCB) (int,Tag*);	// callback - called on external value update
    void (*_publishTagCB) (int,Tag*);	// callback - called to publish value
    int _valueUpdateID;                 // ID for value update callback
    int _publishTagID;                  // ID for tag publish callback
	bool _noreadStatus;					// true = value not available or invalid
    bool _publish;                       // true = publish to topic when value changes
    bool _subscribe;                     // true = subscribe to this topic
    bool _retain;                       // retain option sent to broker on publish 
    tag_type_t _type;                   // data type
};

class TagStore {
public:
    TagStore();
    ~TagStore();

    /**
     * Add a tag
     * @param tagTopic: the topic as a string
     * @return reference to new tag or NULL on failure
     */
    Tag* addTag(const char* tagTopic);

    /**
     * Delete all tags from tag list
     */
    void deleteAll(void);

    /**
     * Find tag in the list and return reference
     * @param tagTopic: the topic as a string
     * @return reference to tag or NULL is if not found
     */
    Tag* getTag(const char* tagTopic);

    /**
     * Get first tag from store
     * use in conjunction with getNextTag to iterate over all tags
     * @return reference to first tag or NULL if tagList is empty
     */
    Tag* getFirstTag(void);

    /**
     * Get next tag from store
     * use in conjunction with getFirstTag to iterate over all tags
     * @return reference to next tag or NULL if end of tagList
     */
     Tag* getNextTag(void);

private:
    Tag *tagList[MAX_TAG_NUM];     // An array references to Tags
    int iterateIndex;              // to interate over all tags in store
};

#endif /* _DATATAG_H_ */
