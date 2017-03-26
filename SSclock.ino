/* 7-Segment Display Clock

   fetches live data from DS1307 Real Time Clock
   updates from RTC every second via pulse driven interrupts
   time on the DS1307 can be set via UART in the format: T(sec)(min)(hour) or TSSMMHH
   hours are enetered 1-24, but the display only shows 1-12
   the code can tell when it's AM or PM, but this is not visible from the display since this functionality wasn't needed

   Copyright 2017 Robert Peteuil

 */


#include "Arduino.h"
#include "SSclockInits.h"                               // var declares and defines
#include <Wire.h>                                       // I2C used with DS1307 RTC
#include <SoftwareSerial.h>                              // Software Serial Library
SoftwareSerial SSserial(SSRXPIN, SSTXPIN);               //   Connect to 7-Segment Display



// interrupt handler - driven by external square wave at 1Hz
void interruptUpdatePulse() {
  SSflagNeedsUpdate = true;                             // Set 7Seg-Disp Update Flag
}

// interrupt handler - Button driven interrupt
void interruptButtonPress() {
  if (MENUflag) return;                                 // if menu flag set then return
  if (BTN1state) {                                      // Button #1 flag set
    if ((millis() - BTN1time) > BTNdebounce) {          // Pressed > debounce time?
        MENUflag = true;                                // Yes - set Menu flag
        BTN1state = false;                              // Clear button #1 flag
    }
  } else {                                              // button #1 flag not set
    BTN1state = true;                                   // Set button #1 flag
    BTN1time = millis();                                // Start button #1 debounce timer
    }
}



void setup() {
  PINset();                                             // Initialize I/O Pins
  SPKeffect();                                          // Click the speaker

  // Attach interrupt 0 = digital pin 2
  attachInterrupt(0, interruptUpdatePulse, FALLING);    // assign funct on falling edge

  // Attach interrupt 1 = digital pin 3
  attachInterrupt(1, interruptButtonPress, CHANGE);     // assign funct on ANY edge

  // Initialize Communication buses
  Wire.begin();                                         // RTC on I2C bus
  Serial.begin(19200);                                  // Debug on Serial
  SSserial.begin(9600);                                 // 7Seg-Disp on Soft Serial

  // Control RTC Module (DS1307)
  RTCconfigSqWave();                                    // Turn on Square Wave Output

  RTCreadData();                                        // Read vars from RTC NVRAM

  // If the checkbyte read does not equal our preset value, the Vars need to be set
  if (NVRAMcheckbyte != RTCcheckbyte) {
    NVRAMbrightness = SSdataBrightness;                 // Set default Brightness
    NVRAMsound = 2;                                     // Set default Sound
    RTCsetData();                                       // Write these to the NVRAM
  }

  // Startup Animations - for bling value only
  SSanimate();                                          // count up from 8888 to 0000
  SPKeffect(2);                                         // play 3-note sound effect

  // put clock data on 7-Segment Display
  RTCreadTime();                                        // read time & set time vars
  RTCconfigClkEnable();                                 // set clock enable bit
  SSprintTime();                                        // output time to 7Seg-Disp
}



void loop() {
  if (Serial.available()) {                             // serial data?
    UARTprocessInput();                                 // Yes - process it
  }
  if (SSflagNeedsUpdate) {                              // display update flag set?
    RTCreadTime();                                      // Yes - update time vars
    SSprintTime();                                      // output the time to the 7Seg-Disp
    SSflagNeedsUpdate = false;                          // clear update flag
  }
  if (MENUflag) {                                       // Is the manu flag set?
    MENUset();                                          // yes - process menu
  }
}



