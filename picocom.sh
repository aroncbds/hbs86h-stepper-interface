#!/bin/bash

echo -e "Connecting to ATMega2560-board..."
picocom -b 115200 /dev/ttyUSB0