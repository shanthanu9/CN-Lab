################################
#     ___________________
#    |                   |
#    |                   |
#   br0 ----- br1 ----- br2
#    |         |         | 
# (1)|      (5)|      (9)|
#    n0        n1        n2 
#
#   (i) : 10.0.0.i/24
#
#   br0, br1 and br2 are bridges
#   n0, n1 and n2 are namespaces
#
################################

# Note: A loop is created hence ping wont't work! 

ip netns add n0
ip netns add n1
ip netns add n2

ip link add name br0 type bridge
ip link add name br1 type bridge
ip link add name br2 type bridge

ip link set dev br0 up
ip link set dev br1 up
ip link set dev br2 up

echo Added namespaces and bridges

ip link add n0-br0 type veth peer name br0-n0
ip link add n1-br1 type veth peer name br1-n1
ip link add n2-br2 type veth peer name br2-n2
ip link add br0-br1 type veth peer name br1-br0
ip link add br1-br2 type veth peer name br2-br1
ip link add br2-br0 type veth peer name br0-br2

echo Created veth pairs

ip link set n0-br0 netns n0
ip link set n1-br1 netns n1
ip link set n2-br2 netns n2
ip link set br0-n0 master br0
ip link set br1-n1 master br1
ip link set br2-n2 master br2
ip link set br0-br1 master br0
ip link set br1-br0 master br1
ip link set br1-br2 master br1
ip link set br2-br1 master br2
# Comment the below 2 lines for ping to work
ip link set br2-br0 master br2
ip link set br0-br2 master br0

echo Assigned veth endpoints to namespaces and bridges

ip netns exec n0 ip link set dev n0-br0 up
ip netns exec n1 ip link set dev n1-br1 up
ip netns exec n2 ip link set dev n2-br2 up
ip link set dev br0-n0 up
ip link set dev br1-n1 up
ip link set dev br2-n2 up
ip link set dev br0-br1 up
ip link set dev br1-br0 up
ip link set dev br1-br2 up
ip link set dev br2-br1 up
ip link set dev br2-br0 up
ip link set dev br0-br2 up

echo Set devices up

ip netns exec n0 ip addr add 10.0.0.1/24 dev n0-br0
ip netns exec n1 ip addr add 10.0.0.5/24 dev n1-br1
ip netns exec n2 ip addr add 10.0.0.9/24 dev n2-br2

echo Addresses assigned

echo *****Running bash in n0*****
echo n0: 10.0.0.1
echo n1: 10.0.0.5
echo n2: 10.0.0.9

ip netns exec n0 bash

# Cleanup

ip netns exec n0 ip link del n0-br0
ip netns exec n1 ip link del n1-br1
ip netns exec n2 ip link del n2-br2
ip link del br0-br1
ip link del br1-br2
ip link del br2-br0
ip link del br0
ip link del br1
ip link del dev br2
ip netns del n0
ip netns del n1
ip netns del n2
echo Cleaned up stuff