// menu to allow for setting of time, brightness, and sounds
void MENUset() {
  // Prepare the environment
  // Turn off interrupt on the menu button, since we're already in the menu
    detachInterrupt(1);
    SSprintTime(true, true);                            // Print the time in 24 hour format
    SPKeffect();                                        // Click speaker to acknowledge entering menu mode

  // Initialize local varibles used in this function
    char NEWminute, NEWhour;                            // Temporarily holds new time settings
      NEWminute = RTCminute;                            // Set new hours/minute vars to current value
      NEWhour = RTChour;
    byte NewBrightness, ScaledBrightness, NewSound;     // Temporarily holds new brightness & sound settings
      NewBrightness = NVRAMbrightness;                  // Set new brightness variable to existing value
      NewSound = NVRAMsound;                            // Set new sound variable to existing value

  // MODE = Hours
    while (1) {                                         // Hours Mode Loop - loop until a break is encountered
      if (SSflagNeedsUpdate) {                          // Blink the hours digits on/off so user knows what's being set
        SSprintColon(true);                             // Print Colon Symbol
        if (SSflagVisibility) {                         // It's VISIBLE
          SSprintTime(true, true);                      // Call function to display time in 24 hour mode
          SSflagVisibility = false;                     // Clear the visibility flag
        } else {                                        // HOURS digits are NOT visible
          SSserial.print(SSdataBlankDigit);             // Print 2 blank digits where hours would be
          SSserial.print(SSdataBlankDigit);
          if (RTCminute < 10) SSserial.print("0");      // If the minute is between 1 and 9 then print a 0 first
          SSserial.print(RTCminute, DEC);
          SSflagVisibility = true;                      // Toggle the visibility flag
        }
        SSflagNeedsUpdate = false;                      // Clear the update flag
      }
      BTNpollAll();                                     // Poll the buttons
      if (BTN1flag) {                                   // Button 1 was pressed
        BTN1flag = false;                               // Clear the button 1 flag
        SPKeffect();                                    // Click the speaker
        break;                                          // Since button #1 was pressed, break out of the Hours Mode loop
      }
      if (BTN2flag) {                                   // Button #2 was pressed
        BTN2flag = false;                               // Clear the button flag
        SPKeffect();                                    // Click the speaker
        NEWhour++;                                      // Increment the hour
        if (NEWhour > 23) NEWhour = 0;                    // If the hours is set to 24, then rollover to zero
        RTChour = NEWhour;
        SSprintTime(true, true);                        // Print new time & force 24h format
      }
    }
    SSflagVisibility = false;                           // Clear the visibility flag

  // MODE = Minutes
    while (1) {                                         // Minute Mode Loop - loop until a break is encountered
      if (SSflagNeedsUpdate) {                          // Blink the minutes digit on/off so user knows what's being set
        if (SSflagVisibility) {                         // it's VISIBLE
          SSprintTime(true, true);                      // Call function to display time in 24 hour mode
          SSflagVisibility = false;                     // Clear the visibility flag
        } else {                                        // MINUTES digits are NOT visible
          SSserial.print(SScmdReset);                   // Resets the 7-segment display
          SSprintColon(true);
          // If the hour is between 1 and 9 then print a space first
          if (RTChour < 10) SSserial.print(SSdataBlankDigit);
          SSserial.print(RTChour, DEC);
          SSserial.print(SSdataBlankDigit);             // Print 2 blank digits where the minutes digits would be
          SSserial.print(SSdataBlankDigit);
          SSflagVisibility = true;                      // Toggle the visibility flag
        }
        SSflagNeedsUpdate = false;                      // Clear the update flag
      }
      BTNpollAll();                                     // Poll the buttons
      if (BTN1flag) {
        BTN1flag = false;                               // Clear the button flag
        SPKeffect();                                    // Click the speaker
        break;                                          // Button #1 was pressed, break out of the Minutes Mode loop
      }
      if (BTN2flag) {                                   // Button #2 was pressed
        BTN2flag = false;                               // Clear the button flag
        SPKeffect();                                    // Click the speaker
        NEWminute++;                                    // Increment the minutes
        if (NEWminute > 59) NEWminute = 0;              // If min = 60, rollover to 0
        RTCminute = NEWminute;
        SSprintTime(true, true);                        // Print the new time value
      }
    }

  // Wrap-up 'minutes adjustment mode' - write the new time values to the RTC, and play 'acknowledged' sound effect
    RTCsecond = 0;                                      // Clear the second variable
    RTCsetTime();                                       // Call function to write the new time to the RTC
    SPKeffect();                                        // Play "acknowledged" sound

  // MODE = brightness
    SSserial.print(SScmdColon);
    SSserial.print(0x00);                               // Turn the colon and all dots off
    SSserial.print(SScmdReset);                         // Reset the Display
    SSserial.print(SSdataMenuBrightness);               // Print "br" to identify brightness setting
    if (NewBrightness < 10) SSserial.print("0");        // If NVRAMbrightness is < 10 then print a 0 first
    SSserial.print(NewBrightness, DEC);                 // Print the brightness value (in Hex so it fits in 2 digits)
    while (1) {
      BTNpollAll();                                     // Poll the buttons
      if (BTN1flag) {
        BTN1flag = false;                               // Clear the button flag
        SPKeffect();                                    // Click the speaker
        break;                                          // Button #1 was pressed, break out of the brightness Mode loop
      }
      if (BTN2flag) {                                   // Button #2 was pressed
        BTN2flag = false;                               // Clear the button flag
        SPKeffect();                                    // Click the speaker
        NewBrightness++;                                // Increment the brightness
        // If the brightness is more than 2 digits rollover to zero
        if (NewBrightness > 99) NewBrightness = 1;
        ScaledBrightness = map(NewBrightness, 1, 99, 1, 254);   // remap the brightness value of 1-99 to 1-254
        SSsetBrightness(ScaledBrightness);              // Set the brightness using the new value
        SSserial.print(SScmdReset);                     // Reset the Display
        SSserial.print(SSdataMenuBrightness);           // Print "br" to identify brightness setting
        if (NewBrightness < 10) SSserial.print("0");    // If NVRAMbrightness is < 10 then print a 0 first
        SSserial.print(NewBrightness, DEC);             // Print the brightness value (its 0-99 so it fits in 2 digits)
      }
    }
    NVRAMbrightness = NewBrightness;                    // Set the brightness variable with its new permanent value

  // MODE = sounds
    SSserial.print(SScmdReset);
    SSserial.print(SSdataMenuSound);                    // Print "Snd" to identify Sound Setting
    SSserial.print(NewSound, DEC);                      // Print the Sound Configuration value
    while (1) {
      BTNpollAll();                                     // Poll the buttons
      if (BTN1flag) {
        BTN1flag = false;                               // Clear the button flag
        SPKeffect();                                    // Click the speaker
        break;                                          // Button #1 was pressed, break out of the sound Mode loop
      }
      if (BTN2flag) {                                   // Button #2 was pressed
        BTN2flag = false;                               // Clear the button flag
        SPKeffect();                                    // Click the speaker
        NewSound++;                                     // Increment the sound setting value
        if (NewSound > 3) NewSound = 0;                 // If sound > 3, rollover to 0
        SSserial.print(SScmdReset);
        SSserial.print(SSdataMenuSound);                // Print "Snd" to identify Sound Setting
        SSserial.print(NewSound, DEC);                  // Print the Sound Configuration value
      }
    }
    NVRAMsound = NewSound;                              // Set the sound vsariable to its new permanent value

  // Finalize function, clear flags and exit
    RTCsetData();                                       // Write the new brightness & sound values to NVRAM
    SSprintTime(true, false);                           // Print the time in 12 hour format (normal format)
    SPKeffect(1);                                       // Play "acknowledged" sound
    MENUflag = false;                                   // Clear the Menu Flag
    delay(500);                                         // Pause for 1/2 a second

  // Re-attach interrupt for button press to get to menu
    attachInterrupt(1, interruptButtonPress, CHANGE);
}



