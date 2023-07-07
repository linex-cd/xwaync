#!/bin/bash

sudo chmod +x /opt/xwaync/xwaync
sudo chmod +x /opt/xwaync/start_xwayncd.sh

sudo cp /opt/xwaync/xwayncd.service /etc/systemd/system/xwayncd.service

sudo systemctl daemon-reload

sudo systemctl start xwayncd

sudo systemctl enable xwayncd