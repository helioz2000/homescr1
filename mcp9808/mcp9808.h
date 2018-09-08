/**
 * @file mcp9808.h
-----------------------------------------------------------------------------
 Read MCP9808 temperature sensor via I2C bus
-----------------------------------------------------------------------------
*/

#ifndef _MCP9808_H_
#define _MCP9808_H_

#include <stdint.h>

class Mcp9808 {
public:
    /**
     * Empty constructor - use default I2C address
     */
    Mcp9808();

    /**
     * Constructor
     * @param address: I2C address of MCP9808
     */
    Mcp9808(const unsigned char address);

    /**
     * Destructor
     */
    ~Mcp9808();

    /**
     * Read temperature
     * @return the actual temperature
     */
    bool readTempC(float *tempValue);
    bool readTempF(float *tempValue);

private:
    // All properties of this class are private
    // Use setters & getters to access these values

    int _i2c_bus_file;          // file used to access I2C bus
    unsigned char _i2c_address;       // address of mcp9808
    bool _config_done;
    unsigned long _read_failure_count;

    bool openI2Cbus(void);
    bool config(void);
    void init(const unsigned char address);

};


#endif /* _MCP9808_H_ */
