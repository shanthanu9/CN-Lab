#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/netanim-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

int main(int argc, char *args[])
{

    uint32_t spokes = 2;
    PointToPointHelper pointToPoint;
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    PointToPointStarHelper starHelper(spokes, pointToPoint);

    InternetStackHelper internetStack;
    starHelper.InstallStack(internetStack);

    Ipv4AddressHelper address("10.1.1.0", "255.255.255.0");
    starHelper.AssignIpv4Addresses(address);

    uint16_t port = 1025;
    Address hubAdddress(InetSocketAddress(Ipv4Address::GetAny(), port));
    // @TcpSocketFactory name of the protocol used to receive traffic and the type of socket factory used to create sockets for the app
    PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory", hubAdddress);
    ApplicationContainer hub = packetSinkHelper.Install(starHelper.GetHub());
    hub.Start(Seconds(1.0));
    hub.Start(Seconds(10.0));

    OnOffHelper onOffHelper("ns3::TcpSocketFactory", Address());
    onOffHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    onOffHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));

    ApplicationContainer spokeApps;
    for (uint32_t i = 0; i < starHelper.SpokeCount(); ++i)
    {
        AddressValue remoteAddress(InetSocketAddress(starHelper.GetHubIpv4Address(i), port));
        onOffHelper.SetAttribute("Remote", remoteAddress);
        spokeApps.Add(onOffHelper.Install(starHelper.GetSpokeNode(i)));
    }

    spokeApps.Start(Seconds(1.0));
    spokeApps.Stop(Seconds(10.0));

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
