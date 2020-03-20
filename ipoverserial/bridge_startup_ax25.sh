
# Note, run as root from /etc/rc.local
set -x

# Commands at boot for bridge raspberry pi: Bridge is 192.168.100.101
# Connected raspberry pi is 192.168.100.102

kissattach /dev/serial0 serial 192.168.100.101

# Assumes serial port connection is via /dev/ax0 and
# available connection (to the real internet) is /dev/wlan0.

iptables -A FORWARD -i ax0 -j ACCEPT
iptables --flush -t nat
iptables -t nat -I POSTROUTING -o wlan0 -j MASQUERADE

echo 1 > /proc/sys/net/ipv4/ip_forward
