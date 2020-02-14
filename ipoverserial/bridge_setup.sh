set -x
cd /home/pi
sudo apt install ax25-apps ax25-tools ntpsec
sed -e '/^fi$/,/^exit 0$/d' /etc/rc.local > /tmp/rc.local
echo "fi" >> /tmp/rc.local
wget -q -O - 'https://raw.githubusercontent.com/silver-sat/ipoverradio/master/ipoverserial/rc.local.insert.sh' >> /tmp/rc.local
echo "exit 0" >> /tmp/rc.local
sudo mv -f /tmp/rc.local /etc/rc.local
sudo chmod +x /etc/rc.local
sed -e '/^serial/d' /etc/ax25/axports > /tmp/axports
wget -q -O - 'https://raw.githubusercontent.com/silver-sat/ipoverradio/master/ipoverserial/axports.append.txt' >> /tmp/axports
sudo mv -f /tmp/axports /etc/ax25/axports
wget -O bridge_startup.sh 'https://raw.githubusercontent.com/silver-sat/ipoverradio/master/ipoverserial/bridge_startup.sh'
rm -f .startup.sh
ln -s bridge_startup.sh .startup.sh
chmod +x .startup.sh
