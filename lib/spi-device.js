var SPIInterface = require("../Build/.../foo");

var SPI = module.exports = function() {
  this._handle = -1;
};

SPI.prototype.open = function(device, options, callback) {
  var spi = this;

  SPIInterface.open(device, function(err, fd) {
    if(err) return callback(err);

    spi._handle = fd;
    callback();
  });
};

SPI.prototype.send = function(data, callback) {
  SPIInterface.send(this._handle, data, callback);
};

SPI.prototype.close = function(callback) {
  SPIInterface.close(this._handle, callback);
};
