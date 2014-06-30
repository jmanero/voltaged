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
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <cstdint>
#include <cstring>
#include <errno.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#include <v8.h>
#include <node.h>

// SPIInterface Operations
#define SPI_INTERFACE_OPEN 1
#define SPI_INTERFACE_TRANSFER 2
#define SPI_INTERFACE_CLOSE 3
#define SPI_INTERFACE_ERROR 255

using namespace std;
using namespace node;
using namespace v8;

struct SPIBaton {
  // uv_work_t request;
  Persistent<Function> callback;
  int32_t error_code;
  string error_message;

  int32_t fd;
  uint8_t operation;
  void* payload;
};

struct SPITransfer {
  uint32_t length;
  uint8_t* data;
}

struct SPISetup {
  string device;
  uint8_t mode;
  uint8_t word;
  uint8_t speed;
}

namespace SPIDevice {
  void open(uv_work_t* req);
  bool control(SPIBaton* baton, uint64_t request, void* argp, const string &message);
  void close(uv_work_t* req);
  void transfer(uv_work_t* req);
};

namespace SPIInterface {
  void Request(uint32_t fd, uint8_t operation, void* payload, Persistent<Function> callback);
  void Result(uv_work_t* req)

}

#endif
