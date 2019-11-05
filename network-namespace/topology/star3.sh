###############################
#           n1
#           |
#           |
#           B
#          / \
#         /   \
#        n3    n2
# n1, n2 and n3 are namespaces
# B is a linux bridge
###############################

ns1=$1
ns2=$2
ns3=$3
br0=$4
a1=$5
a2=$6
a3=$7

# Create namespaces
ip netns add $ns1
ip netns add $ns2
ip netns add $ns3

# Create bridge
brctl addbr $br0

echo Added ns and bridge

# Create veths
ip link add dev eth1 type veth peer name veth-eth1
ip link add dev eth2 type veth peer name veth-eth2
ip link add dev eth3 type veth peer name veth-eth3

echo Created veth

# Add veth interfaces to namespaces and bridge
ip link set dev eth1 netns $ns1
ip link set dev eth2 netns $ns2
ip link set dev eth3 netns $ns3
brctl addif $br0 veth-eth1
brctl addif $br0 veth-eth2
brctl addif $br0 veth-eth3

echo Added veth to ns and bridge

# Set veth pair up
ip netns exec $ns1 ip link set dev eth1 up
ip netns exec $ns2 ip link set dev eth2 up
ip netns exec $ns3 ip link set dev eth3 up
ip link set dev veth-eth1 up
ip link set dev veth-eth2 up
ip link set dev veth-eth3 up

echo Set veth up

# Assign addresses to veth (assuming subnet of size 24)
ip netns exec $ns1 ip addr add $a1/24 dev eth1
ip netns exec $ns2 ip addr add $a2/24 dev eth2
ip netns exec $ns3 ip addr add $a3/24 dev eth3

echo Assign addresses

# Set bridge up
ip link set dev $br0 up

# Run whatever in ns1
echo In $ns1 namespace
ip netns exec $ns1 bash

# Cleanup
ip netns exec $ns1 ip link del dev eth1
ip netns exec $ns2 ip link del dev eth2
ip netns exec $ns3 ip link del dev eth3
ip netns del $ns1
ip netns del $ns2
ip netns del $ns3
ip link set dev $br0 down
brctl delbr $br0

echo Cleaned up stuff
