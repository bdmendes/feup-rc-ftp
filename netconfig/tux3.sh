#!/bin/bash

systemctl restart networking

# Guide 1
ip link set up eth0
ip addr flush dev eth0
ip addr add 172.16.10.1/24 dev eth0
ip -family inet neigh flush any
