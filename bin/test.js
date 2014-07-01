var SPI = require("../lib/spi");

var ch0 = new SPI();
ch0.open("/dev/spidev0.0", function(err) {
  if (err) return console.log(err);
  console.log("Open. Handle " + ch0._handle);

  var send = "Hello THere! I'm a long string of characters. ;lsdkfj;aasdfasdfd";
  console.log("Sending " + send.length + " bytes");

  ch0.transfer(Buffer(send, "ascii"), function(err, res) {
    if (err) return console.log(err);
    console.log("Received " + res.toString("utf8"));

    ch0.close(function(err) {
      if (err) return console.log(err);
      console.log("Closed");
    });

    console.log("Or close!");
  });
  console.log("Sweet! Neither did transfer");

});

console.log("Hey, look! The open call didn't block!");
