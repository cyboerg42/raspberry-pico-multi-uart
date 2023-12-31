// /dev/ttyACM0 => 2 (TX); 3 (RX) | PIO
// /dev/ttyACM1 => 4 (TX); 5 (RX) | PIO
// /dev/ttyACM2 => 6 (TX); 7 (RX) | PIO
// /dev/ttyACM3 => 8 (TX); 9 (RX) | HW-UART
// /dev/ttyACM4 => 10 (TX); 11 (RX) | PIO
// /dev/ttyACM5 => 12 (TX); 13 (RX) | HW-UART

#include <Adafruit_TinyUSB.h>

// define the baudrate used for all ports, you could define custom ones for each one in the setup()
#define BAUDRATE 19200

// LED Stuff
#define LED LED_BUILTIN
int LEDstate = 0;

// define SerialPIO ports 
SerialPIO ser3(2,3);
SerialPIO ser4(4,5);
SerialPIO ser5(6,7);
SerialPIO ser6(10,11);

// define CDC interfaces; Serial will already point to it per default.
Adafruit_USBD_CDC USBSer1;
Adafruit_USBD_CDC USBSer2;
Adafruit_USBD_CDC USBSer3;
Adafruit_USBD_CDC USBSer4;
Adafruit_USBD_CDC USBSer5;

void setup() {
  // set LED pin to GPIO output mode
  pinMode(LED, OUTPUT); 
  // start CDC interfaces
  Serial.begin(BAUDRATE);
  USBSer1.begin(BAUDRATE);
  USBSer2.begin(BAUDRATE);
  USBSer3.begin(BAUDRATE);
  USBSer4.begin(BAUDRATE);
  USBSer5.begin(BAUDRATE);
  // why?! but it won't work without...
  delay(1000);
  // set pins of HW interfaces
  Serial1.setTX(12);
  Serial1.setRX(13);
  Serial2.setTX(8);
  Serial2.setRX(9);
  // start HW & PIO interfaces
  Serial1.begin(BAUDRATE);
  Serial2.begin(BAUDRATE);
  ser3.begin(BAUDRATE);
  ser4.begin(BAUDRATE);
  ser5.begin(BAUDRATE);
  ser6.begin(BAUDRATE);
}

// runs on core 0; core 0 also handles USB and various other stuff.
void loop() {
  // UART Buffer
  int ch;
  // USB CDC -> UART
  ch = Serial.read();
  if (ch > 0) ser3.write(ch);
  ch = USBSer1.read();
  if (ch > 0) ser4.write(ch);
  ch = USBSer2.read();
  if (ch > 0) ser5.write(ch);
  ch = USBSer3.read();
  if (ch > 0) Serial2.write(ch);
  ch = USBSer4.read();
  if (ch > 0) ser6.write(ch);
  ch = USBSer5.read();
  if (ch > 0) Serial1.write(ch);
  // blink LED
  if (delay_without_delaying(500)) {
    LEDstate = !LEDstate;
    digitalWrite(LED, LEDstate);
  }
}

// runs on core 1
void loop1() {
  // UART buffer
  int ch;
  // UART -> USB CDC
  ch = ser3.read();
  if (ch > 0) Serial.write(ch);
  ch = ser4.read();
  if (ch > 0) USBSer1.write(ch);
  ch = ser5.read();
  if (ch > 0) USBSer2.write(ch);
  ch = Serial2.read();
  if (ch > 0) USBSer3.write(ch);
  ch = ser6.read();
  if (ch > 0) USBSer4.write(ch);
  ch = Serial1.read();
  if (ch > 0) USBSer5.write(ch);
}

// Helper: non-blocking "delay" alternative.
boolean delay_without_delaying(unsigned long time) {
  // return false if we're still "delaying", true if time ms has passed.
  static unsigned long previousmillis = 0;
  unsigned long currentmillis = millis();
  if (currentmillis - previousmillis >= time) {
    previousmillis = currentmillis;
    return true;
  }
  return false;
}
