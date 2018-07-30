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
-----------------------------------------------------------------------------
*/

#ifndef _DATATAG_H_
#define _DATATAG_H_

#include <stdint.h>

#include <iostream>
#include <string>

#define MAX_TAG_NUM 100         // The mximum number of tags which can be stored in TagList


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
     * Register a callback function to notify value changes
     * @param updatePtr a pointer to the upadate function
     */
    void registerCallback(void (*updateCallback) (int,Tag*), int callBackID );
    
    void testCallback();
    
    /**
     * Set the value
     * @param doubleValue: the new value
     */
    void setValue(double doubleValue);
    
    /**
     * Set the value
     * @param floatValue: the new value
     */
    void setValue(float floatValue);
    
    /**
     * Set the value
     * @param intValue: the new value
     */
    void setValue(int intValue);
    
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


private:
    // All properties of this class are private
    // Use setters & getters to access these values
    std::string topic;                  // storage for topic path
    uint16_t topicCRC;                  // CRC on topic path
    double topicDoubleValue;            // storage numeric value
    time_t lastUpdateTime;              // last update time (change of value)
    void (*valueUpdate) (int,Tag*);     // callback for value update
    int valueUpdateID;
    
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
    
private:
    Tag *tagList[MAX_TAG_NUM];          // An array references to Tags
    
};

#endif /* _DATATAG_H_ */
