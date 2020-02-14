set -x
cd /home/pi
sudo apt install ax25-apps ax25-tools ntpsec
wget 'https://raw.githubusercontent.com/silver-sat/ipoverradio/master/ipoverserial/rc.local.insert.sh' | \
     sudo sed -i -e '/^fi$/,/^exit 0$/{//!d;};' -e '/^exit 0$/r /dev/stdin' /etc/rc.local
wget 'https://raw.githubusercontent.com/silver-sat/ipoverradio/master/ipoverserial/axports.append.txt' | \
     sudo sed -i -e '/^serial/d' -e '$ r /dev/stdin' /etc/ax25/axports
wget -O bridge_startup.sh 'https://raw.githubusercontent.com/silver-sat/ipoverradio/master/ipoverserial/bridge_startup.sh'
rm -f .startup.sh
ln -s bridge_startup.sh .startup.sh
chmod +x .startup.sh
