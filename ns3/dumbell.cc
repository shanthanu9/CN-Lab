#include <iostream>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/netanim-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-layout-module.h"

using namespace ns3;

int main(int argc, char *argv[])
{

    int left_nodes = 1;
    int right_nodes = 1;
    std::string animFile = "dumbell.xml";

    PointToPointHelper pointToPointRouter;
    pointToPointRouter.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    pointToPointRouter.SetChannelAttribute("Delay", StringValue("1ms"));
    PointToPointHelper pointToPointLeaf;
    pointToPointLeaf.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    pointToPointLeaf.SetChannelAttribute("Delay", StringValue("1ms"));

    PointToPointDumbbellHelper dumbell(left_nodes, pointToPointLeaf, right_nodes, pointToPointLeaf, pointToPointRouter);

    InternetStackHelper stack;
    dumbell.InstallStack(stack);

    dumbell.AssignIpv4Addresses(Ipv4AddressHelper("10.1.1.0", "255.255.255.0"),
                                Ipv4AddressHelper("10.2.1.0", "255.255.255.0"),
                                Ipv4AddressHelper("10.3.1.0", "255.255.255.0"));

    OnOffHelper clientHelper("ns3::UdpSocketFactory", Address());
    clientHelper.SetAttribute("OnTime", StringValue ("ns3::UniformRandomVariable"));
    clientHelper.SetAttribute("OffTime", StringValue ("ns3::UniformRandomVariable"));
    ApplicationContainer clientApps;

    for (int i = 0; i < right_nodes; i++)
    {
        AddressValue remoteAddress(InetSocketAddress(dumbell.GetLeftIpv4Address(i), 1000));
        clientHelper.SetAttribute("Remote", remoteAddress);
        clientApps.Add(clientHelper.Install(dumbell.GetRight(i)));
    }

    clientApps.Start(Seconds(0.0));
    clientApps.Stop(Seconds(3.0));

    // Set the bounding box for animation
    dumbell.BoundingBox(1, 1, 100, 100);

    // Create the animation object and configure for specified output
    AnimationInterface anim(animFile);
    anim.EnablePacketMetadata();                                // Optional
    anim.EnableIpv4L3ProtocolCounters(Seconds(0), Seconds(10)); // Optional

    // Set up the acutal simulation
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    Simulator::Run();
    std::cout << "Animation Trace file created:" << animFile.c_str() << std::endl;
    Simulator::Destroy();

    return 0;
}
