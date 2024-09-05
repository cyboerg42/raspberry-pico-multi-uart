This sketch is a PoC for using multiple CDC interfaces and PIO on a RP2040.

CFG_TUD_CDC must be at least 8.

Config file is located in the library. (Adafruit_TinyUSB_Arduino/src/arduino/ports/rp2040/tusb_config_rp2040.h)

You need also to widen the _desc_cfg_buffer to 1024.

It's located in Adafruit_TinyUSB_Arduino/src/arduino/Adafruit_USBD_Device.h


```
/dev/ttyACM0 => 2 (TX); 3 (RX) | PIO
/dev/ttyACM1 => 4 (TX); 5 (RX) | PIO
/dev/ttyACM2 => 6 (TX); 7 (RX) | PIO
/dev/ttyACM3 => 8 (TX); 9 (RX) | HW-UART
/dev/ttyACM4 => 10 (TX); 11 (RX) | PIO
/dev/ttyACM5 => 12 (TX); 13 (RX) | HW-UART
```
