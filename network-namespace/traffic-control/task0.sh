ip netns add n0
ip netns add n1

ip link add n0-n1 type veth peer name n1-n0

ip link set n0-n1 netns n0
ip link set n1-n0 netns n1

ip netns exec n0 ip link set n0-n1 up
ip netns exec n1 ip link set n1-n0 up

ip netns exec n0 ip addr add 10.0.0.1/24 dev n0-n1
ip netns exec n1 ip addr add 10.0.0.2/24 dev n1-n0

# Add bandwidth limit

# Uncomment any of the following commands
#ip netns exec n0 tc qdisc add dev n0-n1 root tbf rate 1mbit burst 32kbit latency 400ms
#ip netns exec n0 tc qdisc add dev n0-n1 root netem rate 1mbit
#ip netns exec n0 tc qdisc add dev n0-n1 netem delay 200ms

echo topo: n0 -- n1
echo n0: 10.0.0.1
echo n1: 10.0.0.2
echo *****bash running in n0*****
ip netns exec n0 bash

# To run iperf server
# iperf -s

# To run iperf client
# iperf -c ip

# Cleanup
ip netns exec n0 ip link del n0-n1
ip netns del n0
ip netns del n1
