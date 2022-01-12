#!/bin/bash

systemctl restart networking

# Guide 1
ip link set up eth0
ip addr flush dev eth0
ip addr add 172.16.10.254/24 dev eth0
