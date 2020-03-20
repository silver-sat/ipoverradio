set -x
cd /home/pi

# install ax25 tools
sudo apt install ax25-apps ax25-tools ntpsec

# modify /etc/rc.local
sed -e '/^fi$/,/^exit 0$/d' /etc/rc.local > /tmp/rc.local
echo "fi" >> /tmp/rc.local
wget -q -O - 'https://raw.githubusercontent.com/silver-sat/ipoverradio/master/ipoverserial/rc.local.insert.sh' >> /tmp/rc.local
echo "exit 0" >> /tmp/rc.local
sudo mv -f /tmp/rc.local /etc/rc.local
sudo chown root.root /etc/rc.local
sudo chmod +x /etc/rc.local

# motify /etc/ax25/axports
sed -e '/^serial/d' /etc/ax25/axports > /tmp/axports
wget -q -O - 'https://raw.githubusercontent.com/silver-sat/ipoverradio/master/ipoverserial/axports.append.txt' >> /tmp/axports
sudo mv -f /tmp/axports /etc/ax25/axports
sudo chown root.root /etc/ax25/axports

# Download and place appropriate startup script
wget -O satellite_startup_ax25.sh 'https://raw.githubusercontent.com/silver-sat/ipoverradio/master/ipoverserial/satellite_startup_ax25.sh'
wget -O satellite_startup_ppp.sh 'https://raw.githubusercontent.com/silver-sat/ipoverradio/master/ipoverserial/satellite_startup_ppp.sh'
rm -f .startup.sh
ln -s satellite_startup_ax25.sh .startup.sh
chmod +x .startup.sh
