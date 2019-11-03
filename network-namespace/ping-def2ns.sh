#
# ns ----------- root
# addr1          addr2

ns=$1
addr1=$2
addr2=$3

# Create namespace
ip netns add $ns

# Create veth
ip link add dev eth0 type veth peer name eth1
ip link set dev eth1 netns $ns

# Set veth pair up
ip link set eth0 up
ip netns exec $ns ip link set eth1 up

# Add addresses to veth
ip netns exec $ns ip addr add $2 dev eth1
ip addr add $3 dev eth0

# Run whatever command in namespace
bash

# Delete namespace and veth
ip netns exec $ns ip link del eth1
ip netns del $ns
