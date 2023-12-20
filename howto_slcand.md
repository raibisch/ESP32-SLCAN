## Howto get an SLCAN Adpater work on Linux

...all settings tested with original Lawicel USBCAN-Adapter

### first test with serial Terminal Prog
some Serial-Commands:

Input-Cmd    | Return-Val  | Desciption
-------------|--------------------|-----------
`V<CR>`      | `V1011<CR>`        | Version
`sx<CR>`     | `<CR>`             | 5=250kBbit, s6=500Kbit, s8=1Mbit
`O<CR>`      | `<CR>`             | Open CAN connection
`C<CR>`      | `<CR>`             | Close CAN connection

if`<NAK>` is returned : command is not accepted - try to Close the connection.

### Test can-support in Linux
1. Install the „can-utils“ package (which includes the required slcan daemon):
sudo apt-get install can-utils

2. Check if the kernel modules „can“, „can_raw“ and „slcan“ are already loaded:
lsmod
If not, load them manually:

```bash
sudo modprobe can
sudo modprobe can_raw
sudo modprobe slcan

```

### Init slcand
example with 250kBd CAN and 3MBd Serial on USB4 (replace s5 and USB4 with your parameter)

```bash
sudo slcand -o -s5 -t hw -S115200 /dev/ttyUSB0

sudo slcand -o -c -s8 -S1000000 /dev/ttyUSB0 can0 

2. Bring the device up
sudo ip link set can0 up 
```

if everything worked fine, you should now see the 'slcan0' network device. To activate it,
type the following command:

```bash
sudo ip link set up can0
```

HINT: in some older docs 'slcan0' is the network name - for me it works with 'can0'

### Test with candump
example output generated with CAN-Adapter connected with 2. SLCAN-adapter.

Output created with serial-terminal on 2.SLCAN-adapter:
`T0100011081122334455667788<CR>`

#### CAN-Utils

```bash
candump can0 # Show all traffic receiven on can0
...
can0  01000110   [8]  11 22 33 44 55 66 77 88

#..other commands:
cansend can0 999#DEADBEEF   # Send a frame to 0x999 with payload 0xdeadbeef
canbusload can0 500000      # Calculate bus loading percentage on can0 
cansniffer can0             # Display top-style view of can traffic
cangen can0 -D 11223344DEADBEEF -L 8    # Generate fixed-data CAN messages
```

### external links
<https://elinux.org/Bringing_CAN_interface_up#SLCAN_based_Interfaces>

<https://www.fischl.de/usbtin/linux_can_socketcan/>

<https://www.elektronik-keller.de/index.php/projekte/stm32/stm32-can>

#### extra Infos for CANable-Module with candelite firmware

getting up:

```bash
sudo ip link set can0 up type can bitrate 250000
 ```

<https://canable.io/getting-started.html>
