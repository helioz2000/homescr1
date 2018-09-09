/**
 * @file mcp9808.cpp
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>

#include "mcp9808.h"

#define MCP9808_DEFAULT_ADDRESS 0x18
#define I2C_BUS "/dev/i2c-1"		// Raspberry Pi is /dev/i2c-1

// uncomment below to enable debug output
//#define MCP9808_DEBUG


Mcp9808::Mcp9808() {
	//fprintf(stderr, "%s: Base Constructor called\n", __func__);
	init( MCP9808_DEFAULT_ADDRESS );
}

Mcp9808::Mcp9808(const unsigned char address) {
	//fprintf(stderr, "%s: Para Constructor called\n", __func__);
	init(address);
}

Mcp9808::~Mcp9808() {
    //fprintf(stderr, "%s: Destructor called\n", __func__);
  	if (_i2c_bus_file >= 0) {
		close(_i2c_bus_file);
		_i2c_bus_file = -1;
#ifdef MCP9808_DEBUG
		printf("%s: I2C bus closed\n", __func__);
#endif
	}
	if (_read_failure_count > 0) {
		syslog(LOG_NOTICE, "MCP9808 read failures: %ld", _read_failure_count);
	}
}

void Mcp9808::init(const unsigned char address) {
	_read_failure_count = 0;
	_i2c_address = address;
	_config_done = false;
	_i2c_bus_file = -1;

}

float Mcp9808::tempHist(float newTemp) {
	if (_tempHist[0] < -100.0) {
		_tempHist[0] = newTemp;
		return newTemp;
	}
	float diff = fabs(_tempHist[0] - newTemp);
	// if differential is too big ignore the new reading and return the old one
	if (diff > 10) {
		return _tempHist[0];
	}
	// shift old temperatures in FIFO
	for (int i=5; i<0; i--){
		_tempHist[i] = _tempHist[i-1];
	}
	// add new temp
	_tempHist[0] = newTemp;
	return newTemp;
}

bool Mcp9808::config(void) {

	for (int i=0; i<4; i++) {
		_tempHist[i] = -500.0;
	}

    if (_i2c_bus_file < 0) {
		return false;
	}

	// Get I2C device, MCP9808 I2C address is 0x18(24)
	ioctl(_i2c_bus_file, I2C_SLAVE, _i2c_address);

	// Select configuration register(0x01)
	// Continuous conversion mode, Power-up default(0x00, 0x00)
	char config[3] = {0};
	config[0] = 0x01;
	config[1] = 0x00;
	config[2] = 0x00;
	write(_i2c_bus_file, config, 3);

	config[0] = 0x08;		// Select resolution register(0x08)
	config[1] = 0x03;		// Resolution = +0.0625 / C(0x03)
	write(_i2c_bus_file, config, 2);

  _config_done = true;
	return true;
}

bool Mcp9808::readTempC(float *tempValue) {
	float cTemp;
  ssize_t bytes_written;

	if (_i2c_bus_file < 0) {
		if (!openI2Cbus()) {
			_read_failure_count++;
			return false;
		}
	}

	if (!_config_done) {
		if (!config()) {
			fprintf(stderr, "%s: config() failed \n", __func__);
			_read_failure_count++;
			return false;
		}
	}
	// Read 2 bytes of data from register(0x05)
	// temp msb, temp lsb
	char reg[1] = {0x05};
	bytes_written = write(_i2c_bus_file, reg, 1);
	if (bytes_written < 0) {
		  syslog(LOG_ERR, "I2C write data error: %s", strerror(errno));
			fprintf(stderr, "%s: I2C write data error: %s\n", __func__, strerror(errno));
			_read_failure_count++;
			return false;
	}
	if (bytes_written != 1) {
		  syslog(LOG_ERR, "I2C write data error ");
			fprintf(stderr, "%s: I2C write data error\n", __func__);
			_read_failure_count++;
			return false;
	}
	char data[2] = {0};
	if(read(_i2c_bus_file, data, 2) != 2) {
		syslog(LOG_ERR, "I2C read data error");
		fprintf(stderr, "%s: Input/Output error \n", __func__);
		_read_failure_count++;
		return false;
	}

	// Convert the data to 13-bits
	int temp = ((data[0] & 0x1F) * 256 + data[1]);
	if(temp > 4095) {
			temp -= 8192;
	}
	cTemp = tempHist(temp * 0.0625);

  /*
    //Sanity check
		if ( (cTemp == 1.8) || (cTemp == 0.0) || (cTemp > 80.0) || (cTemp < -20.0)  ) {
			syslog(LOG_ERR, "MCP9808 sanity check error [%f]", cTemp);
			return false;
		}
  */
#ifdef MCP9808_DEBUG
		// Debug output data to screen
		printf("Temperature: %.2f C \n", cTemp);
#endif
	*tempValue = cTemp;

	return true;
}

bool Mcp9808::readTempF(float *tempValue) {
	float cTemp;
	if (!readTempC(&cTemp)) {
		return false;
	}
	float fTemp = cTemp * 1.8 + 32;
	*tempValue = fTemp;
#ifdef MCP9808_DEBUG
	printf("Temperature: %.2f F \n", fTemp);
#endif
	return true;
}

bool Mcp9808::openI2Cbus(void) {
	const char *bus = I2C_BUS;
	if ((_i2c_bus_file = open(bus, O_RDWR)) < 0)
	{
		syslog(LOG_ERR, "Failed to open I2C bus [%s]", bus);
		fprintf(stderr, "%s: Failed to open I2C bus [%s] \n", __func__, bus);
		return false;
	} else {
#ifdef MCP9808_DEBUG
		printf("%s: I2C bus open success\n", __func__);
#endif
		return true;
	}
}
