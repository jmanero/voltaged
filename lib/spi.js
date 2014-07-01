var SPIInterface = require("../build/Release/SPIInterface");

/**
 * State storage and pretty interface to the stateless
 *  SPIInterface binding.
 */
var SPI = module.exports = function(device, options) {
  options = options || {};

  // Options and defaults
  this._handle = -1;
  this.device = device || "/dev/spidev0.0";
  this.mode = options.mode || SPIInterface.CONST.SPI_MODE_0;
  this.wordLength = options.wordLength || 8;
  this.speed = options.speed || 1000000; // 1MHz
};

SPI.prototype.open = function(callback) {
  var spi = this;

  SPIInterface.open(this.device, this.mode,
    this.wordLength, this.speed, function(err, fd) {
    if(err) return callback(err);

    spi._handle = fd; // Store new file-handle for future-like things
    callback();
  });
};

SPI.prototype.transfer = function(send, callback) {
  /**
   * Allocate the receive buffer in JS-land. Turns out there is a negligible
   * performance difference, and making buffers in CPP is a huge pain...
   */
  var receive = new Buffer(send.length);

  SPIInterface.transfer(this._handle, this.wordLength, this.speed,
    send, receive, function(err) {
      callback(err, receive);
    });
};

SPI.prototype.close = function(callback) {
  SPIInterface.close(this._handle, callback);
};
