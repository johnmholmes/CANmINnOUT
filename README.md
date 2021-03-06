# CBUSmINnOUT
An Arduino program to allocate all available pins as either switch input or LED output.

Key Features:
- MERG CBUS interface
- LED flash rate selectable from event variables
- Switch function controllable by node variables
- Modular construction to ease adaptation to your application.

## Overview

The program is written in C++ but you do not need to understand this to use the program.
The program includes a library that manages the LED functionality.
The program does not allow for the inclusion of a CBUS setup switch or LEDs. Consequently,
the following library versions are required:
- CBUS Version 1.1.14 or later
- CBUSConfig 1.1.10 or later
Notwithstanding the fact that they are not used, CBUSSwitch and CBUSLED libraries must be 
available for access by the CBUS and CBUSConfig libraries for reason of backwards library
compatibility. 

## Library Dependancies

The following third party libraries are required:
Library | Purpose
---------------|-----------------
Streaming.h  |*C++ stream style output, v5, (http://arduiniana.org/libraries/streaming/)*
Bounce2.h    |*Debounce of switch inputs*
ACAN2515.h   |*library to support the MCP2515/25625 CAN controller IC*
CBUS2515.h   |*CAN controller and CBUS class
CBUSconfig.h |*module configuration*
CBUS.h       |*CBUS Class*
cbusdefs.h   |*Definition of CBUS codes*
CBUSParams   |*Manage CBUS parameters*
CBUSSwitch   |*library compatibility*
CBUSLED      |*library compatibility*

### CBUS Op Codes

The following Op codes are supported:
OP_CODE | HEX | Function
----------|---------|---------
 OPC_ACON | 0x90 | On event
 OPC_ACOF | 0x91 | Off event
 OPC_ACOF | 0x98 | Short event on
 OPC_ASOF | 0x99 | Short event off

### Event Variables

The following EV values are defined to control the LEDs:
 EV Value | Function
--------|-----------
 0 | LED off
 1 | LED on
 2 | LED flash at 500mS
 3 | LED flash at 250mS
 
 
 
 
 
