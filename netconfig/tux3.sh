#!/bin/bash

systemctl restart networking

# Guide 1
ip link set up eth0
ip addr flush dev eth0
ip addr add 172.16.10.1/24 dev eth0
ip -family inet neigh flush any

# Guide 4
ip route add 172.16.11.0/24 via 172.16.10.254
ip route show

ip -family inet neigh flush any

ip route add default via 172.16.10.254
