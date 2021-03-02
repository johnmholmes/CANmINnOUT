
//
///
//
/**************************************************************************************
   Version 1
  Allows all available Arduino pins to be allocated as either an input or output (but not both!).
  Each output pin, here defined as LED, is assigned to an Event Variable that can be
  used in any combination against a received event.
  An input pin, here defined as switch, will generate an on or off event.
  The event handler has been re-written so that action is determined by op Code.
  Op codes supported are ACON, ASON, ACOF, ACON.
  This sketch doe snot include the use of the CBUS switch or LEDs.  The CBUS and CBUSConfig
  libraries must be able to accomodate this.
***************************************************************************************
*/

/*
  Copyright (C) 2021 Martin Da Costa
  Including copyrights from CBUS_1in1out and Arduino CBUS Libraries


  This work is licensed under the:
      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
   To view a copy of this license, visit:
      http://creativecommons.org/licenses/by-nc-sa/4.0/
   or send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.

   License summary:
    You are free to:
      Share, copy and redistribute the material in any medium or format
      Adapt, remix, transform, and build upon the material

    The licensor cannot revoke these freedoms as long as you follow the license terms.

    Attribution : You must give appropriate credit, provide a link to the license,
                  and indicate if changes were made. You may do so in any reasonable manner,
                  but not in any way that suggests the licensor endorses you or your use.

    NonCommercial : You may not use the material for commercial purposes. **(see note below)

    ShareAlike : If you remix, transform, or build upon the material, you must distribute
                 your contributions under the same license as the original.

    No additional restrictions : You may not apply legal terms or technological measures that
                                 legally restrict others from doing anything the license permits.

   ** For commercial use, please contact the original copyright holder(s) to agree licensing terms

    This software is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE

*/

/*
      3rd party libraries needed for compilation:

      Streaming   -- C++ stream style output, v5, (http://arduiniana.org/libraries/streaming/)
      ACAN2515    -- library to support the MCP2515/25625 CAN controller IC
*/
///////////////////////////////////////////////////////////////////////////////////
// Pin Use map UNO:
// Digital pin 2          Interupt CAN
// Digital pin 3 (PWM)    Not Used
// Digital pin 4          Not Used
// Digital pin 5 (PWM)    Not Used
// Digital pin 6 (PWM)    Module Switch 2
// Digital pin 7          Module LED 2
// Digital pin 8          Module LED 1
// Digital pin 9 (PWM)    Module Switch 1
// Digital pin 10 (SS)    CS    CAN
// Digital pin 11 (MOSI)  SI    CAN
// Digital pin 12 (MISO)  SO    CAN
// Digital pin 13 (SCK)   Sck   CAN

// Digital / Analog pin 0     Not Used
// Digital / Analog pin 1     Not Used
// Digital / Analog pin 2     Not Used
// Digital / Analog pin 3     Not Used
// Digital / Analog pin 4     Not Used
// Digital / Analog pin 5     Not Used
//////////////////////////////////////////////////////////////////////////

#define DEBUG 1       // set to 0 for no serial debug

// 3rd party libraries
#include <Streaming.h>
#include <Bounce2.h>

// CBUS library header files
#include <CBUS2515.h>            // CAN controller and CBUS class
#include "LEDControl.h"          // CBUS LEDs
#include <CBUSconfig.h>          // module configuration
#include <cbusdefs.h>            // MERG CBUS constants
#include "CBUSParams.h"

////////////DEFINE MODULE/////////////////////////////////////////////////

// module name
unsigned char mname[7] = { 'm', 'I', 'N', 'n', 'O', 'U', 'T' };

// constants
const byte VER_MAJ = 1;         // code major version
const char VER_MIN = " ";       // code minor version
const byte VER_BETA = 0;        // code beta sub-version
const byte MODULE_ID = 99;      // CBUS module type

#define NUM_LEDS 2              // How many LEDs are there?
#define NUM_SWITCHES 2          // How many switchs are there?

