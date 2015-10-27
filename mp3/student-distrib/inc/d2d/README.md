## d2d stands for "driver to driver"

## Description

These headers define interfaces between drivers within kernel, so that drivers do not have to communicate using user-level protocols.

## For example

Terminal will define some "callback" functions here for any input device to call. Upon ps/2 keyboard interrupt, usb keyboard interrupt, and mouse interrupt (if any), the interrupt handlers will call the function of their kind and invoke terminal to display inputs on the display.
