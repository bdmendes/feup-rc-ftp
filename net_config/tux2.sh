#!/bin/bash

systemctl restart networking

# Preparatives
ip link set up eth0
ip addr flush dev eth0
ip addr add 172.16.11.1/24 dev eth0

# Guide 4
ip route add 172.16.10.0/24 via 172.16.11.253
ip -family inet neigh flush any
ip route add default via 172.16.11.254
