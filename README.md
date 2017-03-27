# Arduino Clock Example Code
### Demonstration Code for: I2C, Interrupts, external Header File, Preprocessor Commands: Defines and Conditional Compilation
[![Code Climate](https://codeclimate.com/github/robertpeteuil/arduino-clock-example/badges/gpa.svg)](https://codeclimate.com/github/robertpeteuil/arduino-clock-example)
[![GitHub issues](https://img.shields.io/github/issues/robertpeteuil/arduino-clock-example.svg)](https://github.com/robertpeteuil/arduino-clock-example)
[![GitHub release](https://img.shields.io/github/release/robertpeteuil/arduino-clock-example.svg?colorB=2067b8)](https://github.com/robertpeteuil/arduino-clock-example)
[![lang](https://img.shields.io/badge/language-C%2B%2B-f34b7d.svg?style=flat-square)]()
[![lang](https://img.shields.io/badge/language-Arduino-bd79d1.svg?style=flat-square)]()
[![license](https://img.shields.io/github/license/robertpeteuil/arduino-clock-example.svg?colorB=2067b8)](https://github.com/robertpeteuil/arduino-clock-example)

---

The project is intended to server as a reference and example for:
- Accessing a module via I2C
- Using hardware interrupts and assigning functions for varying parameters
- Read and debounce button presses via hardware interrupts    
- Parsing a serial stream to get user data
- Storing data in non-volatile memory via I2C
- Including and using an External Header File
- Using Compiler PreProcessor Commands - Defines, and Conditional Compilation

To maximize ease of re-use:
- The code is broken into discrete functions.
- The Functions are named by device: 7-Segment-Display, RTC, UART, and Menu.
- The code is extensively commented to make it easy to re-purpose.  

### Functionality

This project reads the current time from a RTC, displays it on a 7-Segment Display Module, allows external configuration of settings via buttons, and uses sound to provide user feedback.  

Details:
- Set / Read current time from DS1307 module
- Displays time on 7-Segment Display
- Interrupt-driven realtime display of time
- Outputs sound via small speaker
- Use Hardware buttons to allow user settings changes  

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
