#ifndef _DATATAG_H_
#define _DATATAG_H_

#include <stdint.h>

#include <iostream>
#include <string>

#define MAX_TAG_NUM 50

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------

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
    
    std::string topic;
    uint16_t topicCRC;
    double topicDoubleValue;
    time_t lastUpdateTime;
    void (*valueUpdate) (int,Tag*);      // callback for value update
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
    Tag *tagList[MAX_TAG_NUM];
    
};

#endif /* _DATATAG_H_ */
