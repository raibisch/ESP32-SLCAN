# ESP32-SCLAN  

This repo contains the source code for CAN-over-Serial-Line interfaces based on [Lawicel SLCAN protocol](http://www.can232.com/docs/canusb_manual.pdf).
This work is mostly based on the work from:

* SLCAN protocol on ESP32: [mintynet/esp32-sclan](https://github.com/mintynet/esp32-slcan)
* ESP32CAN-Class: [miwagner/ESP32-Arduino-CAN](https://github.com/miwagner/ESP32-Arduino-CAN)
* ESP32 CAN driver: [ThomasBarth/ESP32-CAN-Driver](https://github.com/ThomasBarth/ESP32-CAN-Driver)

it was tested and developed on PlatformIO, but could easy adpated to ARDUINO-IDE.

## Hardware-Adaption

For propper CAN-Signals we need a hardware CAN controller TJA1050:

#### Datasheet

<https://www.mikrocontroller.net/part/TJA1050>

#### Board-Example

<https://www.komputer.de/zen/index.php?main_page=product_info&products_id=437>

## Standard Settings

* NEW: Serial-Baudrate could be (persistent) switched from 115200Bd to 460800Bd for better performance at high CAN      busload. Command 'b0=115200Bd' b1=460800Bd'
* NEW: Command 'x' for ESP32 restart (Softare-Reset)
* CAN-Bus Baudrate : 250kBd ...could be chanced with 'S' command
* CAN-Pins: CAN_TX_PIN GPIO_NUM_5, CAN_RX_PIN GPIO_NUM_4 (conntect TJA1050 to this pins)
* Serial-Bluetooth: disabled (could be enabled with '#define BT_SERIAL'

## CAN-Applications supporting SLCAN

the [CANable-Projekt getting started](https://canable.io/getting-started.html)

## Download and Installation

### Arduino IDE

To download click the DOWNLOAD ZIP button, rename the uncompressed folder. rename 'main.cpp' to 'ESP32-SCLAN.ino' and remove '#include <Ardunio.h>'.
All required libs are local in the 'lib' folder, so there is no need for external libraries.

### Platform IO

To download click the DOWNLOAD ZIP button form Github. All required libs are local in the 'lib' folder) so there is no need for external libraries.
the hardware platform is defined for minumum 'DEV-KIT V1' and should match for most ESP32 hardware.
