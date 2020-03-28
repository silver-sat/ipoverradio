# IP via Radio

Demonstration code / configuration for setting up a raspberry pi to respberry pi IP connection via radio connected arduinos. Tested on two Raspbery Pi Zero Ws running Rapberian buster, two Arduino MEGA 2560s, each using a RFM96W radio chip via SPI.

## Notes
1. The arduino code is designed for AX25 based IP connections, but could probably be addapted for PPP.
1. The arduino code is designed to detect and transmit AX25 packets.
1. The RPi sends serial data in blocks that may contain multiple KISS framed packets. 
1. The RFM96W chip can send at most 255 bytes per message and is only half duplex.
1. Droping the MTU low enough to guarantee AX25 packets less than 255 bytes does not appear reliable, even though this appears technically legal, some documentation advises at least 476 bytes.
1. Reliability of single messages seems to drop off (for me) above 250 bytes, not sure why.
1. The arduino code works for 512 byte MTU, using up to three messages if necessary, per AX25/KISS frame.
1. There is no acknowledgement of messages or packets in the radio layer.
1. The connection works OK as long as the messages are one-way or the other, but when multiple simultaneous messages back and forth are sent, packtes get dropped. It is unclear whether these are droped in the serial connection, the radio connection etc. 
1. Pings are easy as they are single direction and can be constrained in size. Big pings are mostly reliable, even when multi-messages are needed. The -s option for ping will change the size of the ping.
1. The connection will support a (slow) SSH login from RPi-a to RPi-b, which suggests bi-directional communication is working. Diagnostic logs suggest that most of these IP packets are small and the packets are request and response, and seems to work OK. The connection seems to transmit about 100-200 bytes / sec in this mode. 
1. The connection will support a HTTP request download using wget if --limit-rate 512 is used. 

## Preamble
See instructions for [IP over Serial](../ipoverserial/README.md) for the RPi configuration needed. This is not changed for IP via Radio. Note that the MTU should be set to 512 in /etc/ax25/axports.

## Instructions (for each side of the radio link)
1. Install [4-channel BSS138 I2C-safe Bi-directional Logic Level Converter](https://www.adafruit.com/product/757) (LLC) in breadboard. 
1. Install [RFM96W LoRa Radio Transceiver Breakout - 433 MHz](https://www.adafruit.com/product/3073) (RFM96W) in breadboard. 
1. Connect wires:
   * RPi-a GPIO GND (pin 6) to LLC LOW-SIDE GND 
   * RPi-a GPIO RX (pin 10) to LLC A1
   * RPi-a GPIO TX (pin 8) to LLC A2
   * RPi-a GPIO 3.3V (pin 1) to LLC LV
   * MEGA GND to LLC HIGH-SIDE GND
   * MEGA TX3 (pin 14) to LLC B1
   * MEGA RX3 (pin 15) to LLC B2
   * MEGA 5V to LLC HV
   * MEGA GND to RFM95W GND
   * MEGA PIN 2 to RFM95W RST
   * MEGA PIN 3 to RFM95W DIO0/G0
   * MEGA PIN 4 to RFM95W CS
   * MEGA PIN 50 to RFM95W MISO
   * MEGA PIN 51 to RFM95W MOSI
   * MEGE PIN 52 to RFM95W SCK
   * MEGA 5V to RFM95W VIN
  
   <A href="ipviaradio_schem.png"><img src="ipviaradio_schem.png" width=700></A>
   
1. Push the file [RadioPassthrough.ino](RadioPassthrough.ino) to the Arduino Mega
1. Reboot the raspberry pis. 
   
   
