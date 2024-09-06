// /dev/ttyACM0 => 2 (TX); 3 (RX) | PIO
// /dev/ttyACM1 => 4 (TX); 5 (RX) | PIO
// /dev/ttyACM2 => 6 (TX); 7 (RX) | PIO
// /dev/ttyACM3 => 8 (TX); 9 (RX) | HW-UART
// /dev/ttyACM4 => 10 (TX); 11 (RX) | PIO
// /dev/ttyACM5 => 12 (TX); 13 (RX) | HW-UART

#include <Adafruit_TinyUSB.h>
#include <EEPROM.h>

// EEPROM constants
#define EEPROM_VALID_FLAG 0xA5
#define EEPROM_SIZE 128

// define the baudrate used for all ports
#define BAUDRATE 9600

// LED Stuff
#define LED LED_BUILTIN
int LEDstate = 0;

// define SerialPIO ports (initialized with default pins)
SerialPIO ser3(2, 3);   // ACM0
SerialPIO ser4(4, 5);   // ACM1
SerialPIO ser5(6, 7);   // ACM2
SerialPIO ser6(10, 11); // ACM4

// define CDC interfaces; Serial will already point to ACM0 per default.
Adafruit_USBD_CDC USBSer1;  // ACM1
Adafruit_USBD_CDC USBSer2;  // ACM2
Adafruit_USBD_CDC USBSer3;  // ACM3
Adafruit_USBD_CDC USBSer4;  // ACM4
Adafruit_USBD_CDC USBSer5;  // ACM5
Adafruit_USBD_CDC USBSerCmd; // Command interface
String cmdBuffer = "";


int serialBaudRates[6] = {BAUDRATE, BAUDRATE, BAUDRATE, BAUDRATE, BAUDRATE, BAUDRATE}; // Store baudrates
int serialTXPins[6] = {2, 4, 6, 8, 10, 12};  // Default TX pins
int serialRXPins[6] = {3, 5, 7, 9, 11, 13};  // Default RX pins

void setup() {
  EEPROM.begin(EEPROM_SIZE); // Initialize EEPROM
  pinMode(LED, OUTPUT);      // Set LED pin to output
  
  // Load configuration from EEPROM or initialize with default values
  if (!loadConfigFromEEPROM()) {
    initDefaultConfig();
    saveConfigToEEPROM();
  }

  // Start all serial and USB interfaces
  initSerialAndUSB();
  configureHardwareUART();
  startAllInterfaces();
  USBSerCmd.print("# ");
}

void initSerialAndUSB() {
  Serial.begin(serialBaudRates[0]); // ACM0
  USBSer1.begin(serialBaudRates[1]); // ACM1
  USBSer2.begin(serialBaudRates[2]); // ACM2
  USBSer3.begin(serialBaudRates[3]); // ACM3
  USBSer4.begin(serialBaudRates[4]); // ACM4
  USBSer5.begin(serialBaudRates[5]); // ACM5
  USBSerCmd.begin(BAUDRATE);         // Command interface
  delay(500); // Ensure USB CDCs are initialized
}

void configureHardwareUART() {
  Serial1.setTX(serialTXPins[5]);
  Serial1.setRX(serialRXPins[5]);
  Serial2.setTX(serialTXPins[3]);
  Serial2.setRX(serialRXPins[3]);
}

void startAllInterfaces() {
  Serial1.begin(serialBaudRates[5]);
  Serial2.begin(serialBaudRates[3]);
  ser3.begin(serialBaudRates[0]);
  ser4.begin(serialBaudRates[1]);
  ser5.begin(serialBaudRates[2]);
  ser6.begin(serialBaudRates[4]);
}

void loop() {
  USBtoUART();
  handleLED();
  handleCmd();
}

void loop1() {
  UARTtoUSB();
}

void USBtoUART() {
  forwardData(Serial, ser3);
  forwardData(USBSer1, ser4);
  forwardData(USBSer2, ser5);
  forwardData(USBSer3, Serial2);
  forwardData(USBSer4, ser6);
  forwardData(USBSer5, Serial1);
}

void forwardData(Stream &input, Stream &output) {
  while (input.available()) {
    int ch = input.read();
    if (ch >= 0) output.write(ch);
  }
}

void handleLED() {
  if (delay_without_delaying(500)) {
    LEDstate = !LEDstate;
    digitalWrite(LED, LEDstate);
  }
}

void UARTtoUSB() {
  forwardData(ser3, Serial);
  forwardData(ser4, USBSer1);
  forwardData(ser5, USBSer2);
  forwardData(Serial2, USBSer3);
  forwardData(ser6, USBSer4);
  forwardData(Serial1, USBSer5);
}

