#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/mobility-module.h>
#include <ns3/lte-module.h>
#include <ns3/config-store.h>
#include "ns3/config-store-module.h"
#include <ns3/radio-bearer-stats-calculator.h>
#include <ns3/internet-stack-helper.h>
#include <ns3/point-to-point-helper.h>
#include "ns3/point-to-point-module.h"
#include <ns3/ipv4-static-routing.h>
#include <ns3/ipv4-static-routing-helper.h>
#include <ns3/packet-sink-helper.h>
#include <ns3/udp-client-server-helper.h>
#include <ns3/epc-helper.h>
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/data-collector.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/nstime.h"
#include <iomanip>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <ctime>

using namespace ns3;

const int RRH = 40;
const int vehicle = 200;
const int dim = 2;

long double string_to_double(std::string text){
  long double number = 0;
  int it = 0;
  int cyfra = 0;
  long double temp = 0;
  long double decimalcounter = 1;

  for(int i = 0; i < text.size(); i++){
    if(text[i] == '.'){
      break;
    }
    cyfra = int(text[i]) - 48;
    number *= 10;
    number += cyfra;
    it++;
  }
  it++;

  for(int i = it; i < text.size(); i++){
    decimalcounter /= 10;
    temp = int(text[i]) - 48;
    temp *= decimalcounter;
    number += temp;
  }

  return number;
}

int string_to_int(std::string text){
  int number = 0;
  int cyfra = 0;

  for(int i = 0; i < text.size(); i++){
    cyfra = int(text[i]) - 48;
    number *= 10;
    number += cyfra;
  }

  return number;
}

void enb_position_reading(long double enbPositionArray[][dim]){
  std::fstream file;
  file.open("rrh_coordinates_200_40.txt", std::ios::in);
  std::string tmp;

  for(int i = 0; i < RRH; i++){
    for(int j = 0; j < dim; j++){
      file >> tmp;
      enbPositionArray[i][j] = string_to_double(tmp);
    }
  }

  file.close();
}

void ue_position_reading(long double uePositionArray[][dim]){
  std::fstream file;
  file.open("vehicle_coordinates_200_40.txt", std::ios::in);
  std::string tmp;

  for(int i = 0; i < vehicle; i++){
    for(int j = 0; j < dim; j++){
      file >> tmp;
      uePositionArray[i][j] = string_to_double(tmp);
    }
  }

  file.close();
}

void required_RBs_reading(int requiredRBs[][vehicle]){
  std::fstream file;
  file.open("required_rbs_200_40.txt", std::ios::in);
  std::string tmp;

  for(int i = 0; i < RRH; i++){
    for(int j = 0; j < vehicle; j++){
      file >> tmp;
      requiredRBs[i][j] = string_to_int(tmp);
    }
  }

  file.close();
}

