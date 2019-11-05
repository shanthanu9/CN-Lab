# n0 -- r0 -- r1 -- n1

ip netns add n0
ip netns add n1
ip netns add r0
ip netns add r1

echo Addded namespaces and routers

ip link add n0-r0 type veth peer name r0-n0
ip link add r0-r1 type veth peer name r1-r0
ip link add r1-n1 type veth peer name n1-r1

echo Created veth pairs

ip link set n0-r0 netns n0
ip link set r0-n0 netns r0
ip link set r0-r1 netns r0
ip link set r1-r0 netns r1
ip link set r1-n1 netns r1
ip link set n1-r1 netns n1

echo Assigned veth endpoints to namespaces

ip netns exec n0 ip link set n0-r0 up
ip netns exec r0 ip link set r0-n0 up
ip netns exec r0 ip link set r0-r1 up
ip netns exec r1 ip link set r1-r0 up
ip netns exec r1 ip link set r1-n1 up
ip netns exec n1 ip link set n1-r1 up

echo Set veth devices up

ip netns exec n0 ip addr add 10.0.0.1/24 dev n0-r0
ip netns exec r0 ip addr add 10.0.0.2/24 dev r0-n0
ip netns exec r0 ip addr add 10.0.1.1/24 dev r0-r1
ip netns exec r1 ip addr add 10.0.1.2/24 dev r1-r0
ip netns exec r1 ip addr add 10.0.2.1/24 dev r1-n1
ip netns exec n1 ip addr add 10.0.2.2/24 dev n1-r1

echo Assigned ip address

ip netns exec n0 ip route add default via 10.0.0.2 dev n0-r0
ip netns exec n1 ip route add default via 10.0.2.1 dev n1-r1
ip netns exec r0 sysctl -w net.ipv4.ip_forward=1
ip netns exec r1 sysctl -w net.ipv4.ip_forward=1

ip netns exec r0 ip route add 10.0.2.2 via 10.0.1.2 dev r0-r1
ip netns exec r1 ip route add 10.0.0.1 via 10.0.1.1 dev r1-r0

echo Added routing

echo *****Running bash in b0*****

ip netns exec n0 bash

# Cleanup
ip netns exec n0 ip link del n0-r0
ip netns exec r0 ip link del r0-r1
ip netns exec r1 ip link del r1-n1

ip netns del n0
ip netns del n1
ip netns del r0
ip netns del r1
