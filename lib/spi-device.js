var SPIInterface = require("../Build/.../foo");

/**
 * State storage and pretty interface to the stateless
 *  SPIInterface binding.
 */
var SPI = module.exports = function() {
  this._handle = -1;
};

SPI.prototype.open = function(device, options, callback) {
  var spi = this;

  // Options and defaults
  this.device = device || "/dev/spidev0.0";
  this.mode = options.mode || SPIInterface.CONST.SPI_MODE_0;
  this.wordLength = options.wordLength || 8;
  this.speed = options.speed || 1000000;

  SPIInterface.open(this.device, this.mode,
    this.wordLength, this.speed, function(err, fd) {
    if(err) return callback(err);

    // Store new file-handle for future-like things
    spi._handle = fd;
    callback();
  });
};

SPI.prototype.transfer = function(send, callback) {
  // Allocate the receive buffer in user-land
  var receive = new Buffer(send.length);

  SPIInterface.transfer(this._handle, this.wordLength, this.speed,
    send, receive, callback);
};

SPI.prototype.close = function(callback) {
  SPIInterface.close(this._handle, callback);
};
