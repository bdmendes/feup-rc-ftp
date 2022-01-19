#!/bin/bash

systemctl restart networking

# Guide 1
ip link set up eth0
ip addr flush dev eth0
ip addr add 172.16.10.254/24 dev eth0

# Guide 4
ip link set up eth1
ip addr flush dev eth1
ip addr add 172.16.11.253/24 dev eth1

echo 1 > /proc/sys/net/ipv4/ip_forward
echo 0 > /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts

ip addr show

ip route show

ip -family inet neigh flush any
