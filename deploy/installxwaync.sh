
sudo chmod +x xwaync

sudo cp xwaync /opt/xwaync

sudo cp xwayncd.service /etc/systemd/system/xwayncd.service

sudo systemctl daemon-reload

sudo systemctl start xwayncd

sudo systemctl enable xwayncd