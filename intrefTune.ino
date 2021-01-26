// -*- c++ -*-
// Sketch for tuning the INTREF value and programming it into EEPROM
// 1) Burn the fuses for EEPROM preservation 
// 2) Connect the Vcc and GND to a Voltage meter
// 3) Connect MISO and (ICSP pin 1) and SCK (ICSP pin 3) with buttons switching to GND
// 4) Connect MOSI (ICSP pin  4, opposite to SCK) to RX on a FTDI adapter, 2400 baud
// 5) Press one of the buttons to change the INTREF value and compare reported voltage with meter,
//    the INTREF value changes every 0.3 seconds when the button is continously pressed
// 6) When satisfied, wait 30 seconds. The INTREF value will then be
//    stored in EEPROM (if STORE_TO_EEPROM is defined), either at the beginning or at the end,
//    depending on whether the compile time switch STORE_AT_END is defined.
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

#define VERSION "1.0.2"
#define STORE_TO_EEPROM
#define STORE_OFFSET 0 // reserved for INTREF (either first two or last two bytes!)
#define BAUDRATE 2400
#define WAITTIMEMS (30UL*1000UL)
#define REPEATTIMEMS 300
#define DEFAULT_INTREF 1100


#include <EEPROM.h>
#include <Vcc.h>
#include <TXOnlySerial.h>

#ifdef INTREF_AT_END_OF_EEPROM
#define EE_ADDR (E2END-STORE_OFFSET-1)
#else
#define EE_ADDR STORE_OFFSET
#endif


TXOnlySerial mySerial(MISO); // note: this is the MOSI-ISP pin!
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
  pinMode(MOSI, INPUT_PULLUP); // note: this is the MISO-ISP pin!
  pinMode(SCK, INPUT_PULLUP);
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
  if (digitalRead(MOSI) == LOW) { // note: this is the MISO-ISP pin
    intref++;
    lastpress = millis();
    pressed = true;
  } else if (digitalRead(SCK) == LOW) {
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

