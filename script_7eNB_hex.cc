#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/mobility-module.h>
#include <ns3/lte-module.h>

using namespace ns3;

int main (int argc, char *argv[])
{
    int ENBS_NUMBER = 7;
    int UES_NUMBER = 2;
    std::vector<long double> EnbPositionArray[2];
    Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();

    //te 14 linijek niżej to jest hex o boku 4 otoczony hexami o boku 4, współrzędne z zaokrągleniem do 2 miejsc po przecinaku
    EnbPositionArray[0].push_back(0.0);
    EnbPositionArray[1].push_back(0.0);
    EnbPositionArray[0].push_back(0.0);
    EnbPositionArray[1].push_back(6.93);
    EnbPositionArray[0].push_back(6.0);
    EnbPositionArray[1].push_back(3.46);
    EnbPositionArray[0].push_back(6.0);
    EnbPositionArray[1].push_back(-3.46);
    EnbPositionArray[0].push_back(0.0);
    EnbPositionArray[1].push_back(-6.93);
    EnbPositionArray[0].push_back(-6.0);
    EnbPositionArray[1].push_back(-3.46);
    EnbPositionArray[0].push_back(-6.0);
    EnbPositionArray[1].push_back(3.46);

    NodeContainer enbNodes;
    enbNodes.Create (ENBS_NUMBER);
    NodeContainer ueNodes;
    ueNodes.Create (UES_NUMBER);

    Ptr<ListPositionAllocator> enbpositionAlloc = CreateObject<ListPositionAllocator> ();
    for(int i=0; i<ENBS_NUMBER; i++){
        enbpositionAlloc -> Add(Vector (EnbPositionArray[0][i] * 1000, EnbPositionArray[1][i] * 1000, 0));
    }
    
    MobilityHelper enbMobility;
    MobilityHelper ueMobility;
    enbMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    enbMobility.SetPositionAllocator ()
    enbMobility.Install (enbNodes);
    ueMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    ueMobility.Install (ueNodes);

    NetDeviceContainer enbDevs;
    enbDevs = lteHelper->InstallEnbDevice (enbNodes);

    NetDeviceContainer ueDevs;
    ueDevs = lteHelper->InstallUeDevice (ueNodes);

    lteHelper->Attach (ueDevs, enbDevs.Get (0));

    enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
    EpsBearer bearer (q);
    lteHelper->ActivateDataRadioBearer (ueDevs, bearer);

    Simulator::Stop (Seconds (0.005));

    Simulator::Run ();

    Simulator::Destroy ();
    return 0
}