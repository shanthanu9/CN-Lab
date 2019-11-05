###############################
#   ns1  ----  ns2  ----  ns2
#   a1        a2 a3       a4
###############################

ns1=$1
ns2=$2
ns3=$3
a1=$4
a2=$5
a3=$6
a4=$7

# Create namespaces
ip netns add $ns1
ip netns add $ns2
ip netns add $ns3

# Create veths
ip link add dev $ns1-$ns2 type veth peer name $ns2-$ns1
ip link add dev $ns2-$ns3 type veth peer name $ns3-$ns2

# Add veth interfaces to namespaces
ip link set dev $ns1-$ns2 netns $ns1
ip link set dev $ns2-$ns1 netns $ns2
ip link set dev $ns2-$ns3 netns $ns2
ip link set dev $ns3-$ns2 netns $ns3

# Set veth pair up
ip netns exec $ns1 ip link set dev $ns1-$ns2 up
ip netns exec $ns2 ip link set dev $ns2-$ns1 up
ip netns exec $ns2 ip link set dev $ns2-$ns3 up
ip netns exec $ns3 ip link set dev $ns3-$ns2 up

# Assign addresses to veth (assuming subnet of size 24)
ip netns exec $ns1 ip addr add $a1/24 dev $ns1-$ns2
ip netns exec $ns2 ip addr add $a2/24 dev $ns2-$ns1
ip netns exec $ns2 ip addr add $a3/24 dev $ns2-$ns3
ip netns exec $ns3 ip addr add $a4/24 dev $ns3-$ns2

# Setup routing
ip netns exec $ns1 ip route add default via $a2 dev $ns1-$ns2
ip netns exec $ns3 ip route add default via $a3 dev $ns3-$ns2
ip netns exec $ns2 sysctl -w net.ipv4.ip_forward=1

# Run whatever in ns1
echo In $ns1 namespace
ip netns exec $ns1 bash

# Cleanup
ip netns exec $ns1 ip link del dev $ns1-$ns2
ip netns exec $ns3 ip link del dev $ns3-$ns2
ip netns del $ns1
ip netns del $ns2
ip netns del $ns3
