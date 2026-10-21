# ATmega1284P template. There is no single Arduino-style board for this part,
# so USB upload is left unset and the LED is off until you name the pin.
# Select with: make BOARD=atmega1284p
# Override the crystal with: make BOARD=atmega1284p F_CPU=20000000UL

MCU := atmega1284p
BOARD_NAME := ATmega1284P
F_CPU := 16000000UL
AVRDUDE_MCU := m1284p

FLASH_SIZE := 131072
SRAM_SIZE := 16384
# Typical Optiboot section if you later enable BOOTLOADER=1. Full flash is
# the default because a bare 1284P has no bootloader.
BOOTLOADER_SIZE := 1024
BOOTLOADER ?= 0
HEAP_SIZE := 10240
THREE_BYTE_PC := 0

# Set USB_PROGRAMMER (arduino or wiring) and USB_BAUD once a serial
# bootloader is actually installed.
USB_PROGRAMMER :=
USB_BAUD := 115200
USB_PORT ?= /dev/ttyUSB0

ISP_PROGRAMMER := dragon_isp
JTAG_PROGRAMMER := dragon_jtag
DEBUG_PORT ?= usb
AVRDUDE_BITCLOCK ?= 1
# AVaRICE -B is the JTAG bitrate in kHz. Dragon accepts 22 through 6400.
JTAG_BITRATE ?= 250
GDB_PORT ?= 4242

# Fuses are board-specific (clock source in particular). Read them before
# writing any. JTAG pins on this device are PC2/TCK, PC3/TMS, PC4/TDO, PC5/TDI.
FUSES_KNOWN := 0
LFUSE :=
HFUSE :=
EFUSE :=
HFUSE_JTAG :=
HFUSE_NOBOOT :=
HFUSE_JTAG_NOBOOT :=

# USART0 is PD0/RXD0 and PD1/TXD0. Define the LED when the board has one, for
# example -DBOARD_HAS_LED=1 -DBOARD_LED_DDR=DDRB -DBOARD_LED_PORT=PORTB
# -DBOARD_LED_PIN=PINB -DBOARD_LED_BIT=PB0
BOARD_DEFS := \
	-DBOARD_HAS_LED=0 \
	-DBOARD_USART_BAUD=115200UL \
	-DBOARD_NAME='"ATmega1284P"'
