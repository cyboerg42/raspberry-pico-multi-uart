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
SerialPIO ser3(2, 3);
SerialPIO ser4(4, 5);
SerialPIO ser5(6, 7);
SerialPIO ser6(10, 11);

// define CDC interfaces; Serial will already point to it per default.
Adafruit_USBD_CDC USBSer1;
Adafruit_USBD_CDC USBSer2;
Adafruit_USBD_CDC USBSer3;
Adafruit_USBD_CDC USBSer4;
Adafruit_USBD_CDC USBSer5;

void setup() {
  // set LED pin to GPIO output mode
  pinMode(LED, OUTPUT);
  
  // Start all serial and USB interfaces
  initSerialAndUSB();

  // set pins of HW interfaces
  configureHardwareUART();

  // start HW & PIO interfaces
  startAllInterfaces();
}

void initSerialAndUSB() {
  Serial.begin(BAUDRATE);
  USBSer1.begin(BAUDRATE);
  USBSer2.begin(BAUDRATE);
  USBSer3.begin(BAUDRATE);
  USBSer4.begin(BAUDRATE);
  USBSer5.begin(BAUDRATE);
  
  // A small delay to ensure all USB CDCs are initialized properly
  delay(500); 
}

void configureHardwareUART() {
  Serial1.setTX(12);
  Serial1.setRX(13);
  Serial2.setTX(8);
  Serial2.setRX(9);
}

void startAllInterfaces() {
  Serial1.begin(BAUDRATE);
  Serial2.begin(BAUDRATE);
  ser3.begin(BAUDRATE);
  ser4.begin(BAUDRATE);
  ser5.begin(BAUDRATE);
  ser6.begin(BAUDRATE);
}

// runs on core 0; core 0 also handles USB and various other stuff.
void loop() {
  USBtoUART();
  LED();
}

// runs on core 1
void loop1() {
  UARTtoUSB();
}

void USBtoUART() {
  //  USB CDC -> UART communication
  forwardData(Serial, ser3);
  forwardData(USBSer1, ser4);
  forwardData(USBSer2, ser5);
  forwardData(USBSer3, Serial2);
  forwardData(USBSer4, ser6);
  forwardData(USBSer5, Serial1);
}

void forwardData(Stream &input, Stream &output) {
  int ch = input.read();
  if (ch > 0) {
    output.write(ch);
  }
}

void LED() {
  // blink LED
  if (delay_without_delaying(500)) {
    LEDstate = !LEDstate;
    digitalWrite(LED, LEDstate);
  }
}

void UARTtoUSB() {
  // UART -> USB CDC communication
  forwardData(ser3, Serial);
  forwardData(ser4, USBSer1);
  forwardData(ser5, USBSer2);
  forwardData(Serial2, USBSer3);
  forwardData(ser6, USBSer4);
  forwardData(Serial1, USBSer5);
}

// Helper: non-blocking "delay" alternative.
boolean delay_without_delaying(unsigned long time) {
  static unsigned long previousmillis = 0;
  unsigned long currentmillis = millis();
  if (currentmillis - previousmillis >= time) {
    previousmillis = currentmillis;
    return true;
  }
  return false;
}
