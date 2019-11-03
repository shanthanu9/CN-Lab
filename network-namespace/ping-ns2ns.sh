#########################
#
#  ns1  ---- ns2
#  addr1     addr2
#########################

ns1=$1
ns2=$2
addr1=$3
addr2=$4

# Add namespace
ip netns add $ns1
ip netns add $ns2

# Assign device to ns
ip link add dev eth0 type veth peer name eth1
ip link set dev eth0 netns $ns1
ip link set dev eth1 netns $ns2

# Set devices up
ip netns exec $ns1 ip link set eth0 up
ip netns exec $ns2 ip link set eth1 up

# Set addresses
ip netns exec $ns1 ip addr add $addr1 dev eth0
ip netns exec $ns2 ip addr add $addr2 dev eth1

# Run whatever in $ns1
ip netns exec $ns1 bash

# cleanup
ip netns exec $ns1 ip link del dev eth0
ip netns del $ns1
ip netns del $ns2
