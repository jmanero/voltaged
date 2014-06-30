/**
 * Original code from
 * http://hertaville.com/2013/07/24/interfacing-an-spi-adc-mcp3008-chip-to-the-raspberry-pi-using-c/
 */
#include "spi.h"

/**********************************************************
 * open() :function is called by the constructor.
 * It is responsible for opening the spidev device
 * "devspi" and then setting up the spidev interface.
 * private member variables are used to configure spidev.
 * They must be set appropriately by constructor before calling
 * this function.
 * *********************************************************/
void SPIDevice::open(uv_work_t* req) {
  SPIBaton* baton = static_cast<SPIBaton*>(req->data);
  SPISetup* setup = static_cast<SPIBaton*>(baton->payload);

  if (baton->fd = ::open(setup->device.c_str(), O_RDWR) < 0) {
    baton->error_code = errno;
    baton->error_message = "Unable to open SPI device";
    return;
  }

  if(control(baton, SPI_IOC_WR_MODE, &(setup->mode), "Could not set SPI Write Mode")) return;
  if(control(baton, SPI_IOC_RD_MODE, &(setup->mode), "Could not set SPI Read Mode")) return;
  if(control(baton, SPI_IOC_WR_BITS_PER_WORD, &(setup->word), "Could not set SPI Write Bits-per-Word")) return;
  if(control(baton, SPI_IOC_RD_BITS_PER_WORD, &(setup->word), "Could not set SPI Read Bits-per-Word")) return;
  if(control(baton, SPI_IOC_WR_MAX_SPEED_HZ, &(setup->speed), "Could not set SPI Write Speed")) return;
  if(control(baton, SPI_IOC_RD_MAX_SPEED_HZ, &(setup->speed), "Could not set SPI Read Speed")) return;

  // All done with the payload object
  delete baton->setup;
  baton->setup = NULL;
}

/***********************************************************
 * control(): Wrap ioctl calls on the SPI file handle
 ***********************************************************/
bool SPIDevice::control(SPIBaton* baton, uint64_t request, void* argp, const string &message) {
  if(ioctl(baton->fd, request, argp) < 0) {
    baton->error_code = errno;
    baton->error_message = message;
    return true;
  }
  return false;
}

/***********************************************************
 * close(): Responsible for closing the spidev interface.
 * Called in destructor
 * *********************************************************/
void SPIDevice::close(uv_work_t* req) {
  SPIBaton* baton = static_cast<SPIBaton*>(req->data);

  if(::close(baton->fd) < 0){
    baton->error_code = errno;
    baton->error_message = "Unable to open SPI device";
  }
}

/********************************************************************
 * This function writes data "data" of length "length" to the spidev
 * device. Data shifted in from the spidev device is saved back into
 * "data".
 * ******************************************************************/
void SPIDevice::transfer(uv_work_t* req) {
  SPIBaton* baton = static_cast<SPIBaton*>(req->data);
  SPITransfer* transfer = static_cast<SPIBaton*>(baton->payload);

  uint8_t* data = transfer->data;
  struct spi_ioc_transfer transfer_[transfer->length];

  for (uint32_t i = 0; i < length; i++){
    transfer_[i].tx_buf = (uint64_t)(data + i); // transmit from "data"
    transfer_[i].rx_buf = (uint64_t)(data + i); // receive into "data"
    transfer_[i].len = sizeof(*(data + i));
    transfer_[i].delay_usecs = 0 ;
    transfer_[i].speed_hz = this->speed;
    transfer_[i].bits_per_word = this->bits_per_word;
    transfer_[i].cs_change = 0;
  }

  control(baton, SPI_IOC_MESSAGE(length), &transfer_, "Unable to transmit/receive");
}
