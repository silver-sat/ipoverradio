set -x
cd /home/pi
wget -O bridge_startup.sh 'https://raw.githubusercontent.com/silver-sat/ipoverradio/master/ipoverserial/bridge_startup.sh'
rm -f .startup.sh
ln -s bridge_startup.sh .startup.sh
chmod +x .startup.sh