boolean delay_without_delaying(unsigned long time) {
  static unsigned long previousmillis = 0;
  unsigned long currentmillis = millis();
  if (currentmillis - previousmillis >= time) {
    previousmillis = currentmillis;
    return true;
  }
  return false;
}

void handleCmd() {
  while (USBSerCmd.available()) {
    char ch = USBSerCmd.read();
    if (ch == '\r' || ch == '\n') {
      USBSerCmd.print("\n\r");
      processCmd(cmdBuffer);
      cmdBuffer = "";
    } else if (ch == '\b' && cmdBuffer.length() > 0) {
      cmdBuffer.remove(cmdBuffer.length() - 1);
      USBSerCmd.print("\b \b");
      USBSerCmd.flush();
    } else if (ch == 127 && cmdBuffer.length() > 0) {
      cmdBuffer.remove(cmdBuffer.length() - 1);
      USBSerCmd.print("\b \b");
      USBSerCmd.flush();
    } else if (ch >= 32 && ch <= 126) {
      cmdBuffer += ch;
      USBSerCmd.write(ch);
      USBSerCmd.flush();
    }
  }
}

void processCmd(String cmd) {
  if (cmd.startsWith("reset_baud")) {
    resetBaudRates();
  } else if (cmd.startsWith("set_baud")) {
    setBaudRate(cmd);
  } else if (cmd.startsWith("set_pin")) {
    setSerialPins(cmd);
  } else if (cmd.startsWith("reset_pins")) {
    resetPins();
  } else if (cmd.startsWith("print_config")) {
    printConfig();
  } else {
    USBSerCmd.println("Unknown command");
    printHelp();
  }
  USBSerCmd.print("# ");
  USBSerCmd.flush();
}

void printHelp() {
  USBSerCmd.println("Available commands:");
  USBSerCmd.println("  set_baud X Y       - Set baud rate of ACMX to Y");
  USBSerCmd.println("  set_pin X TX RX    - Set the TX and RX pins of ACMX");
  USBSerCmd.println("  reset_baud         - Reset all baud rates to default");
  USBSerCmd.println("  reset_pins         - Reset all pins to default");
  USBSerCmd.println("  print_config       - Print current configuration (baud rates and pins)");
}

void resetBaudRates() {
  for (int i = 0; i < 6; i++) serialBaudRates[i] = BAUDRATE;
  saveConfigToEEPROM();
  USBSerCmd.println("Baud rates reset to default.");
}

void resetPins() {
  int defaultTX[] = {2, 4, 6, 8, 10, 12};
  int defaultRX[] = {3, 5, 7, 9, 11, 13};
  
  for (int i = 0; i < 6; i++) {
    serialTXPins[i] = defaultTX[i];
    serialRXPins[i] = defaultRX[i];
  }
  configureHardwareUART();
  saveConfigToEEPROM();
  USBSerCmd.println("Pins reset to default.");
}

void setBaudRate(String cmd) {
  int acmIndex, baud;
  if (sscanf(cmd.c_str(), "set_baud %d %d", &acmIndex, &baud) == 2 && acmIndex >= 0 && acmIndex <= 5) {
    serialBaudRates[acmIndex] = baud;

    // Restart the ACM and the corresponding HW UART interface with the updated baud rate
    if (acmIndex == 0) {
      ser3.end();
      ser3.begin(serialBaudRates[0]);
      Serial.begin(serialBaudRates[0]);
    } else if (acmIndex == 1) {
      ser4.end();
      ser4.begin(serialBaudRates[1]);
      USBSer1.end();
      USBSer1.begin(serialBaudRates[1]);
    } else if (acmIndex == 2) {
      ser5.end();
      ser5.begin(serialBaudRates[2]);
      USBSer2.end();
      USBSer2.begin(serialBaudRates[2]);
    } else if (acmIndex == 3) {
      Serial2.end();
      Serial2.begin(serialBaudRates[3]);
      USBSer3.end();
      USBSer3.begin(serialBaudRates[3]);
    } else if (acmIndex == 4) {
      ser6.end();
      ser6.begin(serialBaudRates[4]);
      USBSer4.end();
      USBSer4.begin(serialBaudRates[4]);
    } else if (acmIndex == 5) {
      Serial1.end();
      Serial1.begin(serialBaudRates[5]);
      USBSer5.end();
      USBSer5.begin(serialBaudRates[5]);
    }

    // Save the updated baud rate to EEPROM
    saveConfigToEEPROM();

    USBSerCmd.printf("Baud rate for ACM%d set to %d.\n\r", acmIndex, baud);
  } else {
    USBSerCmd.println("Invalid command. Use: set_baud X baudrate");
  }
}