// Polls all the buttons, sets state variables, sets debounce timers, and set output flags
void BTNpollAll() {
  if (digitalRead(BTN1PIN) == LOW) {                    // Button 1 is pressed
    if (BTN1state) {                                    // Was button #1 state set?
      if ((millis() - BTN1time) > BTNdebounce) {        // If so, has it been down longer than the debounce time?
        if (BTN1released) {                             // Has the button been released sine the last flagging?
          BTN1flag = true;
          BTN1state = false;                            // Clear button #1 state for next time
          BTN1released = false;                         // Clear the "released' flag
        }
      }
    } else {                                            // Button #2 state was not set
      BTN1state = true;                                 // Set Button #2 state flag
      BTN1time = millis();                              // Set Button #2 debounce timer
      }
  } else {
    BTN1state = false;                                  // If BTN1PIN is HIGH (button unpressed) clear the flag
    // Set a flag that the button was released, this prevents holding the button down
    BTN1released = true;
  }

  if (digitalRead(BTN2PIN) == LOW) {                    // Button 2 is pressed
    if (BTN2state) {                                    // Was button #2 state set?
      if ((millis() - BTN2time) > BTNdebounce) {        // If so, has it been down longer than the debounce time?
        BTN2flag = true;
        BTN2state = false;                              // Clear button #2 state for next time
      }
    } else {                                            // Button #2 state was not set
      BTN2state = true;                                 // Set Button #2 state flag
      BTN2time = millis();                              // Set Button #2 debounce timer
      }
  } else {
    BTN2state = false;                             // If button unpressed clear flag
  }
}


