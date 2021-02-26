  This Arduino program allows all available Arduino pins to be allocated as either an input or output (but not both!).
  Each output pin, here defined as LED, is assigned to an Event Variable that can be
  used in any combination against a received event.
  An input pin, here defined as switch, will generate an on or off event.
  The event handler has been re-written so that action is determined by op Code.
  Op codes supported are ACON, ASON, ACOF, ACON.
  This sketch doe snot include the use of the CBUS switch or LEDs.  The CBUS and CBUSConfig
  libraries must be able to accomodate this.
 
