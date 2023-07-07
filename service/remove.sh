#!/bin/bash

sudo systemctl disable xwayncd

sudo systemctl stop xwayncd


sudo rm /etc/systemd/system/xwayncd.service

sudo systemctl daemon-reload


