/* 7-Segment Display Clock

    Variable Defines
    Variable Declarations
    HW Module Specification
    Conditional Var Declarations by HW Module

    Copyright 2017 Robert Peteuil

*/

#ifndef SSCLOCKINITS_H_
#define SSCLOCKINITS_H_

#define VERSION 200


// DEFINE HARDWARE MODULE USED
  // Sparkfun's Original 7-Segment Display (Pins on sides) - Sparkfun Part # COM-09230
  // #define SS_HWMOD_ORIGINAL

  // Sparkfun's Updated design (pins on top) - Sparkfun Part # COM-09764
  #define SS_HWMOD_UPDATED


// DEBUG MODE SETTING
  // #define DEBUGMODE    // Un-Flag to turn on debug mode


// PIN DEFINITIONS - set with defines as they dont change
  #define SQWPIN 2                              // Square Wave input from RTC Module
  #define BTN1PIN 3                             // Button 1
  #define BTN2PIN 4                             // Button 2
  #define SSRXPIN 5                             // Comm bus to 7-Seg Display
  #define SSTXPIN 6                             // Comm bus to 7-Seg Display
  #define SPKPIN 7                              // Speaker Pin


// 7-SEGMENT DISPLAY VARIABLES - Conditionally Set based on HW Version
  #if defined (SS_HWMOD_ORIGINAL)             // GEN1 values for 7-Segment Display
    const char SSdataColonOnly = 0b00110000;      // colon display data
    char SSdataColon = SSdataColonOnly;           //   On GEN1 - this variable remains steady
    const char SSdataBrightness = 0x18;           // brightness level
    #define SSdataMenuBrightness  "b "            // display-text for brightness
    #define SSdataMenuSound "5  "                 // display-text for sound
  #endif
  #if defined (SS_HWMOD_UPDATED)              // GEN2 values for 7-Segment Display
    const char SSdataColonOnly = 0b00010000;      // colon display data
    char SSdataColon = SSdataColonOnly;           //   On GEN2 - this variable gets toggled
    const byte SSdataBrightness = 0x28;           // brightness level
    #define SSdataMenuBrightness "br"             // display-text for brightness
    #define SSdataMenuSound "snd"                 // display-text for sound
  #endif

// 7-SEGMENT DISPLAY CONSTANTS
  const char SScmdReset = 0x76;                 // 7Seg-Disp reset command
  const char SScmdBrightness = 0x7A;            // 7Seg-Disp brightness command
  const char SSdataBlankDigit = 0x78;           // 7Seg-Disp blank digit
  const byte SScmdColon = 0x77;                 // 7Seg-Disp dot control command

// 7-SEGMENT DISPLAY VARIABLES
  char SSdisplayedHour, SSdisplayedMinute;      // Time value currently displayed
  char SSdisplayedSecond;                       // Second value when the colon last toggled
  volatile bool SSflagNeedsUpdate = false;      // Flag set by interrupt to update clock
  bool SSflagVisibility = false;                // Flag to blink hours and minutes in set menu


// BUTTON VARIABLES - used for tracking the button states
  volatile unsigned long BTNdebounce = 75;      // Debounce time for buttons
  volatile bool BTN1state = false;              // State of Button 1 (sets menu mode)
  volatile bool BTN1flag = false;               // Debounced value of button 1
  volatile bool BTN1released = false;           // Value of button 1 being "released"
  volatile unsigned long BTN1time = 0;          // Time Button 1 was toggled
  bool BTN2state = false;                       // State of Button 2
  bool BTN2flag = false;                        // Debounced value of button 2
  long BTN2time = 0;                            // Time Button 2 was toggled


// MENU VARIABLES
  volatile bool MENUflag = false;               // Menu Mode flag - set by interrupt


// RTC VARIABLES FOR DS1307
  const int RTCaddress = 0b1101000;             // I2C address of the DS1307
  const byte RTCregpointerTime = 0x00;          // Register pointer to time data
  const byte RTCregpointerData = 0x10;          // Register pointer to NVRAM data
  const byte RTCregpointerCReg = 0x07;          // Register pointer to control reg
  const byte RTCregdataSqWave = 0b00010000;     // Ctrl-Reg Bit-Mask to enable sq-wave out
  const byte RTCcheckbyte = 0x87;               // Value used to check for prior init
  const byte RTCbitCH = 0b10000000;             // Bit-Mask for the CH bit of the clock reg
  char RTCsecond, RTCminute, RTChour;           // Time data retreived from DS1307
  byte NVRAMbrightness, NVRAMsound;             // Persistent values stored in DS1307 NVRAM
  byte NVRAMcheckbyte;                          // Variable used to check for prior init

#endif  // SSCLOCKINITS_H_
