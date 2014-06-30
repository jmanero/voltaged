/**
 * Original code from
 * http://hertaville.com/2013/07/24/interfacing-an-spi-adc-mcp3008-chip-to-the-raspberry-pi-using-c/
 */
#include "device.h"

/**********************************************************
 * open() :function is called by the constructor.
 * It is responsible for opening the spidev device
 * "devspi" and then setting up the spidev interface.
 * private member variables are used to configure spidev.
 * They must be set appropriately by constructor before calling
 * this function.
 * *********************************************************/
void SPIDevice::open() {
  this->spifd = ::open(this->device.c_str(), O_RDWR);
  if (this->spifd < 0) throw SPIException(this->spifd, "Unable to open SPI device");

  this->control(SPI_IOC_WR_MODE, &(this->mode), "Could not set SPI Write Mode");
  this->control(SPI_IOC_RD_MODE, &(this->mode), "Could not set SPI Read Mode");
  this->control(SPI_IOC_WR_BITS_PER_WORD, &(this->bits_per_word), "Could not set SPI Write Bits-per-Word");
  this->control(SPI_IOC_RD_BITS_PER_WORD, &(this->bits_per_word), "Could not set SPI Read Bits-per-Word");
  this->control(SPI_IOC_WR_MAX_SPEED_HZ, &(this->speed), "Could not set SPI Write Speed");
  this->control(SPI_IOC_RD_MAX_SPEED_HZ, &(this->speed), "Could not set SPI Read Speed");
}

/***********************************************************
 * control(): Wrap ioctl calls on the SPI file handle
 ***********************************************************/
void SPIDevice::control(uint64_t request, void *argp, const string &message) {
  if(ioctl(this->spifd, request, argp) < 0)
    throw SPIException(errno, message);
}

/***********************************************************
 * close(): Responsible for closing the spidev interface.
 * Called in destructor
 * *********************************************************/
void SPIDevice::close(){
  if(::close(this->spifd) < 0)
    throw SPIException(errno, "Unable to close SPI device");
}

/********************************************************************
 * This function writes data "data" of length "length" to the spidev
 * device. Data shifted in from the spidev device is saved back into
 * "data".
 * ******************************************************************/
void SPIDevice::transfer( uint8_t *data, uint32_t length) {
  struct spi_ioc_transfer transfer_[length];

  for (uint32_t i = 0; i < length; i++){
    transfer_[i].tx_buf = (uint64_t)(data + i); // transmit from "data"
    transfer_[i].rx_buf = (uint64_t)(data + i); // receive into "data"
    transfer_[i].len = sizeof(*(data + i));
    transfer_[i].delay_usecs = 0 ;
    transfer_[i].speed_hz = this->speed;
    transfer_[i].bits_per_word = this->bits_per_word;
    transfer_[i].cs_change = 0;
  }

  this->control(SPI_IOC_MESSAGE(length), &transfer_, "Unable to transmit/receive");
}

/*************************************************
 * Default constructor. Set member variables to
 * default values and then call open()
 * ***********************************************/
SPIDevice::SPIDevice(): device() {
  SPIDevice("/dev/spidev0.0", SPI_MODE_0, 8, 1000000);
}

/*************************************************
 * overloaded constructor. let user set member variables to
 * and then call open()
 * ***********************************************/
SPIDevice::SPIDevice(const string &device_, uint8_t mode_,
  uint8_t bits_per_word_, uint32_t speed_): device(device_) {
  this->mode = mode_;
  this->bits_per_word = bits_per_word_;
  this->speed = speed_;
  this->spifd = -1;
}

SPIException::SPIException(uint8_t code_, const string &message_): message(message_) {
  this->code = code_;
}

const char* SPIException::what() const throw() {
  return (this->message + " (" + to_string(this->code) + "): " + strerror(this->code)).c_str();
}