//Module pins available for use are Pins 3 - 9 and A0 - A5
const byte LED[NUM_LEDS] = {8, 7};            // LED pin connections through 1K8 resistor
const byte SWITCH[NUM_SWITCHES] = {9, 6};     // Module Switch takes input to 0V.

// module objects
Bounce moduleSwitch[NUM_SWITCHES];  //  switch as input
LEDControl moduleLED[NUM_LEDS];     //  LED as output


//////////////////////////////////////////////////////////////////////////

//CBUS pins
const byte CAN_INT_PIN = 2;  // Only pin 2 and 3 support interrupts
const byte CAN_CS_PIN = 10;
//const byte CAN_SI_PIN = 11;  // Cannot be changed
//const byte CAN_SO_PIN = 12;  // Cannot be changed
//const byte CAN_SCK_PIN = 13;  // Cannot be changed

// CBUS objects
CBUS2515 CBUS;                      // CBUS object
CBUSConfig config;                  // configuration object

//
///  setup CBUS - runs once at power on called from setup()
//
void setupCBUS()
{
  // set config layout parameters
  config.EE_NVS_START = 10;
  config.EE_NUM_NVS = 10;
  config.EE_EVENTS_START = 50;
  config.EE_MAX_EVENTS = 64;
  config.EE_NUM_EVS = NUM_LEDS;
  config.EE_BYTES_PER_EVENT = (config.EE_NUM_EVS + 4);

  // initialise and load configuration
  config.setEEPROMtype(EEPROM_INTERNAL);
  config.begin();

  Serial << F("> mode = ") << ((config.FLiM) ? "FLiM" : "SLiM") << F(", CANID = ") << config.CANID;
  Serial << F(", NN = ") << config.nodeNum << endl;

  // show code version and copyright notice
  printConfig();

  // set module parameters
  CBUSParams params(config);
  params.setVersion(VER_MAJ, VER_MIN, VER_BETA);
  params.setModuleId(MODULE_ID);
  params.setFlags(PF_FLiM | PF_COMBI);
  // params.setProcessor(CPUM_ATMEL, 0x32, "328P");

  // assign to CBUS
  CBUS.setParams(params.getParams());
  CBUS.setName(mname);

  // register our CBUS event handler, to receive event messages of learned events
  CBUS.setEventHandler(eventhandler);

  // configure and start CAN bus and CBUS message processing
  CBUS.setNumBuffers(2);         // more buffers = more memory used, fewer = less
#if CANBUS8MHZ
  CBUS.setOscFreq(8000000UL);   // select the crystal frequency of the CAN module to 8MHz
#else
  CBUS.setOscFreq(16000000UL);   // select the crystal frequency of the CAN module to 16Mhz
#endif
  CBUS.setPins(10, 2);           // select pins for CAN bus CE and interrupt connections
  CBUS.begin();
}

//
/// setup - runs once at power on
//

void setup() {

  Serial.begin (115200);
  Serial << endl << endl << F("> ** CBUS no HWUI 2 in 2 out v1 ** ") << __FILE__ << endl;

  setupCBUS();

  ///////////////////////////////////////////////////////////////////////////
  // configure the module switches, active low
  for (int i = 0; i < NUM_SWITCHES; i++)
  {
    moduleSwitch[i].attach(SWITCH[i], INPUT_PULLUP);
    moduleSwitch[i].interval(5);
  }

  // configure the module LEDs
  for (int i = 0; i < NUM_LEDS; i++) {
    moduleLED[i].setPin(LED[i]);
  }
  ///////////////////////////////////////////////////////////////////////////
  // end of setup
#if DEBUG
  Serial << F("> ready") << endl << endl;
#endif
}

//
/// loop - runs forever
//

