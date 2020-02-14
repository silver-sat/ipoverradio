set -x
cd /home/pi
sudo apt install ax25-apps ax25-tools ntpsec
sudo sed -i -e '/^fi$/,/^exit 0$/{/^fi$/!d;};' /etc/rc.local
sudo wget -q -O - 'https://raw.githubusercontent.com/silver-sat/ipoverradio/master/ipoverserial/rc.local.insert.sh' >> /etc/rc.local
sudo echo "exit 0" >> /etc/rc.local
sudo sed -i -e '/^serial/d' /etc/ax25/axports
sudo wget -q -O - 'https://raw.githubusercontent.com/silver-sat/ipoverradio/master/ipoverserial/axports.append.txt' >> /etc/ax25/axports
wget -O bridge_startup.sh 'https://raw.githubusercontent.com/silver-sat/ipoverradio/master/ipoverserial/bridge_startup.sh'
rm -f .startup.sh
ln -s bridge_startup.sh .startup.sh
chmod +x .startup.sh