// Call the main function, but passes along the optional parameter
void SSprintTime() {
  SSprintTime(false, false);
}

// Call the main function, but passes along the second optional parameter
void SSprintTime(boolean AlwaysPrint) {
  SSprintTime(AlwaysPrint, false);
}

//  Prints the time to the 7-Segment Display
//  Only print if the display time has changed unless 'true' paramter that forces print
//  TFHtime stands for 24-hour time, as 24 hour formatting is used when setting the clock
void SSprintTime(boolean AlwaysPrint, boolean TFHtime) {
  // This holds the hour value that is actually displayed
  //   allows to adjust for 12 hour display, and midnight
  char DISPLAYEDhour;

  // Run if display time is NOT current
  if (((SSdisplayedHour != RTChour) || (SSdisplayedMinute != RTCminute)) || (AlwaysPrint)) {
    DISPLAYEDhour = RTChour;                            // Set value of the hour value we will display
    if (!TFHtime) {                                     // If NOT in 24-Hour Time then adjust to 12 hour time
      if (DISPLAYEDhour > 12) DISPLAYEDhour -= 12;      // Adjust for 12 hour time
      if (DISPLAYEDhour == 0) DISPLAYEDhour = 12;       // If it's zero o'clock, display 12
    }

    SSserial.print(SScmdReset);                         // Resets the 7-segment display
    SSprintColon();                                     // Toggle the colon on the 7-Segment Display
    // If the hour is between 1 and 9 then print a space first
    if (DISPLAYEDhour < 10) SSserial.print(SSdataBlankDigit);
    SSserial.print(DISPLAYEDhour, DEC);
    if (RTCminute < 10) SSserial.print("0");            // If the minute is between 1 and 9 then print a 0 first
    SSserial.print(RTCminute, DEC);

    SSdisplayedHour = RTChour;                          // Set the hour value displayed
    SSdisplayedMinute = RTCminute;                      // Set the minute value displayed
    if (!AlwaysPrint) {                                 // Don't play sounds if it is a forced print
      if (RTCminute == 0) {                             // If its the "top of the hour"
        if (NVRAMsound == 2) SPKeffect();               // And the sound mode = 2, then tick the speaker
        //  soundmode = 3, then play the chime once for each hour
        else if (NVRAMsound == 3) SPKeffect(3, DISPLAYEDhour);
      }
    }
  } else {                                              //  Toggle the colon every time
    // Only toggle the colon on GEN2 displays, otherwise it will clear it
    #if defined (SS_HWMOD_UPDATED)
      SSprintColon();                                   // Toggle the colon on the 7-Segment Display
    #endif
  }
}

// Call the main function, but passes along the second optional parameter
void SSsetBrightness() {
  SSsetBrightness(SSdataBrightness);                    // If no parameter passed, use pre-defined constant
}

// Send brightness command to 7-segment display
void SSsetBrightness(byte brightness) {
  SSserial.print(SScmdBrightness);
  SSserial.print(brightness);                           // Send brightness level
}

// Call the main function, but passes along the optional parameter
void SSprintColon() {
  SSprintColon(false);
}

// Print colon on the 7-Segment Display (toggle blinking on GEN2 boards)
void SSprintColon(boolean AlwaysPrint) {
  if (SSdisplayedSecond != RTCsecond) {
    #if defined (SS_HWMOD_UPDATED)
        SSdataColon = SSdataColon ^ SSdataColonOnly;    // GEN2 module - toggle bit 4 in the Dot Control data
    #endif
    SSserial.print(SScmdColon);                         // Send Dot Control command to 7-segment display
    SSserial.print(SSdataColon);                        // Send Dot Control data byte to 7-segment display
    SSdisplayedSecond = RTCsecond;                      // Set the second value when the colon was displayed
  }
  // If always print command was used, print colon but dont toggle it
  if (AlwaysPrint) {
    SSserial.print(SScmdColon);                         // Send Dot Control command to 7-segment display
    SSserial.print(SSdataColonOnly);                    // Send Dot Control data byte to 7-segment display
  }
}


