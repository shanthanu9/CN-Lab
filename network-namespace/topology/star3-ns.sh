###############################
#
#                 n1(10.0.1.1/24)
#                 |
#                 |
#                 n0
#                / \
#               /   \
# (10.0.3.1/24)n3    n2(10.0.2.1/24)
# 
# n0, n1, n2 and n3 are namespaces
###############################

# Not asking parameters since network has got big

# Create namespaces
ip netns add n0
ip netns add n1
ip netns add n2
ip netns add n3

echo Added namespaces

# Create veth pairs
ip link add dev n1-n0 type veth peer name n0-n1
ip link add dev n2-n0 type veth peer name n0-n2
ip link add dev n3-n0 type veth peer name n0-n3

echo Added veth pairs

# Assign veth endpoints to namespaces
ip link set dev n1-n0 netns n1
ip link set dev n2-n0 netns n2
ip link set dev n3-n0 netns n3

ip link set dev n0-n1 netns n0
ip link set dev n0-n2 netns n0
ip link set dev n0-n3 netns n0

echo Assigned devices to namespaces

# Set devices up
ip netns exec n1 ip link set dev n1-n0 up
ip netns exec n2 ip link set dev n2-n0 up
ip netns exec n3 ip link set dev n3-n0 up

ip netns exec n0 ip link set dev n0-n1 up
ip netns exec n0 ip link set dev n0-n2 up
ip netns exec n0 ip link set dev n0-n3 up

echo Devices are set up

# Assign addresses to devices
ip netns exec n1 ip addr add 10.0.1.1/24 dev n1-n0
ip netns exec n2 ip addr add 10.0.2.1/24 dev n2-n0
ip netns exec n3 ip addr add 10.0.3.1/24 dev n3-n0

ip netns exec n0 ip addr add 10.0.1.2/24 dev n0-n1
ip netns exec n0 ip addr add 10.0.2.2/24 dev n0-n2
ip netns exec n0 ip addr add 10.0.3.2/24 dev n0-n3

echo Assigned addresses to devices

# Setup routing
ip netns exec n1 ip route add default via 10.0.1.2 dev n1-n0
ip netns exec n2 ip route add default via 10.0.2.2 dev n2-n0
ip netns exec n3 ip route add default via 10.0.3.2 dev n3-n0

ip netns exec n3 sysctl -w net.ipv4.ip_forward=1

echo Setup routing

# Run bash on n1
echo Running bash on n1

ip netns exec n1 bash

# Cleanup
ip netns exec n0 ip link del dev n0-n1
ip netns exec n0 ip link del dev n0-n2
ip netns exec n0 ip link del dev n0-n3
ip netns del n0
ip netns del n1
ip netns del n2
ip netns del n3