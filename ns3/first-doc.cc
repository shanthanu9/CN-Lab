/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");

int
main (int argc, char *argv[])
{
  // These are command line arguements passed to ns-3 program.
  // These are required because arguements passed to waf are 
  // not recognised to be part of the program.
  CommandLine cmd;
  cmd.Parse (argc, argv);
 
  // Sets time resolution to be displayed at runtime.
  // Below its set to nanoseconds
  Time::SetResolution (Time::NS);

  // Logging is done for ease for debugging.
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  // nodes below is the set of computers in the network
  // We use a NodeContainer helper class to easily define 
  // nodes in the network and let ns-3 handle the low level details.
  NodeContainer nodes;
  nodes.Create (2);

  // PointToPointHelper is used to describe the channel
  // between the two nodes. This includes 
  // 1. The physical channel
  // 2. The network devices on the computer
  PointToPointHelper pointToPoint;

  // We set channel attributes here 
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  // devices below are the set of network devices (like NIC)
  // to be installed in the nodes.
  // NetDeviceContainer is a high level helper to store these
  // devices.
  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes);

  // Now the nodes and connection in between them is
  // set up.
  // But we still need to add the internet stack to both
  // these nodes so that both the nodes can communicate.
  InternetStackHelper stack;
  stack.Install (nodes);

  // The following lines assigns addresses to nodes.
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = address.Assign (devices);

  // We now setup the echo application.
  // First we install the echo server 
  // application on node 1.
  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (nodes.Get (1));

  // The following lines dictate from when to when the server is active.
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  // Setting echo client program
  UdpEchoClientHelper echoClient (interfaces.GetAddress (1), 9);

  // Setting attributes of client application
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  // Installing echo client program in node 0
  ApplicationContainer clientApps = echoClient.Install (nodes.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  // Alright we have set up the network topology now.
  // Time to run the simulation!
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
