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
#ifndef LIBSPI_H
#define LIBSPI_H

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <cstdint>
#include <cstring>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#include <errno.h>
#include <exception>

using namespace std;

class SPIDevice {
public:
  SPIDevice();
  SPIDevice(const string &device_, uint8_t mode_, uint8_t bits_per_word_, uint32_t speed_);
  ~SPIDevice() {};

  void open();
  void transfer( uint8_t *data, uint32_t length);
  void close();
private:
  const string device;
  unsigned char mode;
  unsigned char bits_per_word;
  unsigned int speed;
  int spifd;

  void control(uint64_t request, void *argp, const string &message);
};

class SPIException: public exception {
public:
  SPIException(int32_t code_, const string &message_);

  virtual ~SPIException() throw() {};
  const char* what() const throw();
private:
  int32_t code;
  const string message;
};

#endif
