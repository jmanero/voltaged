/**
 * Original code from
 * http://hertaville.com/2013/07/24/interfacing-an-spi-adc-mcp3008-chip-to-the-raspberry-pi-using-c/
 */
#include "spi.h"

/**
 * Open an SPI device. Pass the file-handle back to user-space.
 */
void SPIDevice::open(uv_work_t* req) {
  SPIBaton* baton = static_cast<SPIBaton*>(req->data);

  if (baton->fd = ::open(baton->device.c_str(), O_RDWR) < 0) {
    baton->error_code = errno;
    baton->error_message = "Unable to open SPI device";
    return;
  }

  control(baton, SPI_IOC_WR_MODE, &(baton->mode), "Could not set SPI Write Mode");
  control(baton, SPI_IOC_RD_MODE, &(baton->mode), "Could not set SPI Read Mode");
  control(baton, SPI_IOC_WR_BITS_PER_WORD, &(baton->word), "Could not set SPI Write Bits-per-Word");
  control(baton, SPI_IOC_RD_BITS_PER_WORD, &(baton->word), "Could not set SPI Read Bits-per-Word");
  control(baton, SPI_IOC_WR_MAX_SPEED_HZ, &(baton->speed), "Could not set SPI Write Speed");
  control(baton, SPI_IOC_RD_MAX_SPEED_HZ, &(baton->speed), "Could not set SPI Read Speed");
}

/**
 * Close the device
 */
void SPIDevice::close(uv_work_t* req) {
  SPIBaton* baton = static_cast<SPIBaton*>(req->data);

  if(::close(baton->fd) < 0){
    baton->error_code = errno;
    baton->error_message = "Unable to open SPI device";
  }

  baton->fd = -1;
}

/**
 * Write/read on the SPI file-handle. Replaces data on the
 * baton with received data.
 */
void SPIDevice::transfer(uv_work_t* req) {
  SPIBaton* baton = static_cast<SPIBaton*>(req->data);

  size_t length = Buffer::Length(*(baton->send));
  uint8_t* send = Buffer::Data(*(baton->send));
  uint8_t* receive = Buffer::Data(*(baton->receive));

  struct spi_ioc_transfer transfer_[length];
  for (uint32_t i = 0; i < length; i++){
    transfer_[i].tx_buf = (uint64_t)(send + i);
    transfer_[i].rx_buf = (uint64_t)(receive + i);

    transfer_[i].len = length;
    transfer_[i].speed_hz = baton->speed;
    transfer_[i].bits_per_word = baton->word;
    transfer_[i].delay_usecs = 0 ;
    transfer_[i].cs_change = 0;
  }

  control(baton, SPI_IOC_MESSAGE(length), &transfer_, "Unable to transmit/receive");
}

/**
 * Wrap ioctl calls with error handling
 */
bool SPIDevice::control(SPIBaton* baton, uint64_t request, void* argp, const string &message) {
  if(ioctl(baton->fd, request, argp) < 0) {
    baton->error_code = errno;
    baton->error_message = message;
    return false;
  }
  return true;
}
