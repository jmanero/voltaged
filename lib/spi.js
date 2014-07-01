var Queue = require("./queue");
var SPIInterface = require("../build/Release/SPIInterface");

/**
 * State storage and pretty interface for the stateless
 *  SPIInterface binding.
 */
var SPI = module.exports = function(device, options) {
  options = options || {};

  // Options and defaults
  this._handle = -1;
  this.queue = new Queue({
    paused: true
  });

  this.device = device || "/dev/spidev0.0";
  this.mode = options.mode || SPIInterface.CONST.SPI_MODE_0;
  this.wordLength = options.wordLength || 8;
  this.speed = options.speed || 1000000; // 1MHz
};

SPI.prototype.open = function(callback) {
  var spi = this;
  if(!(callback instanceof Function))
    throw TypeError("SPI::open(callback) must be called with a callback");
  if(this._handle !== -1)
    throw Error("SPI device has already been opened");

  SPIInterface.open(this.device, this.mode,
    this.wordLength, this.speed, function(err, fd) {

    spi._handle = fd; // Store new file-handle for future-like things
    spi.queue.resume();

    callback.call(spi, err);
  });
};

SPI.prototype.transfer = function(send, callback) {
  var spi = this;
  if(!(callback instanceof Function))
    throw TypeError("SPI::transfer(send, callback) must be called with a callback");
  if(!Buffer.isBuffer(send))
    throw TypeError("SPI::transfer(send, callback) must be called with a buffer");

  /**
   * Allocate the receive buffer in JS-land. Turns out there is a negligible
   * performance difference, and making buffers safely in CPP is a huge pain...
   */
  var receive = new Buffer(send.length);
  receive.fill("z");

  this.queue.push(SPIInterface.transfer.bind(null, this._handle, this.wordLength,
    this.speed, send, receive), function(err) {
      callback.call(spi, err, receive);
    });
};

SPI.prototype.close = function(callback) {
  var spi = this;
  if(!(callback instanceof Function))
    throw TypeError("SPI::close(callback) must be called with a callback");
  if(this._handle === -1)
    throw Error("SPI device is not open");

  this.queue.pause();
  SPIInterface.close(this._handle, function(err) {
    callback.call(spi, err);
  });
};
