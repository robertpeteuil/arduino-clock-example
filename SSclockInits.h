/* 7-Segment Display Clock   (azrobbo)

    Current Version
        2.16 - Added to Github 
         
        Detailed changelog at bottom (of this file)
    
    -----------------------------------------------------------------------------------------
*/

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#define VERSION 16

// Define hardware module being used - make sure only one is uncommented
    // Original 7-Segment Display (Pins on sides) - Sparkfun Part # COM-09230
    //      This device prints '0' - '9' and 'a' - 'f' only; it supports brightness settings, and dot/command control
    #define SS_HWMOD_ORIGINAL   
    
    // Updated design (pins on top) - Sparkfun Part # COM-09764
    // #define SS_HWMOD_UPDATED          
 
// DEBUG MODE SETTING
    // #define DEBUGMODE

// Pin Definitions - set with defines as they dont change
    #define SQWPIN 2                              // Square Wave input from RTC Module
    #define BTN1PIN 3                             // Button 1                          
    #define BTN2PIN 4                             // Button 2                         
    #define SSRXPIN 5                             // Comm bus to 7-Seg Display
    #define SSTXPIN 6                             // Comm bus to 7-Seg Display
    #define SPKPIN 7                              // Speaker Pin
    
// 7-Segment Display Variables that differ based on 7-Seg Module being used
    #ifdef SS_HWMOD_ORIGINAL                      // GEN1 values for 7-Segment Display
        const char SSdataColonOnly = 0b00110000;  // 7-Segment Display colon display data - this variable will always display a colon
        char SSdataColon = SSdataColonOnly;       // 7-Segment Display colon display data - On GEN1 H/W, this variable remains steady
        const char SSdataBrightness = 0x18;       // 7-Segment Display brightness level
        #define SSdataMenuBrightness  "b "        // 7-Segment Display Menu text displayed during setting brightness 
        #define SSdataMenuSound "5  "             // 7-Segment Display Menu text displayed during setting brightness 
    #elif defined SS_HWMOD_UPDATED                // GEN2 values for 7-Segment Display
        const char SSdataColonOnly = 0b00010000;  // 7-Segment Display colon display data - this variable will always display a colon
        char SSdataColon = SSdataColonOnly;       // 7-Segment Display colon display data - this variable gets toggled
        const byte SSdataBrightness = 0x28;       // 7-Segment Display brightness level
        #define SSdataMenuBrightness "br"         // 7-Segment Display Menu text displayed during setting brightness 
        #define SSdataMenuSound "snd"             // 7-Segment Display Menu text displayed during setting brightness 
    #else
        #error "No 7-Segment Display Module Defined"
    #endif

// 7-Segment Display Constants
    const char SScmdReset = 0x76;                 // 7-Segment Display reset command
    const char SScmdBrightness = 0x7A;            // 7-Segment Display brightness command
    const char SSdataBlankDigit = 0x78;           // 7-Segment Display blank digit
    const char SScmdColon = 0x77;                 // 7-Segment Display dot control command

// 7-Segment Display Variables
    char SSdisplayedHour, SSdisplayedMinute;      // holds time value currently displayed
    char SSdisplayedSecond;                       // holds the second value when the colon last toggled
    volatile bool SSflagNeedsUpdate = false;      // Flag set by interrupt to update clock
    bool SSflagVisibility = false;                // Flag to blink hours and minutes in set menu

// Button Variables - used for tracking the button states
    volatile long BTNdebounce = 75;               // Debounce time for buttons
    volatile bool BTN1state = false;              // State of Button 1 (Green button - used to set menu mode)
    volatile bool BTN1flag = false;               // Flag holds debounced value of button 1
    volatile bool BTN1released = false;           // Flag holds value of button 1 being "released"
    volatile long BTN1time = 0;                   // Time Button 1 was toggled
    bool BTN2state = false;                       // State of Button 2 (Orange Button)
    bool BTN2flag = false;                        // Flag holds debounced value of button 2
    long BTN2time = 0;                            // Time Button 2 was toggled

// Menu Variables - used for processing the menu
    volatile bool MENUflag = false;               // Flag set by interrupt to enter menu mode

// RTC Variables
    const int RTCaddress = 0b1101000;             // Put I2C address of the DS1307 into the variable we will use
    const byte RTCregpointerTime = 0x00;          // DS1307 Register pointer to address for time data
    const byte RTCregpointerData = 0x10;          // DS1307 Register pointer to address for NVRAM data
    const byte RTCregpointerCReg = 0x07;          // DS1307 Register pointer to address for the control register
    const byte RTCregdataSqWave = 0b00010000;     // DS1307 Control Register data to enable square wave output
    const byte RTCcheckbyte = 0x87;               // value stored in NVRAM, used to check for previous initialization
    const byte RTCbitCH = 0b10000000;             // mask for the CH bit of the clock register
    char RTCsecond, RTCminute, RTChour;           // holds time data retreived from DS1307
    byte NVRAMbrightness, NVRAMsound;             // persistent variables stored in DS1307 NVRAM
    byte NVRAMcheckbyte;                          // variable with known value used to check if NVRAM has been initialized
    
/*      Detailed changelog

    1.0-1.9 - basic RTC integration and I2C comm configuration
        2.0 - added S-Seg update
        2.1 - added blinking colon on S-Seg
        2.2 - removed lots of debug code
        2.3 - variable & function rename & cleanup
        2.4 - adjusted hours to always be 1-12, changed leading character for hours to space, added Sq Wave Output config
        2.5 - added interrupt routine to trigger 7-seg display update (DS1307 Sq Wave output drives interrupt)
        2.6 - removed all date functions & variables
        2.7 - added sound, fixed colon display problem on reset, added code to support GEN1 7-Seg module
        2.8 - added menu interrupt function and debounce, adjusted readTimeUART and RTCsetTime
        2.9 - started menu function, added optional params to SSprint functs
        2.10 - moved declares to SSclock.h, switched UltraEdit Studio for IDE (3/9/2012 10:54:48 PM)
        2.11 - experimental release with seperate functionalized files, approach abandoned 
        2.12 - Menu for setting hours/minutes; sound function created; 2nd param in SSprintTime; moved 12 hour adjustment from ReadTime to Print function
        2.13 - stored brightness level & sound settings in RTC NVM; started menu items for brightness & sounds
        2.14 - Menu finished, LED code removed, adjusted button debounce (faster press on buttons 2 & 3, 1 does not have 'repeat'), added sound code (0=no sound, 1=sound in menu only, 2=click on the hour, 3=hourly chimes)
        2.15 - Changed Pin assignments, Added scaled brightness, Removed Button 3, Added Clock Enable to Setup, fixed brightness menu, added menu string for module versions
        2.16 - Added to Github 
*/
