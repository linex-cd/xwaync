

sudo systemctl disable xwayncd

sudo systemctl stop xwayncd

sudo rm /opt/xwaync
sudo rm /opt/start_xwayncd.sh

sudo rm /etc/systemd/system/xwayncd.service

sudo systemctl daemon-reload