// Read Debug Serial and execute appropriate command
// Command Summary:   T = Set Time, R = Read Time, B = Set 7-Segment Display Brightness
//                    S = Set Sq. Wave output, P = Set Hardcoded Time Values (7 PM)
void UARTprocessInput() {
  int command = Serial.read();

  // Set Time = S for "Set"
  if (command == 83 || command == 115) {
    UARTgetTime();
    RTCsetTime();
  } else if (command == 82 || command == 114) {     // Read Time = R for "Read"
    RTCreadTime();
    UARTprintTime();
  } else if (command == 66 || command == 98) {      // Set Brightness = B for "Brightness"
    NVRAMbrightness = SSdataBrightness;
    SSsetBrightness();
  } else if (command == 87 || command == 119) {     // Turn on Square Wave = W for "Wave"
    RTCconfigSqWave();
  } else if (command == 72 || command == 104) {     // hardcoded values = H for "Hard-coded"
    RTCsetHardcodedTime();
  } else if (command == 69 || command == 101) {     // List env vars - E for "Environment"
    UARTprintVars();
  }
}


//  Allows for setting the time on the 1307 via a UART String
//      converts the chars into time variables
//    Bit 7 of register 0 is the clock halt (CH) bit. (1 = oscillator disabled)
//    Bit 6 of the hours register is defined as the 12- or 24-hour mode select bit. (1 = 12-hour mode).
//          In the 12-hour mode, bit 5 is the AM/PM bit with logic high being PM.
void UARTgetTime() {
  delay(50);              // Needed so that serial port can catch up (data loss without this)
  RTCsecond = (byte) ((Serial.read() - 48) * 10 + (Serial.read() - 48));
  RTCminute = (byte) ((Serial.read() - 48) *10 +  (Serial.read() - 48));
  RTChour  = (byte) ((Serial.read() - 48) *10 +  (Serial.read() - 48));
}

//  Prints a formatted view of the time & date to the debug serial output
void UARTprintTime() {
  if (RTChour < 10) Serial.print(" ");
  Serial.print(RTChour, DEC);
  Serial.print(":");
  if (RTCminute < 10) Serial.print("0");
  Serial.println(RTCminute, DEC);
}

//  Prints all environment variables to the debug serial output
void UARTprintVars() {
  Serial.print("NVRAMbrightness = ");
  Serial.println(NVRAMbrightness, DEC);
  Serial.print("NVRAMsound = ");
  Serial.print(NVRAMsound, DEC);
  switch (NVRAMsound) {
    case 0:
      Serial.println(" (Sound is disabled)");
      break;
    case 1:
      Serial.println(" (Sound in menus only)");
      break;
    case 2:
      Serial.println(" (Sound set to click on the hour)");
      break;
    case 3:
      Serial.println(" (Sound set for hourly chimes)");
      break;
  }
  #if defined (DEBUGMODE)
    Serial.println("DEBUGMODE is on!");
  #endif
  #if defined (SS_HWMOD_ORIGINAL)
    Serial.println("Original 7-Segment Display Module Defined");
  #endif
  #if defined (SS_HWMOD_UPDATED)
    Serial.println("Updated 7-Segment Display Module Defined");
  #endif
}


// Gets the time from the ds1307
void RTCreadTime() {
  Wire.beginTransmission(RTCaddress);
    // Set the register pointer at 00 (address of the time variables)
    Wire.write(RTCregpointerTime);
  Wire.endTransmission();

  Wire.requestFrom(RTCaddress, 3);                     // Request 3 bytes from DS1307
    RTCsecond = convBCDtoDEC(Wire.read() & 0x7f);
    RTCminute = convBCDtoDEC(Wire.read());
    RTChour = convBCDtoDEC(Wire.read() & 0x3f);
}

