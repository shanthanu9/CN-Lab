// Give option for TCP Variant
// Network Topology
//
//       n0 ---+      +--- n5
//             |      |
//       n1 ---n3 -- n4--- n6
//             |      |
//       n2 ---+      +--- n7

//
// - All links are P2P with default 500kb/s and 2ms
// - TCP flow form n0 to n5, n1 to n6
// - UDP flow from n2 to n7

#include <fstream>
#include <string>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/ipv4-global-routing-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Dumbell");

//Create a custom application
class MyApp : public Application{
    public: 
        MyApp();
        virtual ~MyApp();
        void setup(Ptr<Socket> skt, Address address, uint32_t pktSize, uint32_t npkt, DataRate dr);
        
    private:
        virtual void StartApplication(void);
        virtual void StopApplication(void);

        virtual void scheduleTx(void);
        virtual void sendPkt(void);

        Ptr<Socket>     m_socket;
        Address         m_peer;
        uint32_t        m_packetSize;
        uint32_t        m_nPackets;
        DataRate        m_dataRate;
        EventId         m_sendEvent;
        bool            m_running;
        uint32_t        m_packetsSent;
};

MyApp::MyApp():m_socket (0),m_peer (), m_packetSize (0), m_nPackets (0), m_dataRate (0), m_sendEvent (), m_running (false), m_packetsSent (0) {}

MyApp::~MyApp() {
  m_socket = 0;
}

void MyApp::setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate) {
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;
}

void MyApp::StartApplication (void) {
  m_running = true;
  m_packetsSent = 0;
  m_socket->Bind ();
  m_socket->Connect (m_peer);
  sendPkt ();
}

void MyApp::StopApplication (void) {
  m_running = false;

  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }

  if (m_socket)
    {
      m_socket->Close ();
    }
}

void MyApp::sendPkt (void) {
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  m_socket->Send (packet);

  if (++m_packetsSent < m_nPackets)
    {
      scheduleTx ();
    }
}

void MyApp::scheduleTx (void){
  if (m_running)
    {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &MyApp::sendPkt, this);
    }
}

// static void CwndChange (uint32_t oldCwnd, uint32_t newCwnd){
//   std::cout << Simulator::Now ().GetSeconds () << "\t" << newCwnd <<"\n";
// }

int main (int argc, char *argv[])
{
  std::string lat = "2ms";
  std::string rate = "500kb/s"; // P2P link
  bool enableFlowMonitor = false;


  CommandLine cmd;
  cmd.AddValue ("latency", "P2P link Latency in miliseconds", lat);
  cmd.AddValue ("rate", "P2P data rate in bps", rate);
  cmd.AddValue ("EnableMonitor", "Enable Flow Monitor", enableFlowMonitor);

  cmd.Parse (argc, argv);

//
// Explicitly create the nodes required by the topology (shown above).
//
  NS_LOG_INFO ("Create nodes.");
  NodeContainer c; // ALL Nodes
  c.Create(8);

  NodeContainer n0n3 = NodeContainer (c.Get (0), c.Get (3));
  NodeContainer n1n3 = NodeContainer (c.Get (1), c.Get (3));
  NodeContainer n2n3 = NodeContainer (c.Get (2), c.Get (3));

  NodeContainer n3n4 = NodeContainer (c.Get(3),c.Get(4));

  NodeContainer n5n4 = NodeContainer (c.Get (5), c.Get (4));
  NodeContainer n6n4 = NodeContainer (c.Get (6), c.Get (4));
  NodeContainer n7n4 = NodeContainer (c.Get (7), c.Get (4));

//
// Install Internet Stack
//
  InternetStackHelper internet;
  internet.Install (c);

  // We create the channels first without any IP addressing information
  NS_LOG_INFO ("Create channels.");
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue (rate));
  p2p.SetChannelAttribute ("Delay", StringValue (lat));
  NetDeviceContainer d0d3 = p2p.Install (n0n3);
  NetDeviceContainer d1d3 = p2p.Install (n1n3);
  NetDeviceContainer d2d3 = p2p.Install (n2n3);

  NetDeviceContainer d5d4 = p2p.Install (n5n4);
  NetDeviceContainer d6d4 = p2p.Install (n6n4);
  NetDeviceContainer d7d4 = p2p.Install (n7n4);
 
  //Add cmd line arguements for this link only, rest links are defined before 
  p2p.SetDeviceAttribute("DataRate",StringValue("10Mbps"));
  p2p.SetChannelAttribute("Delay",StringValue("1us"));

  NetDeviceContainer d3d4 = p2p.Install (n3n4);
    // Later, we add IP addresses.
  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i0i3 = ipv4.Assign (d0d3);

  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i1i3 = ipv4.Assign (d1d3);

  ipv4.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer i2i3 = ipv4.Assign (d2d3);

  ipv4.SetBase ("10.1.4.0", "255.255.255.0");
  Ipv4InterfaceContainer i3i4 = ipv4.Assign (d3d4);

  ipv4.SetBase ("10.1.5.0", "255.255.255.0");
  Ipv4InterfaceContainer i5i4 = ipv4.Assign (d5d4);

  ipv4.SetBase ("10.1.6.0", "255.255.255.0");
  Ipv4InterfaceContainer i6i4 = ipv4.Assign (d6d4);

  ipv4.SetBase ("10.1.7.0", "255.255.255.0");
  Ipv4InterfaceContainer i7i4 = ipv4.Assign (d7d4);

  NS_LOG_INFO ("Enable static global routing.");
  //
  // Turn on global static routing so we can actually be routed across the network.
  //
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();


  NS_LOG_INFO ("Create Applications.");

  // TCP connfection from N0 to N5

  uint16_t sinkPort = 8080;
  Address sinkAddress (InetSocketAddress (i5i4.GetAddress (0), sinkPort)); // interface of n5
  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
  ApplicationContainer sinkApps = packetSinkHelper.Install (c.Get (5)); //n5 as sink
  sinkApps.Start (Seconds (0.0));
  sinkApps.Stop (Seconds (30.0));

  Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (c.Get (0), TcpSocketFactory::GetTypeId ()); //source at n0

