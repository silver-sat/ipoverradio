# ipoverserial
Demonstration code / configuration for setting up a raspberry pi internet connection over a serial link to another (internet connected) raspberry pi. Tested on two Raspbery Pi Zero Ws running Rapberian buster. 

## Preamble
Get things up and running so both devices can be accessed on the command-line from a single laptop.
1. Get each raspbery pi running and configured by connecting a keyboard, mouse, and monitor as needed. Call one `rpi-a` and one `rpi-b`. 
2. Run raspi-config and ensure that ssh, i2c, spi, and hardware serial are on, and that the login shell is not accessible over the serial interface. 
3. Set up each raspberry pi to automatically connect to a convenient WiFi network by editing `/etc/wpa_supplicant/wpa_supplicant.conf` to provide SSID and password - append the following at the end of file. `
    network={
        ssid="<SSID>"
        psk="<PASSWORD>"
    }
`
4. Set the WiFi router to provide consistent predictable IP addresses for each raspberry pi (after initial connection you'll usually be able to see each pi's MAC address). I used 10.0.0.101 and 10.0.0.102. 
5. Reboot the raspbery pis so they get assigned their assigned IP addresses. 
6. Download the [PuTTY](https://www.chiark.greenend.org.uk/~sgtatham/putty/latest.html) ssh client and check you can connect to each pi at their IP address (10.0.0.101 or 10.0.0.102), using the username pi, and the password set in step 1 or 2. 

## Instructions
1. If not already done, run raspi-config and ensure that hardward serial is on, and the login shell is not accessible over the serial interface.
2. RPi-A (10.0.0.101 over WiFi, 192.168.100.101 over serial) will be the bridge to the internet for RPi-B (10.0.0.102 over WiFi, 192.168.100.102 over serial).
3. Connect wires:
   * RPi-A GPIO GND (pin 6) to RPi-B GPIO GND (pin 6); RPi-A GPIO TX (pin 8) to RPi-B GPIO RX (pin 10); and RPi-A GPIO RX (pin 10) to RPi-B GPIO TX (pin 8). 
4. On both RPi-A and RPi-B, add the contents of [rc.local.insert.sh](rc.local.insert.sh) to the end of `/etc/rc.local` before `exit 0`.
```
sudo nano /etc/rc.local
```
5. On both RPi-A and RPi-B, add the contents of [axports](axports) to the end of `/etc/ax25/axports`.
```
sudo nano /etc/ax25/axports
```
6. On both RPi-A and RPi-B, ensure the ax25 tools are installed
```
% sudo apt install ax25-apps ax25-tools
```
7. On RPi-A, download [bridge_startup.sh](bridge_startup.sh) file to `/home/pi`. Link `.startup.sh` to `bridge_startup.sh` and set `.startup.sh` to be executable.
```
% cd /home/pi; ln -s bridge_startup.sh .startup.sh; chmod +x .startup.sh
```
8. On RPi-B, download the [satellite_startup.sh](satellite_startup.sh) file to `/home/pi`. Link `.startup.sh` to `satellite_startup.sh` and set `.startup.sh` to be executable.
```
% cd /home/pi; ln -s satellite_startup.sh .startup.sh; chmod +x .startup.sh
```
9. Restart both RPi-A and RPi-B.

## Testing and Diagnositics

1. Connect to RPi-A at 10.0.0.101 and login once it reboots and reconnects to WiFi.
2. The `ifconfig` command should show the serial interface created by `kissattach`
```
rpi-a% ifconfig ax0
```
3. The `route` command should show a route for `192.168.100.XXX` via `ax0`.
```
rpi-a% route -n
```
4. The `ping` command should indicate an IP connection with RPi-B.
```
rpi-a% ping -n 192.168.100.102
```
5. Attempt to connect to RPi-B at 10.0.0.102. If the startup script was successful, this will fail. If you are able to connect and login, see what is in the `/home/pi/.startup.sh` file. This may indicate what went wrong. Issue the commands in `.startup.sh` manually, with `sudo`, as needed. Use the diagnistics below to determine what isn't working right.
6. Try to connect to RPi-B from RPi-A using the `ssh` command. 
```
rpi-a% ssh 192.168.100.102
```
7. Check `ifconfig`
```
rpi-b% ifconfig ax0
```
8. Check `route` - should show a route for `192.168.100.XXX` via `ax0`, and a default route for all IP address via `192.168.100.101` (RPi-A).
```
rbi-b% route -n 
```
9. Ping RPi-A.
```
rpi-b% ping 192.168.100.101
```
10. Full internet connection test
```
rpi-b% wget -o /dev/null http://google.com
```
