// Network Topology
//
// n0------------n1------------n2
//   250Kbps,2ms    user-driven 
//               (bottleneck link)
//

#include <iostream>
#include <fstream>
#include <string>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/tcp-header.h"
#include "ns3/udp-header.h"
#include "ns3/enum.h"
#include "ns3/event-id.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/traffic-control-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TcpVariantsExample");

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

MyApp::~MyApp() { m_socket = 0; }

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
  if (m_sendEvent.IsRunning ()){
    Simulator::Cancel (m_sendEvent);
  }
  if (m_socket){
    m_socket->Close ();
  }
}

void MyApp::sendPkt (void) {
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  m_socket->Send (packet);
  if (++m_packetsSent < m_nPackets){
    scheduleTx ();
  }
}

void MyApp::scheduleTx (void){
  if (m_running){
    Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
    m_sendEvent = Simulator::Schedule (tNext, &MyApp::sendPkt, this);
  }
}

int main(int argc, char *argv[]){
    uint32_t mtu_bytes = 400;
    std::string transport_prot = "TcpNewReno";
    std::string bottleneck_bw = "100Kbps";
    std::string bottleneck_d = "2ms";
    uint32_t max_bytes = 10000;
    std::string queue_disc_type = "ns3::PfifoFastQueueDisc";
    uint16_t buff_size = 20; //2 MB buffer size 

    CommandLine cmd;
    cmd.AddValue("transport_prot","Transport protocol to use: TcpNewReno, "
                "TcpHybla, TcpHighSpeed, TcpHtcp, TcpVegas, TcpScalable, TcpVeno, "
                "TcpBic, TcpYeah, TcpIllinois, TcpWestwood, TcpWestwoodPlus, TcpLedbat, "
		        "TcpLp", transport_prot);
    cmd.AddValue("delay","Bottleneck Link Delay",bottleneck_d);
    cmd.AddValue("bandwidth","Bottleneck Link Bandwidth",bottleneck_bw);
    cmd.AddValue("mtu_bytes","Maximum transmission unit size (in bytes)",mtu_bytes);
    cmd.AddValue("max_bytes","Maximum bytes to be transferred",max_bytes);
    cmd.AddValue("queue_disc_type","Queue disc type for bottleneck link (e.g. ns3::CoDelQueueDisc)",queue_disc_type);
    cmd.AddValue("buff_size","Advertised buffer size (1<<buff_size)",buff_size);
    cmd.Parse(argc,argv);

    Config::SetDefault ("ns3::TcpSocket::RcvBufSize",UintegerValue(1 << buff_size));
    Config::SetDefault ("ns3::TcpSocket::SndBufSize",UintegerValue(1 << buff_size));
    transport_prot = std::string ("ns3::") + transport_prot;

    //Calculate the maximum transmission unit size
    Header* temp_header = new Ipv4Header ();
    uint32_t ip_header = temp_header->GetSerializedSize ();
    delete temp_header;
    temp_header = new TcpHeader ();
    uint32_t tcp_header = temp_header->GetSerializedSize ();
    delete temp_header;
    uint32_t tcp_adu_size = mtu_bytes - 20 - (ip_header + tcp_header);

    Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TypeId::LookupByName (transport_prot)));
    
    //Create nodes
    NodeContainer c;
    c.Create(3);

    //Create Node containers
    NodeContainer n0n1 = NodeContainer(c.Get(0),c.Get(1));
    NodeContainer n1n2 = NodeContainer(c.Get(1),c.Get(2));

    //Define Node link properties
    PointToPointHelper RegLink;
    RegLink.SetDeviceAttribute("DataRate",StringValue("250Kbps"));
    RegLink.SetChannelAttribute("Delay",StringValue("2ms"));

    NetDeviceContainer d0d1 = RegLink.Install(n0n1);

    PointToPointHelper BottleNeckLink;
    BottleNeckLink.SetDeviceAttribute("DataRate",StringValue(bottleneck_bw));
    BottleNeckLink.SetChannelAttribute("Delay",StringValue(bottleneck_d));

    NetDeviceContainer d1d2 = BottleNeckLink.Install(n1n2);

    //Install Internet stack
    InternetStackHelper stack;
    stack.Install(c);

    //Assign Addresses
    Ipv4AddressHelper ipv4;
    ipv4.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer regLinkInterface = ipv4.Assign(d0d1);

    TrafficControlHelper tchPfifo;
    tchPfifo.SetRootQueueDisc ("ns3::PfifoFastQueueDisc");
    // tchPfifo.SetQueueLimits("ns3::DynamicQueueLimits","HoldTime",StringValue("4ms"));

    TrafficControlHelper tchCoDel;
    tchCoDel.SetRootQueueDisc ("ns3::CoDelQueueDisc");
    // tchCoDel.SetQueueLimits("ns3::DynamicQueueLimits","HoldTime",StringValue("1ms"));

    // DataRate access_b ("250Kbps");
    // DataRate bottle_b (bottleneck_bw);
    // Time access_d ("2ms");
    // Time bottle_d (bottleneck_d);

    // uint32_t size = static_cast <uint32_t> ((std::min(access_b, bottle_b).GetBitRate()/8)*((access_d+bottle_d)*2).GetSeconds ());

    // Config::SetDefault ("ns3::PfifoFastQueueDisc::MaxSize", QueueSizeValue (QueueSize (QueueSizeUnit::PACKETS, size / mtu_bytes)));
    // Config::SetDefault ("ns3::CoDelQueueDisc::MaxSize", QueueSizeValue (QueueSize (QueueSizeUnit::BYTES, size)));

    //Install queue managements for bottleneck link
    if (queue_disc_type.compare ("ns3::PfifoFastQueueDisc") == 0){
        tchPfifo.Install (d1d2);
    } else if (queue_disc_type.compare ("ns3::CoDelQueueDisc") == 0){
        tchCoDel.Install (d1d2);
    } else {
        NS_FATAL_ERROR ("Queue not recognized. Allowed values are ns3::CoDelQueueDisc or ns3::PfifoFastQueueDisc");
    }
    
    ipv4.SetBase ("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer bottleNeckInterface = ipv4.Assign(d1d2);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    //Establishing TCP Connection from n0 to n2
    
    //Sink Application
    uint16_t sinkPort = 8080;
    Address sinkAddress (InetSocketAddress (bottleNeckInterface.GetAddress (1), sinkPort)); // interface of n2
    PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
    ApplicationContainer sinkApps = packetSinkHelper.Install (c.Get (2)); //n2 as sink
    sinkApps.Start (Seconds (0.0));
    sinkApps.Stop (Seconds (30.0));

    //My Application
    Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (tcp_adu_size));
    Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (c.Get (0), TcpSocketFactory::GetTypeId ()); //source at n0
    Ptr<MyApp> app = CreateObject<MyApp> ();
    app->setup (ns3TcpSocket, sinkAddress, 1040, max_bytes, DataRate ("250Kbps"));
    c.Get (0)->AddApplication (app);
    app->SetStartTime (Seconds (1.0));
    app->SetStopTime (Seconds (30.0));

    Simulator::Stop (Seconds(35.0));
    Simulator::Run ();
    Simulator::Destroy ();
}