void loop() {

  //
  /// do CBUS message, switch and LED processing
  //

  CBUS.process();

  //
  /// process console commands
  //

  processSerialInput();

  //
  /// Run the LED code
  //

  for (int i = 0; i < NUM_LEDS; i++) {
    moduleLED[i].run();
  }

  //
  /// test for switch input
  //

  for (int i = 0; i < NUM_SWITCHES; i++)
  {
    moduleSwitch[i].update();
    if (moduleSwitch[i].changed())
    {
#if DEBUG
      Serial << F("> Button ") << i << F(" state change detected") << endl;
      if (moduleSwitch[i].fell()) {
        Serial << F("> Button ") << i << F(" pressed, send 0x") << _HEX(OPC_ACON) << endl;
      } else {
        Serial << F("> Button ") << i << F(" released, send 0x") << _HEX(OPC_ACOF) << endl;
      }
#endif

      byte opCode = (moduleSwitch[i].fell() ? OPC_ACON : OPC_ACOF);
      sendEvent(opCode, (i + 1));
    }
  }
  //
  /// bottom of loop()
  //
}

// Send an event routine according to Module Switch
bool sendEvent(byte opCode, unsigned int eventNo)
{
  CANFrame msg;
  msg.id = config.CANID;
  msg.len = 5;
  msg.data[0] = opCode;
  msg.data[1] = highByte(config.nodeNum);
  msg.data[2] = lowByte(config.nodeNum);
  msg.data[3] = highByte(eventNo);
  msg.data[4] = lowByte(eventNo);

#if DEBUG
  if (CBUS.sendMessage(&msg)) {
    Serial << F("> sent CBUS message with Event Number ") << eventNo << endl;
  } else {
    Serial << F("> error sending CBUS message") << endl;
  }
#endif
}


//
/// user-defined event processing function
/// called from the CBUS library when a learned event is received
/// it receives the event table index and the CAN frame
//

void eventhandler(byte index, CANFrame *msg) {
  byte opc;
  byte ev;
  int eeaddress;
  byte evval;

#if DEBUG
  Serial << F("> event handler: index = ") << index << F(", opcode = 0x") << _HEX(msg->data[0]) << endl;
#endif

  opc = msg->data[0];

  switch (msg->data[0]) {

    case OPC_ACON:
    case OPC_ASON:

      for (int i = 0; i < NUM_LEDS; i++) {

        ev = i + 1;
        eeaddress = config.EE_EVENTS_START + (index * config.EE_BYTES_PER_EVENT) + 4 + (ev - 1);
        evval = config.readEEPROM(eeaddress);

        switch (evval) {
          case 1:
            moduleLED[i].flash(500);
            break;

          case 2:
            moduleLED[i].flash(250);
            break;

          default:
            break;
        }
      }
      break;

    case OPC_ACOF:
    case OPC_ASOF:

      for (int i = 0; i < NUM_LEDS; i++) {

        ev = i + 1;
        eeaddress = config.EE_EVENTS_START + (index * config.EE_BYTES_PER_EVENT) + 4 + (ev - 1);
        evval = config.readEEPROM(eeaddress);

        if (evval > 0) {
          moduleLED[i].off();
        }
      }
      break;

  }
}

//
/// print code version config details and copyright notice
//

void printConfig(void) {

  // code version
  Serial << F("> code version = ") << VER_MAJ << VER_MIN << F(" beta ") << VER_BETA << endl;
  Serial << F("> compiled on ") << __DATE__ << F(" at ") << __TIME__ << F(", compiler ver = ") << __cplusplus << endl;

  // copyright
  Serial << F("> © Martin Da Costa (MERG M6223) 2020") << endl;


  return;
}

//
/// command interpreter for serial console input
//

