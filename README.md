This sketch is now more then a PoC for utilizing multiple CDC interfaces & PIO to use a RP2040 or RP2350 as a flexible and large USB to UART converter.

## CLI/Config

The config CLI should present itself on the last ACM that the Pico creates, when no other ACM devices are present it will be /dev/ttyACM6. You can access it on Linux via screen like this :

```screen /dev/ttyACM6 19200```

You need to press enter once to see a "# ", but you can just enter a valid command and it will work.

![image](https://github.com/user-attachments/assets/85cb82e0-e867-432e-867c-0288d2c56957)

It saves the config on the internal EEPROM.

## TinyUSB library adaptation

TinyUSB was never made for this purpose. I tried to fix that with a large hammer. It worked.

You need to adapt the header file named Adafruit_TinyUSB_Arduino/src/arduino/ports/rp2040/tusb_config_rp2040.h and set CFG_TUD_CDC to be at least 8.

You also need to widen the _desc_cfg_buffer to 1024 in the header file Adafruit_TinyUSB_Arduino/src/arduino/Adafruit_USBD_Device.h

## Default Pinout

```
/dev/ttyACM0 => 2 (TX); 3 (RX) | PIO
/dev/ttyACM1 => 4 (TX); 5 (RX) | PIO
/dev/ttyACM2 => 6 (TX); 7 (RX) | PIO
/dev/ttyACM3 => 8 (TX); 9 (RX) | HW-UART
/dev/ttyACM4 => 10 (TX); 11 (RX) | PIO
/dev/ttyACM5 => 12 (TX); 13 (RX) | HW-UART
```
