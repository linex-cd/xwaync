#!/bin/bash

sudo systemctl disable xwayncd

sudo systemctl stop xwayncd

sudo rm /opt/xwaync/xwaync
sudo rm /opt/xwaync/start_xwayncd.sh

sudo rm /etc/systemd/system/xwayncd.service

sudo systemctl daemon-reload


