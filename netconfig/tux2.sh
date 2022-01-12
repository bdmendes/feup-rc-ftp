#!/bin/bash

systemctl restart networking

# Preparatives
ip link set up eth0
ip addr flush dev eth0
ip addr add 172.16.11.1/24 dev eth0