// Read variables from RTC NVRAM: SS Brightness, Sound Effect Settings, and Checkbyte
void RTCreadData() {
  Wire.beginTransmission(RTCaddress);
    // Set the register pointer to free storage space (past time/date variables)
    Wire.write(RTCregpointerData);
  Wire.endTransmission();

  Wire.requestFrom(RTCaddress, 3);                     // Request 3 bytes from DS1307
    NVRAMbrightness = convBCDtoDEC(Wire.read());       // 1st = Brightness
    NVRAMsound = convBCDtoDEC(Wire.read());            // 2nd = sound setting
    NVRAMcheckbyte = convBCDtoDEC(Wire.read());        // 3rd = checkbyte
}

// Store variables into RTC NVRAM: SS Brightness, Sound Effect Settings, and Checkbyte
void RTCsetData() {
  Wire.beginTransmission(RTCaddress);
  // Set the register pointer to free storage space (past time/date variables)
  Wire.write(RTCregpointerData);
  Wire.write(convDECtoBCD(NVRAMbrightness));           // Write brightness
  Wire.write(convDECtoBCD(NVRAMsound));                // Write sound setting
  Wire.write(convDECtoBCD(RTCcheckbyte));              // Write checkbyte
  Wire.endTransmission();
}

// Check data for validity, then send to DS1307 via I2C
void RTCsetTime() {
  if ((RTCsecond >= 0 && RTCsecond < 60) && (RTCminute >= 0 && RTCminute < 60) && (RTChour > 0 && RTChour <= 24)) {
    Wire.beginTransmission(RTCaddress);
    // Set register pointer to the beginning of the time data
    Wire.write(RTCregpointerTime);
    Wire.write(convDECtoBCD(RTCsecond));
    Wire.write(convDECtoBCD(RTCminute));
    Wire.write(convDECtoBCD(RTChour));
    Wire.endTransmission();
  }
}

//  CONTROL REGISTER = DS1307 address 0x07
//      When enabled (SQWE bit = 1) the SQW/OUT pin outputs one of four square wave frequencies (1Hz, 4kHz, 8kHz, 32kHz)
//      When disabled, the OUT bit (b7) can be used to set the SQW/OUT pin high or low
//      b7 = OUT (Output Control of SQW pin when sq wave disabled), b6 = 0, b5 = 0,
//      b4 = SQWE (1 = Square Wave Enable), b3 = 0, b2 = 0, b1 = RS1, b2 = RS0
//          RS1/0/Freq = 0/0/1Hz, 0/1/4.096kHz, 1/0/8.192kHz, 1/1/32.768kHz
//      CR value of 0b00010000 (0x10) = turns on sq. wave with 1Hz output
void RTCconfigSqWave() {
  Wire.beginTransmission(RTCaddress);
  // Set register pointer to the Control Register
  Wire.write(RTCregpointerCReg);
  // Send Bit-Mask to enable sq-wave out
  Wire.write(RTCregdataSqWave);
  Wire.endTransmission();
}

// Enable the oscillator by clearing the CH Bit of memory address 0
void RTCconfigClkEnable() {
  byte NewSecondValue;
  NewSecondValue = RTCsecond & (!RTCbitCH);            // Clear out CH bit (bit 7)
  Wire.beginTransmission(RTCaddress);
  // Set register pointer to the Control Register
  Wire.write(RTCregpointerTime);
  // Send clear out CH bit
  Wire.write(convDECtoBCD(NewSecondValue));
  Wire.endTransmission();
}

void RTCsetHardcodedTime() {
  Wire.beginTransmission(RTCaddress);
  Wire.write(RTCregpointerTime);                       // Set pointer to location of 1st write
  // Send Seconds (Also, bit 7 of this memory controls the clock state.  0 starts the clock)
  Wire.write(convDECtoBCD(00));
  Wire.write(convDECtoBCD(00));                        // Send Minutes
  // Send Hours (Also, Bit 6 controls AM/PM setting - also need to change readDateDs1307)
  Wire.write(convDECtoBCD(19));
  Wire.endTransmission();
}



// Plays sound effects - if called without any paramters, pass the param to play a click once
void SPKeffect() {
  SPKeffect(0, 1);
}

