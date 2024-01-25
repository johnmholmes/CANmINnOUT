<img align="right" src="arduino_cbus_logo.png"  width="150" height="75">

## This fork last edited 24/1/2024

### Changes from Martin's original repository

Original sketch has been removed from this repository and is being replaced by seperate sketchs to help people who do not want to alter any code.

- Added a wiki for my benifit so I can keep a record of any changes to the sketch I make. Also I will be showing how I have used the Arduino Nano and the small MCP2515 module for my YouTube channel to help beginners to Arduino CBUS.
- Showing a 6 in 6 out version of the sketch.


### See anything I requested to be changed

- Warning amended as its the Atmel chip that gets damaged. (Change requested issue closed) Minor rewrite of warning
-----
# Martin's Read Me
  
## CANmINnOUT

An Arduino program to allocate all available device pins as either switch input or LED output.
To this end, the number of input pins is regarded as 'm' and the number of output pins as 'n'.

Key Features:
- MERG CBUS interface.
- LED flash rate selectable from event variables.
- Switch function controllable by node variables.
- Modular construction to ease adaptation to your application.
- Start-of-Day reporting selectable from event variables.

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

## Using CANmINnOUT

The MCP2515 interface requires five Arduino pins to be allocated. Three of these are fixed
in the architecture of the Arduino processor. One pin must be connected to an interrupt
capable Arduino pin. The Chip Select pin can be freely defined. Thus, the total number of
pins available for input or output is:
- UNO  13 pins
- NANO 15 pins of which only 13 are usable in this application.
- MEGA 63 pins

The total of input and output pins (m + n) cannot exceed these numbers.

The Nano also has inputs A6 & A7.  However, these are analogue inputs only and have
no digital input or output capability. These are thus of no interest in this application.

**It is the users responsibility to ensure that the total current that the Arduino is asked to supply stays within the capacity of the on board regulator. 
The user should also ensure that the total pin out current source or sink stays within the maximum current rating of the Atmel processor. 
Failure to do this will result in terminal damage to your Arduino.** 



Pins defined as inputs are active low.  That is to say that they are pulled up by an 
internal resistor. The input switch should connect the pin to 0 Volts.

Pins defined as outputs are active high.  They will source current to (say) an LED. It is 
important that a suitable current limiting resistor is fitted between the pin and the LED 
anode.  The LED cathode should be connected to ground.

### Library Dependencies

The following third party libraries are required:

Library | Purpose
---------------|-----------------
Streaming.h  |*C++ stream style output, v5, (http://arduiniana.org/libraries/streaming/)*
Bounce2.h    |*Debounce of switch inputs*
ACAN2515.h   |*library to support the MCP2515/25625 CAN controller IC*
CBUS2515.h   |*CAN controller and CBUS class*
CBUSconfig.h |*module configuration*
CBUS.h       |*CBUS Class*
cbusdefs.h   |*Definition of CBUS codes*
CBUSParams.h   |*Manage CBUS parameters*
CBUSSwitch.h   |*library compatibility*
CBUSLED.h      |*library compatibility*

### Application Configuration

The module can be configured to the users specific configuration in a section of code 
starting at line 92 with the title DEFINE MODULE. The following parameters can be changed 
as necessary:
```
#define DEBUG 0       // set to 0 for no serial debug
```
This define at circa line 73 allows various output reports to be made to the serial monitor 
for use in debugging.  To enable these, change the value of DEBUG from 0 to 1.

```
// module name
unsigned char mname[7] = { 'm', 'I', 'N', 'n', 'O', 'U', 'T' };
```
This can be adjusted as required to reflect the module configuration.  For example, 'm' & 'n' 
could be changed to any number between 0 & 9. However, only one character is allowed between 
each pair of ' ' and the total number of characters must not exceed seven.

```
const unsigned long CAN_OSC_FREQ = 8000000;     // Oscillator frequency on the CAN2515 board
```
If the oscillator frequency on your CAN2515 board is not 8MHz, change the number to match. The 
module will not work if this number does not match the oscillator frequency.

```
//Module pins available for use are Pins 3 - 9 and A0 - A5
const byte LED[] = {8, 7};        // LED pin connections through 1K8 resistor
const byte SWITCH[] = {9, 6};     // Module Switch takes input to 0V.
```
Insert the pin numbers being used for inputs and outputs between the appropriate pair of braces.
Pin numbers must be seperated by a comma.

### CBUS Op Codes

The following Op codes are supported:

OP_CODE | HEX | Function
----------|---------|---------
 OPC_ACON | 0x90 | On event
 OPC_ACOF | 0x91 | Off event
 OPC_ASON | 0x98 | Short event on
 OPC_ASOF | 0x99 | Short event off

### Event Variables

Event Variables control the action to take when an event is received.
The number of Event Variables (EV) is equal to the number of LEDs plus 1.

The first Event Variable (EV1) controls Start-of-Day reporting. Set EV1 to 1 to enable 
reporting of state of the switches.

The remaining Event Variables control the LEDs. 
Event Variable 2 (EV2) controls the first LED pin in the ```LED``` array. 
EV3 controls the second LED pin, etc.

The following EV values are defined to control the LEDs:

 EV Value | Function
--------|-----------
 0 | LED off
 1 | LED on
 2 | LED flash at 500mS
 3 | LED flash at 250mS
 
### Node Variables

Node Variables control the action to take when a switch is pressed or depressed.

The number of Node Variables (NV) is equal to the number of switches.
NV1 corresponds to the first pin defined in the array ```SWITCH```, 
NV2 corresponds to the second pin in that array, etc.

The following NV values define input switch function:

NV Value | Function
--------|--------
 0 | No action
 1 | On/Off switch
 2 | On only push button
 3 | Off only push button
 4 | On/Off toggle push button
 
### Start Of Day

The module will respond to a Start Of Day event if that event has been taught and EV1 is set to 1.
Upon receipt of this SoD message, the unit will report the status of each switch that has an NV value
of 0 or 3. Clearly, "on" only and "off" only switch NV values are of no relevance to SoD.

There is no restriction on setting other EVs to cause LEDs to indicate receipt of an SoD event in
addition to setting EV1.
 
## Set Up and the Serial Monitor

Without a CBUS switch, it is necessary to have another means of registering the module on 
the CBUS and acquiring a node number.  This is accomplished by sending a single character to 
the Arduino using the serial send facility in the serial monitor of the Arduino IDE (or similar).

#### 'r'
This character will cause the module to renegotiate its CBUS status by requesting a node number.
The FCU will respond as it would for any other unrecognised module.

#### 'z'
This character needs to be sent twice within 2 seconds so that its action is confirmed.
This will reset the module and clear the EEPROM.  It should thus be used with care.

Other information is available using the serial monitor using other commands:

#### 'n'
This character will return the node configuration.

#### 'e'
This character will return the learned event table in the EEPROM.

#### 'v'
This character will return the node variables.

#### 'c'
This character will return the CAN bus status.

#### 'h'
This character will return the event hash table.

#### 'y'
This character will reset the CAN bus and CBUS message processing.

#### '\*'
This character will reboot the module.

#### 'm'
This character will return the amount of free memory. 
 
 
 
 
 
