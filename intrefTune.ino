// -*- c++ -*-
// Sketch for tuning the INTREF value and programming it into EEPROM
// 1) Burn the fuses for EEPROM preservation 
// 2) Connect the Vcc and GND to a Voltage meter
// 3) Connect MISO and (ICSP pin 1) and SCK (ICSP pin 3) with buttons switching to GND
// 4) Connect MOSI (ICSP pin  4, opposite to SCK) to RX on a FTDI adapter, 1200 baud
// 5) Press one of the buttons to change the INTREF value and compare reported voltage with meter,
//    the INTREF value changes every 0.3 seconds when the button is continously pressed
// 6) When satisfied, wait 30 seconds. The INTREF value will then be
//    stored in EEPROM (if STORE_TO_EEPROM is defined), at the end of the EEPROM (at E2END-3 and E2END-2) 
// 7) If you want to 'erase' the calibration value, then press both buttons and
//    provoke a reset.
// Retrieve the value in your final sketch from EEPROM and use the bandgap routine, as we used it in
// this sketch.

// Version 1.0.0 (20.1.2021)
// - First working version
// - covers only ATmega1280 and ATmega2560, ATmegaX4, ATmegaX8, ATtinyX4, ATtinyX5, ATtinyX61,
//   ATtinyX7, ATtiny1634
// Version 1.0.1 (21.1.2021)
// - corrected some typos
// Version 1.0.2 (26.1.2021)
// - use now new Vcc library
// Version 1.0.3 (28.1.2021)
// - added possibility to erase the calibration value
// Version 1.0.4 (03.02.2021)
// - print out EEPROM addr where value is stored
// - changed baud rate to 9600
// Version 1.0.5 (1.6.2023)
// - default baud rate is now 1200
// Version 2.0.0 (26.06.2023)
// - value is now always stored at E2END-3 and E2END-2 (in order to be compatible with new Vcc version)
// - the TX pin is now always the pin labeled MOSI


#define VERSION "2.0.0"
#define STORE_TO_EEPROM
#define BAUDRATE 1200
#define WAITTIMEMS (30UL*1000UL)
#define REPEATTIMEMS 300
#define DEFAULT_INTREF 1100


#include <EEPROM.h>
#include <Vcc.h>
#include <TXOnlySerial.h>

#define EE_ADDR (E2END-3)

#ifdef SPIE // if chip has an SPI module
#define TXPIN MOSI
#define UPPIN MISO
#else
#define TXPIN MISO // pin labeled MOSI
#define UPPIN MOSI // pin labeled MISO 
#endif
#define DWPIN SCK
TXOnlySerial mySerial(TXPIN); // note: this always the pin labeled MOSI
unsigned long lastpress = millis();
unsigned int lastintref = 0, intref;
unsigned int voltage = 0, lastvoltage = 0;

void setup()
{
#ifdef STORE_TO_EEPROM
  EEPROM.get(EE_ADDR,intref);
#else
  intref = 0xFFFF;
#endif
  mySerial.begin(BAUDRATE);
  mySerial.println(F("\r\n\nintrefTune V" VERSION "\n"));
  if (intref == 0xFFFF) intref = DEFAULT_INTREF;
  mySerial.print(F("Initial INTREF value: "));
  mySerial.println(intref);
#ifdef STORE_TO_EEPROM
  if (digitalRead(MOSI) == LOW && digitalRead(SCK) == LOW) {
    mySerial.print(F("Erasing stored INTREF value in EEPROM at 0x"));
    mySerial.println(EE_ADDR, HEX);
    EEPROM.put(EE_ADDR,0xFFFF);
    while (1);
  }
#endif
  pinMode(UPPIN, INPUT_PULLUP); // note: this is the MISO-ISP pin!
  pinMode(DWPIN, INPUT_PULLUP);
}


void loop(void)
{
  unsigned long start = millis();
  bool pressed = false;
  unsigned int voltage;
  unsigned int ee_intref;
  
#ifdef STORE_TO_EEPROM
  if (millis() - lastpress > WAITTIMEMS) {
    EEPROM.get(EE_ADDR, ee_intref);
    if (ee_intref != intref) {
      mySerial.print(F("Saving current value "));
      mySerial.print(intref);
      mySerial.println(F(" to EEPROM"));
      EEPROM.put(EE_ADDR, intref);
    }
  }
#endif
  if (digitalRead(UPPIN) == LOW) { // note: this is the MISO-ISP pin
    intref++;
    lastpress = millis();
    pressed = true;
  } else if (digitalRead(DWPIN) == LOW) {
    intref--;
    lastpress = millis();
    pressed = true;
  }
  voltage = Vcc::measure(1000, intref);
  if (intref != lastintref || abs(lastvoltage-voltage) > 5) {
    mySerial.print(F("INTREF="));
    mySerial.print(intref);
    mySerial.print(F(" Voltage="));
    mySerial.print(voltage/1000);
    mySerial.print(F("."));
    if ((voltage%1000) < 100) mySerial.print(F("0"));
    if ((voltage%1000) < 10) mySerial.print(F("0"));
    mySerial.print(voltage%1000);
    mySerial.println(F(" Volt"));
    lastintref = intref;
    lastvoltage = voltage;
  }
  while (millis() - start < REPEATTIMEMS && pressed ) delay(1);
}