//   // Trace Congestion window
//   ns3TcpSocket->TraceConnectWithoutContext ("CongestionWindow", MakeCallback(&CwndChange));

  // Create TCP application at n0
  Ptr<MyApp> app = CreateObject<MyApp> ();
  app->setup (ns3TcpSocket, sinkAddress, 1040, 100000, DataRate (rate));
  c.Get (0)->AddApplication (app);
  app->SetStartTime (Seconds (1.0));
  app->SetStopTime (Seconds (30.0));

  // TCP connfection from N1 to N6
  uint16_t sinkPort2 = 10000;
  Address sinkAddress2 (InetSocketAddress (i6i4.GetAddress (0), sinkPort2)); // interface of n6
  PacketSinkHelper packetSinkHelper2 ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort2));
  ApplicationContainer sinkApps2 = packetSinkHelper2.Install (c.Get (6)); //n6 as sink
  sinkApps2.Start (Seconds (0.0));
  sinkApps2.Stop (Seconds (30.0));

  Ptr<Socket> ns3TcpSocket1 = Socket::CreateSocket (c.Get (1), TcpSocketFactory::GetTypeId ()); //source at n2

  // Create TCP application at n1
  Ptr<MyApp> app2 = CreateObject<MyApp> ();
  app2->setup (ns3TcpSocket1, sinkAddress2, 1040, 100000, DataRate (rate));
  c.Get (1)->AddApplication (app2);
  app2->SetStartTime (Seconds (6.0));
  app2->SetStopTime (Seconds (30.0));


  // UDP connfection from N2 to N7

  uint16_t sinkPort3 = 6000;
  Address sinkAddress3 (InetSocketAddress (i7i4.GetAddress (0), sinkPort3)); // interface of n7
  PacketSinkHelper packetSinkHelper3 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort3));
  ApplicationContainer sinkApps3 = packetSinkHelper3.Install (c.Get (7)); //n7 as sink
  sinkApps3.Start (Seconds (0.0));
  sinkApps3.Stop (Seconds (30.0));

  Ptr<Socket> ns3UdpSocket = Socket::CreateSocket (c.Get (2), UdpSocketFactory::GetTypeId ()); //source at n1

  // Create UDP application at n1
  Ptr<MyApp> app3 = CreateObject<MyApp> ();
  app3->setup (ns3UdpSocket, sinkAddress3, 1040, 100000, DataRate (rate));
  c.Get (2)->AddApplication (app3);
  app3->SetStartTime (Seconds (5.0));
  app3->SetStopTime (Seconds (30.0));

  // Flow Monitor
  Ptr<FlowMonitor> flowmon;
  if (enableFlowMonitor)
    {
      FlowMonitorHelper flowmonHelper;
      flowmon = flowmonHelper.InstallAll ();
    }

//
// Now, do the actual simulation.
//
  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop (Seconds(100.0));
  Simulator::Run ();
  if (enableFlowMonitor)
    {
	  flowmon->CheckForLostPackets ();
	  flowmon->SerializeToXmlFile("lab-2.flowmon", true, true);
    }
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}