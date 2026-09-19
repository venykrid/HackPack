# HackPack
I am building a electronic swiss army knife similar to a flipper zero but with dramatically more capabilities and a form factor similar to that of a magsafe power bank. This uses an Orange Pi Zero 4 as the primary brains for this operation. I am also integrating an ESP23c6 using esp-idf as a coprocessor to handle IR and certain background work. A list of currently planned hardware features:
- Wifi (all 3 bands) and Bluetooth
- IR send/receive
- NFC/RFID read and transmit
- USB C expansion slot
- RGB status indicator
Software:
- Debian Trixie
- Communicates through USB Ethernet primarily with the powering device exposing a web ui that makes all of the tools accessible

Currently this is a bit of a mess but I hope to turn it into something awesome

Follow along in the [docs](/docs/)