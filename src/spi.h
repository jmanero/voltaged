/***********************************************************************
 * Original code from
 * http://hertaville.com/2013/07/24/interfacing-an-spi-adc-mcp3008-chip-to-the-raspberry-pi-using-c/
 *
 * This header file contains the SPIDevice class definition.
 * Its main purpose is to communicate with SPI-connected devices chip using
 * the userspace spidev facility.
 * The class contains four variables:
 * mode        -> defines the SPI mode used. In our case it is SPI_MODE_0.
 * bits_per_word      -> defines the bit width of the data transmitted.
 *        This is normally 8. Experimentation with other values
 *        didn't work for me
 * speed       -> Bus speed or SPI clock frequency. According to
 *                https://projects.drogon.net/understanding-spi-on-the-raspberry-pi/
 *            It can be only 0.5, 1, 2, 4, 8, 16, 32 MHz.
 *                Will use 1MHz for now and test it further.
 * spifd       -> file descriptor for the SPI device
 *
 * The class contains two constructors that initialize the above
 * variables and then open the appropriate spidev device using spiOpen().
 * The class contains one destructor that automatically closes the spidev
 * device when object is destroyed by calling spiClose().
 * The spiWriteRead() function sends the data "data" of length "length"
 * to the spidevice and at the same time receives data of the same length.
 * Resulting data is stored in the "data" variable after the function call.
 * ****************************************************************************/
#ifndef SPI_H
#define SPI_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <string>
#include <cstdint>
#include <cstring>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#include <v8.h>
#include <node.h>
#include <node_buffer.h>

// SPIInterface Operations
#define SPI_INTERFACE_OPEN 1
#define SPI_INTERFACE_TRANSFER 2
#define SPI_INTERFACE_CLOSE 3
#define SPI_INTERFACE_ERROR 255

using namespace v8;
using namespace std;

struct SPIBaton {
  // uv_work_t request;
  Persistent<Function> callback;
  int32_t error_code;
  std::string error_message;

  int32_t fd;
  uint8_t operation;

  char* device;
  uint8_t mode;
  uint8_t word;
  uint32_t speed;

  size_t length;
  uint8_t* send;
  uint8_t* receive;
};

namespace SPIDevice {
  void open(uv_work_t* req);
  bool control(SPIBaton* baton, uint64_t request, void* argp, const string &message);
  void close(uv_work_t* req);
  void transfer(uv_work_t* req);
};

namespace SPIInterface {
  using namespace node;

  uv_work_t* create_request(Local<Value> fd, uint8_t operation, Local<Value> callback);
  Handle<Value> validate_request(Local<Value> fd, Local<Value> callback);
  void Result(uv_work_t* req);
  Handle<Value> Open(const Arguments& args);
  Handle<Value> Transfer(const Arguments& args);
  Handle<Value> Close(const Arguments& args);
}

#endif