// Plays sound effects - if called without the replay parameter, pass the paramter as 1
void SPKeffect(char x)  {
  SPKeffect(x, 1);
}

// Plays sound effects - based on param passed to it & repeats 'y' times
// no param/0 = "click", 1 = "acknowledged" tone, 2 = "bootup sound"
void SPKeffect(char x, char y) {
  // If sound setting is set to 0 (no sounds) then return
  if (NVRAMsound == 0) return;
  // If sound setting is set to 1 (menu sound only) and not in a menu then return
  if ((NVRAMsound == 1) && (!MENUflag)) return;

  for (int i = 1; i <= y; i++) {
    if (x == 0) {                                      // "Click" sounds
      tone(SPKPIN, 784);
      delay(15);
    }
    if (x == 1) {                                      // "Acknowledged" Sound
      tone(SPKPIN, 523);
      delay(100);
      noTone(SPKPIN);
      delay(50);
      tone(SPKPIN, 784);
      delay(50);
    }
    if (x == 2) {                                      // Play quick tone at setup so initialization can be heard
      tone(SPKPIN, 523);
      delay(150);
      noTone(SPKPIN);
      tone(SPKPIN, 784);
      delay(75);
      noTone(SPKPIN);
      delay(50);
      tone(SPKPIN, 784);
      delay(75);
    }
    if (x == 3) {                                      // "Chimes" that repeats 'y' times
      tone(SPKPIN, 523);
      delay(100);
      noTone(SPKPIN);                                  // Pause between chimes
      delay(500);
    }
  }
  noTone(SPKPIN);
}



// Initalize the 7-segment display & put on a little show
void SSanimate() {
  char ssPulseDelay = 14;                              // 7-Segment Display Delay
  SSserial.print(SScmdReset);                          // Resets the 7-segment display
  SSserial.print(SScmdColon);
  SSserial.print(0x00);                                // Turn all dots & colons off

  for (int j = 3; j >= 0; j--) {                       // Cycle through all 4 digits, right to left
    for (int i = 9; i >= 0; i--) {                     // Counter containing the digit to display
      SSserial.print(SScmdReset);                      // Clear the display
      if (j > 0) {                                     // Dont print leading spaces on left-most digit
        for (int k = 1; k <= j; k++) {                 // Loop for the number of spaces needed
          SSserial.print(" ");                         // Print one space
        }
      }
      SSserial.print(i);                               // Print counter
      if (j < 3) {                                     // Only print trailing 0's on right 3 digits
        for (int k = 1; k <= (3-j); k++) {             // Print (3-j) trailing 0's
          SSserial.print("0");
        }
      }
      delay(ssPulseDelay);
    }
  }
}


// Convert normal decimal numbers to binary coded decimal
byte convDECtoBCD(byte val) {
  return ( (val/10*16) + (val%10) );
}

// Convert binary coded decimal to normal decimal numbers
byte convBCDtoDEC(byte val) {
  return ( (val/16*10) + (val%16) );
}


// Setup all Pins based on variables and declarations
void PINset() {
  // Setup I2C Bus pins - for RTC comms
  pinMode(A4, INPUT); digitalWrite(A4, HIGH);          // Sets as input & turn on pullup resistor
  pinMode(A4, OUTPUT);                                 // Now set as output
  pinMode(A5, INPUT); digitalWrite(A5, HIGH);          // Sets as input & turn on pullup resistor
  pinMode(A5, OUTPUT);                                 // Now set as output
  // Setup Softserial pins - for debug UART comms
  pinMode(SSTXPIN, OUTPUT);                            // Set 5 as output
  pinMode(SSRXPIN, INPUT);                             // Not used - the 7-segment display does not send any data back
  // Setup input for Square Wave signal from RTC Module
  //      This must be on an interrupt capable I/O line as it will be used to trigger updates
  pinMode(SQWPIN, INPUT); digitalWrite(SQWPIN, HIGH);
  // Set Speaker pin as output
  pinMode(SPKPIN, OUTPUT);
  // Set Button pins -
  pinMode(BTN1PIN, INPUT); digitalWrite(BTN1PIN, HIGH);
  pinMode(BTN2PIN, INPUT); digitalWrite(BTN2PIN, HIGH);
}