void processSerialInput(void) {

  byte uev = 0;
  char msgstr[32], dstr[32];

  if (Serial.available()) {

    char c = Serial.read();

    switch (c) {

      case 'n':

        // node config
        printConfig();

        // node identity
        Serial << F("> CBUS node configuration") << endl;
        Serial << F("> mode = ") << (config.FLiM ? "FLiM" : "SLiM") << F(", CANID = ") << config.CANID << F(", node number = ") << config.nodeNum << endl;
        Serial << endl;
        break;

      case 'e':

        // EEPROM learned event data table
        Serial << F("> stored events ") << endl;
        Serial << F("  max events = ") << config.EE_MAX_EVENTS << F(" EVs per event = ") << config.EE_NUM_EVS << F(" bytes per event = ") << config.EE_BYTES_PER_EVENT << endl;

        for (byte j = 0; j < config.EE_MAX_EVENTS; j++) {
          if (config.getEvTableEntry(j) != 0) {
            ++uev;
          }
        }

        Serial << F("  stored events = ") << uev << F(", free = ") << (config.EE_MAX_EVENTS - uev) << endl;
        Serial << F("  using ") << (uev * config.EE_BYTES_PER_EVENT) << F(" of ") << (config.EE_MAX_EVENTS * config.EE_BYTES_PER_EVENT) << F(" bytes") << endl << endl;

        Serial << F("  Ev#  |  NNhi |  NNlo |  ENhi |  ENlo | ");

        for (byte j = 0; j < (config.EE_NUM_EVS); j++) {
          sprintf(dstr, "EV%03d | ", j + 1);
          Serial << dstr;
        }

        Serial << F("Hash |") << endl;

        Serial << F(" --------------------------------------------------------------") << endl;

        // for each event data line
        for (byte j = 0; j < config.EE_MAX_EVENTS; j++) {

          if (config.getEvTableEntry(j) != 0) {
            sprintf(dstr, "  %03d  | ", j);
            Serial << dstr;

            // for each data byte of this event
            for (byte e = 0; e < (config.EE_NUM_EVS + 4); e++) {
              sprintf(dstr, " 0x%02hx | ", config.readEEPROM(config.EE_EVENTS_START + (j * config.EE_BYTES_PER_EVENT) + e));
              Serial << dstr;
            }

            sprintf(dstr, "%4d |", config.getEvTableEntry(j));
            Serial << dstr << endl;
          }
        }

        Serial << endl;

        break;

      // NVs
      case 'v':

        // note NVs number from 1, not 0
        Serial << "> Node variables" << endl;
        Serial << F("   NV   Val") << endl;
        Serial << F("  --------------------") << endl;

        for (byte j = 1; j <= config.EE_NUM_NVS; j++) {
          sprintf(msgstr, " - %02d : %3hd | 0x%02hx", j, config.readNV(j), config.readNV(j));
          Serial << msgstr << endl;
        }

        Serial << endl << endl;

        break;

      // CAN bus status
      case 'c':

        CBUS.printStatus();
        break;

      case 'h':
        // event hash table
        config.printEvHashTable(false);
        break;

      case 'y':
        // reset CAN bus and CBUS message processing
        CBUS.reset();
        break;

      case '*':
        // reboot
        config.reboot();
        break;

      case 'm':
        // free memory
        Serial << F("> free SRAM = ") << config.freeSRAM() << F(" bytes") << endl;
        break;

      case 'r':
        // renegotiate
        CBUS.renegotiate();
        break;

      case 'z':
        // Reset module, clear EEPROM
        static bool ResetRq = false;
        static unsigned long ResWaitTime;
        if (!ResetRq) {
          // start timeout timer
          Serial << F(">Reset & EEPROM wipe requested. Press 'z' again within 2 secs to confirm") << endl;
          ResWaitTime = millis();
          ResetRq = true;
        }

        else {
          // This is a confirmed request
          // 2 sec timeout
          if (ResetRq && ((millis() - ResWaitTime) > 2000)) {
            Serial << F(">timeout expired, reset not performed") << endl;
            ResetRq = false;
          }

          else {
            //Request confirmed within timeout
            Serial << F(">RESETTING AND WIPING EEPROM") << endl;
            config.resetModule();
            ResetRq = false;
          }
        }
        break;


      default:
        // Serial << F("> unknown command ") << c << endl;
        break;
    }
  }
}
