{
  "targets": [
    {
      "target_name": "spi-device",
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "cflags": [ "-std=c++0x" ],
      "sources": [ "src/interface.cc", "src/device.cc" ]
    }
  ]
}
