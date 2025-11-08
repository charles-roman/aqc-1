# AQC-1

![Flight GIF](Media/flight.gif)

---

## Table of Contents
1. [Background](#background)  
2. [Overview](#overview)  
   - [System Diagram](#system-diagram)
   - [Control System Diagram](#control-system-diagram)
3. [Flight Software](#flight-software)  
   - [RX Module](#rx-module)  
   - [Sensor Modules](#sensor-modules)
      - [IMU](#imu) 
   - [Core Flight Control Software](#core-flight-control-software)  
   - [ESC Module](#esc-module)  
   - [Miscellaneous](#miscellaneous)  
4. [Future Updates](#future-updates)  
5. [Getting Started](#integrating-this-code-with-your-own-hardware)  
6. [Contributing](#contributing)  

---

## Background
Behold. Fully functioning, uniquely designed, open-source quadcopter flight software.

This project, dubbed **"aqc-1,"** contains a straightforward and easily scalable flight software stack broken into stand-alone modules. It is intended for an STM32 platform, but could be adapted to others meeting the minimum hardware requirements.  

If you have visited this repo before, please give it another look — it has had a major overhaul since the original publishing.  

In retrospect, C was not the ideal language for this project. Features such as Object-Oriented Programming, namespacing, overloading, and templates in C++ would benefit this project greatly. A full code-base migration to C++ may be in the cards for a future update.  

The primary motivation behind **aqc-1** is to deepen my understanding of embedded software design and control systems, while also providing a beginner-friendly open-source flight software stack for those looking to gain knowledge and experience in this area.

---

## Overview  
The current system hardware is built out as shown in the diagram below.

### System Diagram
![System Schematic](Media/system-schematic.jpg)

Current Hardware:  
- RadioMaster Zorro remote control
- MATEKSYS ELRS 2.4GHz PWM receiver
- LSM6DSOX IMU
- STM32F405-RGT6 MCU
- SpeedyBee F405 BLS 50A 30x30 4-in-1 ESC

Future Hardware Additions:  
- BMP3090 Barometer
- MTK3333 GPS

This hardware along with the current working control software implement only an attitude control system, supporting both ANGLE and RATE modes which are standard on most modern hobbyist quadcopters. The control system design schematic for the attitude control system is shown below.

### Control System Diagram
![Control System Schematic](Media/control-system-schematic.jpg)  

For additional information regarding the hardware selection, component configuration, or control system design, please visit: [Portfolio_Link](https://portfolium.com/CharlesRoman2/portfolio)

---

## Flight Software
Currently, the flight software is structured as a **main-owned, stack-allocated coordination model** running continuously in a super-loop. That is to say, the main application initializes and owns the critical data for running the flight control system and passes this data between functions via pointers to the relevant handles. Future improvements adding altitude and position control will likely require the addition of a scheduler for efficient operation.

The architecture is broken into individual device modules to manage communication with each of the peripheral devices and a core flight control package, which is responsible for handling the core flight control logic. In addition to their functionality, each module maintains a simple error reporting interface which can easily be scaled to handle more detailed error reporting capabilities.

Each sub-section below will provide a slightly more in-depth look at these individual modules along with some examples of their functionality.

### RX Module
- `details coming soon...`

### Sensor Modules
### IMU
- `details coming soon...`

### Core Flight Control Software
- `details coming soon...`

### ESC Module
- `details coming soon...`

### Miscellaneous
- `details coming soon...`

---

## Future Updates
- `details coming soon...`

---

## Integrating This Code With Your Own Hardware
- `details coming soon...`

---

## Contributing
- `details coming soon...`

---

## License
- `details coming soon...`