# Arduino Mega 2560 (ATmega2560, 16 MHz crystal, ATmega16U2 USB-serial bridge).
# Select with: make BOARD=atmega2560

MCU := atmega2560
BOARD_NAME := Arduino Mega 2560
F_CPU := 16000000UL
AVRDUDE_MCU := m2560

FLASH_SIZE := 262144
SRAM_SIZE := 8192
# Stock boot section is 4096 words (8 KiB) at the top of flash.
BOOTLOADER_SIZE := 8192
BOOTLOADER ?= 1
HEAP_SIZE := 4096
THREE_BYTE_PC := 1

# USB upload through the serial bridge. Official boards enumerate as ttyACM0
# (16U2). CH340 clones enumerate as ttyUSB0. Prefer a node that exists.
# Override with USB_PORT=/dev/ttyUSB0.
USB_PROGRAMMER := wiring
USB_BAUD := 115200
USB_PORT ?= $(shell if [ -e /dev/ttyACM0 ]; then echo /dev/ttyACM0; elif [ -e /dev/ttyUSB0 ]; then echo /dev/ttyUSB0; else echo /dev/ttyACM0; fi)

ISP_PROGRAMMER := dragon_isp
JTAG_PROGRAMMER := dragon_jtag
DEBUG_PORT ?= usb
AVRDUDE_BITCLOCK ?= 1
# AVaRICE -B is the JTAG bitrate in kHz. Dragon accepts 22 through 6400.
JTAG_BITRATE ?= 250
GDB_PORT ?= 4242

# Stock Arduino Mega 2560 fuses. Bits are active-low: 0 means programmed.
#   LFUSE 0xFF  external full-swing crystal, CKDIV8 off
#   HFUSE 0xD8  BOOTSZ=8 KiB, BOOTRST on, SPIEN on, JTAG off, OCD off
#   EFUSE 0xFD  BOD 2.7 V
# HFUSE_JTAG enables OCDEN and JTAGEN and leaves the bootloader in place.
# HFUSE_NOBOOT clears BOOTRST so reset enters the application.
# HFUSE_JTAG_NOBOOT does both.
FUSES_KNOWN := 1
LFUSE := 0xFF
HFUSE := 0xD8
EFUSE := 0xFD
HFUSE_JTAG := 0x18
HFUSE_NOBOOT := 0xD9
HFUSE_JTAG_NOBOOT := 0x19

# LED "L" is digital 13, PB7, active high.
# USART0 (the USB console) is PE0/RXD0 and PE1/TXD0, Arduino pins 0 and 1.
# JTAG, once enabled, occupies analog A4-A7:
#   TCK PF4 (A4), TMS PF5 (A5), TDO PF6 (A6), TDI PF7 (A7), plus RESET.
BOARD_DEFS := \
	-DBOARD_HAS_LED=1 \
	-DBOARD_LED_DDR=DDRB \
	-DBOARD_LED_PORT=PORTB \
	-DBOARD_LED_PIN=PINB \
	-DBOARD_LED_BIT=PB7 \
	-DBOARD_USART_BAUD=115200UL \
	-DBOARD_NAME='"Arduino Mega 2560"'
