# Tuning the INTREF constant

When you want to monitor the supply voltage, you can use the internal bandgap voltage of (approximately) 1.1 Volt. However, the internal bandgap is not always exactly 1.1 Volt. 

With this sketch, you can tune the INTREF value that is used to determine the supply voltage. 
Connect a voltage meter to Vcc and GND and read the comments in the top of the sketch, which describe what to do. If you do not change the sketch, the best INTREF value will be stored in EEPROM at the end. Where the value is stored depends all on compile time constants.

The main point is  that you can use the value stored in EEPROM later in your particular application sketch. One important prerequisite for this is, however, that you burn the EESAVE fuse! Otherwise the values stored into EEPROM will always be deleted when a new sketch is uploaded to flash memory. Alternatively, you can disable storage  in EEPROM and just take the best value that is displayed as a compile time constant -- provided you have only one instance to deploy. 

Note that by now, there is a much easier way to calibrate you AVR MCU: [avrCalibrate](https://github.com/felias-fogg/avrCalibrate)
