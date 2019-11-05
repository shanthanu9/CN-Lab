###############################
#   ns1  ----  bri  ----  ns2
#   addr1                 addr2
###############################

ns1=$1
br0=$2
ns2=$3
addr1=$4
addr2=$5

# Create namespaces
ip netns add $ns1
ip netns add $ns2

# Create bridge
brctl addbr $br0

# Create veths
ip link add dev eth1 type veth peer name veth-ns1
ip link add dev eth2 type veth peer name veth-ns2

# Add veth interfaces to namespaces
ip link set dev eth1 netns $ns1
ip link set dev eth2 netns $ns2

# Add veth interfaces to bridge
brctl addif $br0 veth-ns1
brctl addif $br0 veth-ns2

# Set veth pair up
ip netns exec $ns1 ip link set dev eth1 up
ip netns exec $ns2 ip link set dev eth2 up
ip link set dev veth-ns1 up
ip link set dev veth-ns2 up

# Assign addresses to veth
ip netns exec $ns1 ip addr add $addr1 dev eth1
ip netns exec $ns2 ip addr add $addr2 dev eth2

# Set bridge device up
ip link set dev $br0 up

# Run whatever in ns1
echo In $ns1 namespace
ip netns exec $ns1 bash

# Cleanup
ip netns exec $ns1 ip link del dev eth1
ip netns exec $ns2 ip link del dev eth2
ip link set dev $br0 down
brctl delbr $br0
ip netns del $ns1
ip netns del $ns2
