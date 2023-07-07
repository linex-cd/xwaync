#!/bin/bash

sudo echo 'KERNEL=="uinput", MODE="0666"' | sudo tee /etc/udev/rules.d/99-uinput2.rules

sudo service restart

#sudo reboot
