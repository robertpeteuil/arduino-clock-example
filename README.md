# Simple Arduino 7-Segment Display Project using DS1307 & Sparkfun 7-Segment Display

This started as a simple project to display the time from a DS1307, that expanded to include  additional functions and HW devices.  Its most useful as a function reference for interacting with different types of hardware devices via different interface methods.

Basic Clock Features:
- Set / Read current time from DS1307 module
- Displays time on 7-Segment Display
- Interrupt-driven realtime display of time
- Outputs sound via small speaker
- Use Hardware buttons to allow user settings changes  

The code is broken into discrete functions, which are named by device (7-Segment-Display, RTC, UART, Menu).  These functions are extensively commented to make it easy to re-purpose.  Included functions perform the following:
- Accessing a module via I2C
- Using square wave pulses to HW trigger interrupts (then used to update display)
- Using buttons to trigger HW interrupts (to trigger a manu)
- Debouncing HW buttons
- Parsing a serial stream to get user data
- Storing data in non-volatile memory via I2C
- Settings Menu-Function to allow for change of settings by user with minimal hardware buttons

**2016 Compatibility Update:**
- Updated version 2.0.0 works on current Arduino IDE as of Jan 2016
- Main program now titled 'SSclock.ino'
- Old Version (1.0.0) was written in 2012 on Arduino build 23
- The updates to allow use of current Arduino IDE included changing I2C commands, some variable types, and some serial commands 

### FILES INCLUDED:

**Source Code:**
- 'SSclockInits.h' - program header, variable inits, defines and conditional compilation settings
- 'SSclock.ino' - main program code & all functions

**Datasheets:** - in the "datasheets" sub-folder
- 'DS1307.pdf' - information on DS1307 real-time clock
- 'SFE-0012-DS-7segmentSerial-v3.pdf' - ORIGINAL 7-Segment Display (Sparkfun# COM-09230)
- 'SFE-0012-DS-7segmentSerial-v41.pdf' - NEW 7-Segment Display (Sparkfun# COM-09764)

  *Sparkfun 7-Segment Module Versions:*
  - Differ in the characters they can display, the commands they support, and brightness level
  - So conditional complication is used to set variables/defines for the proper device version
  - Make sure the appropriate device is #defined in the SSclockInits.h file (default is NEW version)

**Photos** - in the "photos" sub-folder
- 'photo-clock-labeled-internals.jpg' - photo of internal of clock implementation
- 'photo-clock-outside-front.jpg' - front of clock implementation
- 'photo-clock-outside-rear.jpg' - rear of clock implementation
