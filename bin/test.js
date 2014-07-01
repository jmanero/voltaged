var SPI = require("../lib/spi");

var channel = new SPI("/dev/spidev0.0", {
  speed: 5000
});

channel.open(function(err) {
  if (err) return console.log(err);
  console.log("Open. Handle " + this._handle);

  var i = 0;
  (function context_loop() {
    var b = new Buffer(16);
    b.fill(i + "");

    channel.transfer(b, function(err, res) {
      if (err) return console.log(err);
      console.log(res);
    });

    i++;
    if(i < 4) context_loop();
  })();
  console.log("Sweet! Neither did transfer");

});

// Close after a while...
setTimeout(function() {
  channel.close(function(err) {
    console.log("Closed");
    if (err) return console.log(err);
  });
}, 10000);

console.log("Hey, look! The open call didn't block!");