void setSerialPins(String cmd) {
  int acmIndex, txPin, rxPin;
  if (sscanf(cmd.c_str(), "set_pin %d %d %d", &acmIndex, &txPin, &rxPin) == 3 && acmIndex >= 0 && acmIndex <= 5) {
    int occupiedBy = -1;
    if (isPinOccupied(txPin, rxPin, occupiedBy)) {
      USBSerCmd.printf("Error: Pins already occupied by ACM%d.\n\r", occupiedBy);
      return;
    }

    serialTXPins[acmIndex] = txPin;
    serialRXPins[acmIndex] = rxPin;
    
    if (acmIndex == 0) {
      ser3.end();
      ser3 = SerialPIO(txPin, rxPin);
      ser3.begin(serialBaudRates[0]);
    } else if (acmIndex == 1) {
      ser4.end();
      ser4 = SerialPIO(txPin, rxPin);
      ser4.begin(serialBaudRates[1]);
    } else if (acmIndex == 2) {
      ser5.end();
      ser5 = SerialPIO(txPin, rxPin);
      ser5.begin(serialBaudRates[2]);
    } else if (acmIndex == 3) {
      Serial2.end();
      Serial2.setTX(txPin);
      Serial2.setRX(rxPin);
      Serial2.begin(serialBaudRates[3]);
    } else if (acmIndex == 4) {
      ser6.end();
      ser6 = SerialPIO(txPin, rxPin);
      ser6.begin(serialBaudRates[4]);
    } else if (acmIndex == 5) {
      Serial1.end();
      Serial1.setTX(txPin);
      Serial1.setRX(rxPin);
      Serial1.begin(serialBaudRates[5]);
    }

    saveConfigToEEPROM();
    USBSerCmd.printf("Pins for ACM%d set to TX:%d RX:%d\n\r", acmIndex, txPin, rxPin);
  } else {
    USBSerCmd.println("Invalid command. Use: set_pin X TX RX");
  }
}

bool isPinOccupied(int txPin, int rxPin, int& occupiedBy) {
  for (int i = 0; i < 6; i++) {
    if ((serialTXPins[i] == txPin || serialRXPins[i] == rxPin) && occupiedBy == -1) {
      occupiedBy = i;
      return true;
    }
  }
  return false;
}

bool loadConfigFromEEPROM() {
  if (EEPROM.read(0) != EEPROM_VALID_FLAG) {
    return false; // EEPROM is invalid
  }

  int address = 1; // Start reading after the valid flag

  for (int i = 0; i < 6; i++) {
    EEPROM.get(address, serialBaudRates[i]);
    // Validate baud rate (should be within realistic ranges like 300 to 115200)
    if (serialBaudRates[i] < 300 || serialBaudRates[i] > 115200) {
      return false; // Invalid baud rate
    }
    address += sizeof(int);

    EEPROM.get(address, serialTXPins[i]);
    // Validate pin numbers (ensure they are within GPIO pin range)
    if (serialTXPins[i] < 0 || serialTXPins[i] > 28) {
      return false; // Invalid TX pin
    }
    address += sizeof(int);

    EEPROM.get(address, serialRXPins[i]);
    // Validate pin numbers (ensure they are within GPIO pin range)
    if (serialRXPins[i] < 0 || serialRXPins[i] > 28) {
      return false; // Invalid RX pin
    }
    address += sizeof(int);
  }
  return true; // Data is valid
}

void saveConfigToEEPROM() {
  EEPROM.write(0, EEPROM_VALID_FLAG); // Set valid flag

  int address = 1; // Start writing after the valid flag

  for (int i = 0; i < 6; i++) {
    EEPROM.put(address, serialBaudRates[i]);    // Write baud rate
    address += sizeof(int);
    EEPROM.put(address, serialTXPins[i]);       // Write TX pin
    address += sizeof(int);
    EEPROM.put(address, serialRXPins[i]);       // Write RX pin
    address += sizeof(int);
  }
  EEPROM.commit(); // Save to EEPROM
}

void initDefaultConfig() {
  resetBaudRates();
  resetPins();
}

void printConfig() {
  USBSerCmd.println("Current Configuration:");
  for (int i = 0; i < 6; i++) {
    int baud = serialBaudRates[i];
    if (baud == 0) baud = BAUDRATE;
    USBSerCmd.printf("ACM%d: Baud = %d, TX = %d, RX = %d\n\r", i, baud, serialTXPins[i], serialRXPins[i]);
  }
}