int main (int argc, char *argv[])
{   
  Time simTime = MilliSeconds(30000);            //time of simulation
  Time interPacketInterval = MilliSeconds(8.192);
  int ENBS_NUMBER = 40;                          //constant number of eNBs
  int updatedENBS_NUMBER = 0;                   //constant number of eNBs that will be switched on
  int UES_NUMBER = 200;                          //constant number of UEs
  bool RRHstate[ENBS_NUMBER];                   //array of flags to set 1 if RRH is on or 0 if RRH is off
  
  for(int i=0; i<ENBS_NUMBER; i++){
    RRHstate[i] = false;
  }
  int  vehToRRH[UES_NUMBER];                    //array of attachment decision (which vehicle to which RRH)

  //array with enb positions 
  long double enbPositionArray[RRH][dim];
  enb_position_reading(enbPositionArray);

  //array with ue positions 
  long double uePositionArray[vehicle][dim];
  ue_position_reading(uePositionArray);

  //alghorithm of turning on RRHs and attaching UEs to RRHs
  int requiredRBs[RRH][vehicle];
  required_RBs_reading(requiredRBs);

  //loop of making a decision
  for(int i=0; i<UES_NUMBER; i++) 
  {
    int RRHid = 0;
    for(int k=0; k<ENBS_NUMBER; k++){
      if(requiredRBs[i][k] == 0) {
        RRHid = k;
        break;
      }
    }
    RRHstate[RRHid] = true;
    vehToRRH[i] = RRHid;
  }

  //create array with eNBs that will be switched on
  std::vector<long double> updatedEnbPositionArray[2];
  int updatedRRH[ENBS_NUMBER];
  for(int i = 0; i < ENBS_NUMBER; i++){
    updatedRRH[i] = 9999999;
  }

  //putting values to updated array
  for(int i = 0; i < ENBS_NUMBER; i++){
    if(RRHstate[i] == true){
      updatedRRH[i] = updatedENBS_NUMBER;
      updatedEnbPositionArray[0].push_back(enbPositionArray[i][0]);
      updatedEnbPositionArray[1].push_back(enbPositionArray[i][1]);
      updatedENBS_NUMBER++;
    }
  }

  NS_LOG_UNCOND("RRHs turned on");

  // determine the string tag that identifies this simulation run
  // this tag is then appended to all filenames
 
  double enbDist = 20.0;
  double radius = 10.0;
  uint32_t numUes = 1;
  UintegerValue runValue;
  GlobalValue::GetValueByName ("RngRun", runValue);
 
  std::ostringstream tag;
  tag  << "_enbDist" << std::setw (3) << std::setfill ('0') << std::fixed << std::setprecision (0) << enbDist
     << "_radius"  << std::setw (3) << std::setfill ('0') << std::fixed << std::setprecision (0) << radius
     << "_numUes"  << std::setw (3) << std::setfill ('0')  << numUes
     << "_rngRun"  << std::setw (3) << std::setfill ('0')  << runValue.Get ();  
  
  LogComponentEnable("LteHelper", LOG_LEVEL_LOGIC);

  //creating LTE helper
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  
  Config::SetDefault ("ns3::LteUePowerControl::Pcmin", DoubleValue(23.0));
  Config::SetDefault ("ns3::LteUePowerControl::Pcmax", DoubleValue(23.0));
  lteHelper->SetEnbDeviceAttribute("UlBandwidth", UintegerValue(50));
  lteHelper->SetEnbDeviceAttribute("Mtu", UintegerValue(1500));  
  lteHelper->SetUeDeviceAttribute("Mtu", UintegerValue(1500));
  lteHelper->SetAttribute ("FadingModel", StringValue ("ns3::TraceFadingLossModel"));
   
  std::ifstream ifTraceFile;
  ifTraceFile.open ("../../src/lte/model/fading-traces/fading_trace_EPA_3kmph(30sec_50RBs).fad", std::ifstream::in);
  if (ifTraceFile.good ()){
      //script launched by test.py
      lteHelper->SetFadingModelAttribute ("TraceFilename", StringValue ("../../src/lte/model/fading-traces/fading_trace_EPA_3kmph(30sec_50RBs).fad"));
    }
   else{
      //script launched as an example
      lteHelper->SetFadingModelAttribute ("TraceFilename", StringValue ("src/lte/model/fading-traces/fading_trace_EPA_3kmph(30sec_50RBs).fad"));
   }
   
  // these parameters have to be set only in case of the trace format 
  // differs from the standard one, that is
  // - 10 seconds length trace
  // - 10,000 samples
  // - 0.5 seconds for window size
  // - 100 RB
  lteHelper->SetFadingModelAttribute ("TraceLength", TimeValue (Seconds (30.0)));
  lteHelper->SetFadingModelAttribute ("SamplesNum", UintegerValue (30000));
  lteHelper->SetFadingModelAttribute ("WindowSize", TimeValue (Seconds (0.5)));
  lteHelper->SetFadingModelAttribute ("RbNum", UintegerValue (50));

  //EPC helper
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> (); 
  
  //pointing epc helper for lte helper to enable cooperating
  lteHelper->SetEpcHelper (epcHelper);

  //create a remote host to provide simple service 
  Ptr<Node> pgw = epcHelper->GetPgwNode ();

  //create a single RemoteHost
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);

  //create internet stack helper
  InternetStackHelper internet;  
  internet.Install (remoteHostContainer);
  NS_LOG_UNCOND("RRHs turned on");

  //create the internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.001)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  //interface 0 is localhost, 1 is the p2p device
  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

  NS_LOG_UNCOND("Internet created");

  //nodes containers for eNBs and UEs
  NodeContainer enbNodes;
  enbNodes.Create (updatedENBS_NUMBER);
  NodeContainer ueNodes;
  ueNodes.Create (UES_NUMBER);

  //lists of devices positions
  Ptr<ListPositionAllocator> enbpositionAlloc = CreateObject<ListPositionAllocator> ();
  Ptr<ListPositionAllocator> uepositionAlloc = CreateObject<ListPositionAllocator> ();

  //putting values of coordinates to simulation position array
  for(int i=0; i<updatedENBS_NUMBER; i++){
    enbpositionAlloc -> Add(Vector (updatedEnbPositionArray[0][i] * 1000, updatedEnbPositionArray[1][i] * 1000, 0));
  }

  NS_LOG_UNCOND(updatedENBS_NUMBER);

  MobilityHelper enbMobility;
  enbMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  enbMobility.SetPositionAllocator (enbpositionAlloc);
  enbMobility.Install (enbNodes);

  for(int i=0; i<UES_NUMBER; i++){
    uepositionAlloc -> Add(Vector (uePositionArray[i][0] * 1000, uePositionArray[i][1] * 1000, 0));
  }

  MobilityHelper ueMobility;
  ueMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  ueMobility.SetPositionAllocator (uepositionAlloc);
  ueMobility.Install (ueNodes);

  //installing devices
  NetDeviceContainer enbDevs;
  NetDeviceContainer ueDevs;
  enbDevs = lteHelper->InstallEnbDevice (enbNodes);
  ueDevs = lteHelper->InstallUeDevice (ueNodes);
  
  NS_LOG_UNCOND("UEs and eNBs prepared");

  //install the IP stack on the UEs
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs));

  // Assign IP address to UEs, and install applications
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
    Ptr<Node> ueNode = ueNodes.Get (u);
    // Set the default gateway for the UE
    Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
    ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

  //attach UEs to eNBs
  for(int i=0; i<UES_NUMBER; i++){
    lteHelper->Attach (ueDevs.Get (i), enbDevs.Get (updatedRRH[vehToRRH[i]]));  //we take a vehicle and search for the RRH it is supposed to be attached to
  }

  uint16_t dlPort = 1100;
  uint16_t ulPort = 2000;
  uint16_t otherPort = 3000;
  ApplicationContainer clientApps;
  ApplicationContainer serverApps;
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
  {
      ++ulPort;
      PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
      serverApps.Add (ulPacketSinkHelper.Install (remoteHost));

      UdpClientHelper ulClient (remoteHostAddr, ulPort);
      ulClient.SetAttribute ("Interval", TimeValue (interPacketInterval));
      ulClient.SetAttribute ("MaxPackets", UintegerValue (1000000));
      clientApps.Add (ulClient.Install (ueNodes.Get(u)));
  }

  NS_LOG_UNCOND("Application installed");
  
  serverApps.Start (Seconds (0.01));
  clientApps.Start (Seconds (0.02));

  Ptr<FlowMonitor> flowMonitor;
  FlowMonitorHelper flowHelper;
  flowMonitor = flowHelper.InstallAll();

  clientApps.Stop(simTime);
  serverApps.Stop(simTime);
  Simulator::Stop(simTime);

  lteHelper->EnablePhyTraces ();
  lteHelper->EnableMacTraces ();
  lteHelper->EnableRlcTraces ();
  lteHelper->EnableUlPhyTraces ();
  lteHelper->EnableUlRxPhyTraces ();
  lteHelper->EnableUlTxPhyTraces ();
  
  //Insert RLC Performance Calculator
  std::string dlOutFname = "DlRlcStats";
  dlOutFname.append (tag.str ());
  std::string ulOutFname = "UlRlcStats";
  ulOutFname.append (tag.str ());
   
  Simulator::Run ();

  flowMonitor->SerializeToXmlFile("Results_200_40.xml", false, true);

  Simulator::Destroy ();
  return 0;
}
