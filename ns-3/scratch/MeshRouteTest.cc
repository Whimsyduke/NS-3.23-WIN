/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
* Copyright (c) 2009 IITP RAS
*
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
*
* Author: Whimsy Duke <whimsyduke@163.com>
*/

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"
#include "ns3/pmtmgmp-module.h"
#include "ns3/mesh-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/stats-module.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <ctime>

//������ļ�
//#define OUT_TO_FILE 
//��Ӧ�ò�ģʽ
#define NO_APPLICATION
//����ģʽ
#define TEST_LOCATION


//�Ƕ�ת��
#define DEGREES_TO_RADIANS(__ANGLE__) ((__ANGLE__) * 0.01745329252f) // PI / 180
#define MIN_APPLICATION_TIME 120 // ����ǰ·������ʱ��
#define END_APPLICATION_TIME 15 // �����ȴ�ʱ��

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE("MeshRouteTest");

class MeshRouteClass
{
public:
	// ��ʼ������
	MeshRouteClass();
	// ���ò���
	void Configure(int argc, char ** argv);
	// ��ʼ����
	int Run();

private:
	// ��������
	void SimulatorSetting();
	// �����ڵ㼰���Э������
	void CreateNodes();
	// �ڵ�λ������
	void LocateNodes();
	// ��װ����Э��ջ
	void InstallInternetStack();
	// ��װӦ�ò�
	void InstallApplicationRandom();
	// ����ͳ��ģ������
	void StatsDataSet();
	// �����������
	void FlowMonitorReport();

private:
	// �ص�����
	void CallBack_RouteDiscoveryTime(string context, Time time);
	void CallBack_ApplicationTX(string context, Ptr<const Packet> packet);
	// �������
	void Report();

public:
	//�����Ͷ���
	enum MeshProtocolType// Mesh·��Э������
	{
		DOT11S_HWMP,// HWMP·��Э��
		MY11S_PMTMGMP,// PMTMGMP·��Э��
	};

	enum NodeAreaType// �ڵ�������״����
	{
		SQUARE,// ������
		HEXAGON,// ��������
	};

private:
	// ���������ò���
	MeshProtocolType m_ProtocolType;
	NodeAreaType m_AreaType;
	int m_Size;
	double m_RandomStart;
	uint32_t m_NumIface;
	WifiPhyStandard m_WifiPhyStandard;
	double m_Step;
	uint16_t m_ApplicationNum;
	uint32_t  m_MaxBytes;
	int m_SourceNum;
	int m_DestinationNum;
	uint16_t  m_PacketSize;
	string m_DataRate;
	double m_TotalTime;
	string m_Root;
	bool m_Pcap;

private:
	// �ڵ�����
	int l_NodeNum;
	// �ڵ�����
	NodeContainer l_Nodes;
	// �����豸����
	NetDeviceContainer l_NetDevices;
	MeshHelper l_Mesh;
	WmnHelper l_Pmtmgmp;
	// �ӿڵ�ַ:
	Ipv4InterfaceContainer l_Interfaces;
	// ����ͳ��
	FlowMonitorHelper l_Flowmon;
	Ptr<FlowMonitor> l_Monitor;
};

// ��ʼ������
MeshRouteClass::MeshRouteClass() :
m_ProtocolType(MY11S_PMTMGMP),
m_AreaType(SQUARE),
m_Size(4),
m_RandomStart(0.1),
m_NumIface(1),
m_WifiPhyStandard(WIFI_PHY_STANDARD_80211a),
m_Step(70),
m_ApplicationNum(0),
m_MaxBytes(0),
m_SourceNum(0),
m_DestinationNum(0),
m_PacketSize(1024),
m_DataRate("150kbps"),
m_TotalTime(50),
m_Root("00:00:00:00:00:06"),
m_Pcap(false)
{
	// ��·�㼰�����Э������
	l_Mesh = MeshHelper::Default();
	l_Pmtmgmp = WmnHelper::Default();
}

// ���ò���
void MeshRouteClass::Configure(int argc, char ** argv)
{
	// ����������
	CommandLine cmd;

	// �����в���
	int protocolType = (int)m_ProtocolType;
	int areaType = (int)m_AreaType;
	int wifiPhyStandard = (int)m_WifiPhyStandard;
	cmd.AddValue("ProtocolType", "Mesh·��Э�����͡���ѡֵ��0.DOT11S_HWMP���·��Э�飻1.MY11S_PMTMGMP���������������ض�·��·��Э�飻", protocolType);
	cmd.AddValue("AreaType", "�ڵ��������͡���ѡֵ��0.SQUARE�������Σ�1.HEXAGON���������Σ�", areaType);
	cmd.AddValue("Size", "�ڵ������С�Ĳ����������ڵ���������Ϊ�����Σ���Size����߳����ڵ�����Ϊ��Size * Size����Ϊ�����Σ���Size����߳����ڵ�����Ϊ��3 * (Size - 1) * Size��", m_Size);
	cmd.AddValue("RandomStartTime", "�����������ӳ�ʱ�䡣", m_RandomStart);
	cmd.AddValue("NumIface", "ÿ���ڵ���Ƶ�ӿڵ�������", m_NumIface);
	cmd.AddValue("WifiPhyStandard", "ʹ�õ�Wifi������׼����ѡֵ��0.WIFI_PHY_STANDARD_80211a��1.WIFI_PHY_STANDARD_80211b��2.WIFI_PHY_STANDARD_80211g��3.WIFI_PHY_STANDARD_80211_10MHZ��4.WIFI_PHY_STANDARD_80211_5MHZ��5.WIFI_PHY_STANDARD_holland��6.WIFI_PHY_STANDARD_80211n_2_4GHZ��7.WIFI_PHY_STANDARD_80211n_5GHZ����", wifiPhyStandard);
	cmd.AddValue("Step", "�ڵ����������Ч���ɽڵ��������;�����", m_Step);
	cmd.AddValue("ApplicationNum", "������Ӧ�ò���������������ڵ�ɳ��֣����·��Э��ΪPMTMGMP���������һ����ΪMAP��һ����ΪMPP��", m_ApplicationNum);
	cmd.AddValue("MaxBytes", "����������ݡ�", m_MaxBytes);
	cmd.AddValue("SourceNum", "Դ�ڵ���ţ����������������ã��Զ���Ϊ0��", m_SourceNum);
	cmd.AddValue("DestinationNum", "Ŀ�����ţ����������������ã��Զ���Ϊ���һ���ڵ㡣", m_DestinationNum);
	cmd.AddValue("PacketSize", "Ӧ�ò����С��", m_PacketSize);
	cmd.AddValue("DataRate", "�������ʡ�", m_DataRate);
	stringstream ss;
	ss << "������ʱ��,������ǰ��·������ʱ��(" << MIN_APPLICATION_TIME << "s)�ͺ�����ɵȴ�ʱ��(" << END_APPLICATION_TIME << "s)��";
	cmd.AddValue("TotalTime", ss.str(), m_TotalTime);
	cmd.AddValue("Root", "·��Э����Mesh�ڵ��Mac��ַ��", m_Root);

	cmd.Parse(argc, argv);

	m_ProtocolType = (MeshProtocolType)protocolType;
	m_AreaType = (NodeAreaType)areaType;
	m_WifiPhyStandard = (WifiPhyStandard)wifiPhyStandard;
}

// ͨ�����ü����������
void MeshRouteClass::SimulatorSetting()
{
	//������ʱ��
	m_TotalTime += MIN_APPLICATION_TIME + END_APPLICATION_TIME;

	// ����·��
	string l_AttributePath_PeerLink;// Peer Link
	string l_AttributePath_PeerManagementProtocol;// Peer Management Protocol
	string l_AttributePath_RouteProtocol;// Route Protocol
	string l_AttributePath_RouteProtocolPart;// �������Ʋ���

	//·��Э������
	switch (m_ProtocolType)
	{
	case MeshRouteClass::DOT11S_HWMP:
		l_AttributePath_PeerLink = "ns3::dot11s::PeerLink::";
		l_AttributePath_PeerManagementProtocol = "ns3::dot11s::PeerManagementProtocol::";
		l_AttributePath_RouteProtocol = "ns3::dot11s::HwmpProtocol::";
		l_AttributePath_RouteProtocolPart = "Dot11MeshHWMP";
		break;
	case MeshRouteClass::MY11S_PMTMGMP:
		l_AttributePath_PeerLink = "ns3::my11s::PmtmgmpPeerLink::";
		l_AttributePath_PeerManagementProtocol = "ns3::my11s::PmtmgmpPeerManagementProtocol::";
		l_AttributePath_RouteProtocol = "ns3::my11s::PmtmgmpProtocol::";
		l_AttributePath_RouteProtocolPart = "My11WmnPMTMGMP";
		break;
	default:
		break;
	}

	// ����Peer Link����
	Config::SetDefault(l_AttributePath_PeerLink + "MaxBeaconLoss", UintegerValue(32));
	Config::SetDefault(l_AttributePath_PeerLink + "MaxRetries", UintegerValue(8));
	Config::SetDefault(l_AttributePath_PeerLink + "MaxPacketFailure", UintegerValue(8));

	// ����Peer Management Protocol����
	Config::SetDefault(l_AttributePath_PeerManagementProtocol + "EnableBeaconCollisionAvoidance", BooleanValue(false));

	// ����·��Э�����
	Config::SetDefault(l_AttributePath_RouteProtocol + l_AttributePath_RouteProtocolPart + "activePathTimeout", TimeValue(Seconds(100)));
	Config::SetDefault(l_AttributePath_RouteProtocol + l_AttributePath_RouteProtocolPart + "activeRootTimeout", TimeValue(Seconds(100)));
	Config::SetDefault(l_AttributePath_RouteProtocol + l_AttributePath_RouteProtocolPart + "maxPREQretries", UintegerValue(5));
	Config::SetDefault(l_AttributePath_RouteProtocol + "UnicastPreqThreshold", UintegerValue(10));
	Config::SetDefault(l_AttributePath_RouteProtocol + "UnicastDataThreshold", UintegerValue(5));
	Config::SetDefault(l_AttributePath_RouteProtocol + "DoFlag", BooleanValue(true));
	Config::SetDefault(l_AttributePath_RouteProtocol + "RfFlag", BooleanValue(false));
}

// �����ڵ㼰���Э������
void MeshRouteClass::CreateNodes()
{
	// �ڵ��������ͣ�ͳ�ƽڵ�����
	switch (m_AreaType)
	{
	case MeshRouteClass::SQUARE:
		l_NodeNum = m_Size * m_Size;
		break;
	case MeshRouteClass::HEXAGON:
		l_NodeNum = 3 * (m_Size - 1) * m_Size + 1;
		break;
	default:
		break;
	}

	// �����ڵ�
	l_Nodes.Create(l_NodeNum);

	// ���������Э�鼰�ŵ�
	double m_txpower = 18.0; // dbm
	YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
	wifiPhy.Set("EnergyDetectionThreshold", DoubleValue(-89.0));
	wifiPhy.Set("CcaMode1Threshold", DoubleValue(-62.0));
	wifiPhy.Set("TxGain", DoubleValue(1.0));
	wifiPhy.Set("RxGain", DoubleValue(1.0));
	wifiPhy.Set("TxPowerLevels", UintegerValue(1));
	wifiPhy.Set("TxPowerEnd", DoubleValue(m_txpower));
	wifiPhy.Set("TxPowerStart", DoubleValue(m_txpower));
	wifiPhy.Set("RxNoiseFigure", DoubleValue(7.0));
	if (m_Pcap)	wifiPhy.EnablePcapAll(string("pmtmgmp-80211a--"));
	YansWifiChannelHelper wifiChannel;
	wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
	wifiChannel.AddPropagationLoss("ns3::LogDistancePropagationLossModel", "Exponent", StringValue("2.7"));
	wifiPhy.SetChannel(wifiChannel.Create());

	switch (m_ProtocolType)
	{
	case MeshRouteClass::DOT11S_HWMP:
		l_Mesh.SetStandard(WIFI_PHY_STANDARD_80211a);
		if (!Mac48Address(m_Root.c_str()).IsBroadcast())
		{
			l_Mesh.SetStackInstaller("ns3::Dot11sStack", "Root", Mac48AddressValue(Mac48Address(m_Root.c_str())));
		}
		else
		{
			l_Mesh.SetStackInstaller("ns3::Dot11sStack");
		}
		l_Mesh.SetMacType("RandomStart", TimeValue(Seconds(m_RandomStart)));
		l_Mesh.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode", StringValue("OfdmRate6Mbps"), "RtsCtsThreshold", UintegerValue(2500));
		l_Mesh.SetNumberOfInterfaces(m_NumIface);
		l_Mesh.SetSpreadInterfaceChannels(MeshHelper::ZERO_CHANNEL);
		l_NetDevices = l_Mesh.Install(wifiPhy, l_Nodes);
		break;
	case MeshRouteClass::MY11S_PMTMGMP:
		l_Pmtmgmp.SetStandard(m_WifiPhyStandard);
		if (!Mac48Address(m_Root.c_str()).IsBroadcast())
		{
			l_Pmtmgmp.SetStackInstaller("ns3::My11sStack", "Root", Mac48AddressValue(Mac48Address(m_Root.c_str())));
		}
		else
		{
			l_Pmtmgmp.SetStackInstaller("ns3::My11sStack");
		}
		l_Pmtmgmp.SetMacType("RandomStart", TimeValue(Seconds(m_RandomStart)));
		l_Pmtmgmp.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode", StringValue("OfdmRate6Mbps"), "RtsCtsThreshold", UintegerValue(2500));
		l_Pmtmgmp.SetNumberOfInterfaces(m_NumIface);
		l_Pmtmgmp.SetSpreadInterfaceChannels(WmnHelper::ZERO_CHANNEL);
		l_NetDevices = l_Pmtmgmp.Install(wifiPhy, l_Nodes);
		break;
	default:
		break;
	}
}

// �ڵ�λ������
void MeshRouteClass::LocateNodes()
{
	MobilityHelper mobility;

	Ptr<ListPositionAllocator> initialAlloc = CreateObject<ListPositionAllocator>();
	// �ڵ��������ͣ����ýڵ�λ��
	int r = 0, s = 0, t, u;
	double x, y;
	double fx, fy, sx, sy;
	switch (m_AreaType)
	{
	case MeshRouteClass::SQUARE:
		mobility.SetPositionAllocator("ns3::GridPositionAllocator",
			"MinX", DoubleValue(0.0),
			"MinY", DoubleValue(0.0),
			"DeltaX", DoubleValue(m_Step),
			"DeltaY", DoubleValue(m_Step),
			"GridWidth", UintegerValue(m_Size),
			"LayoutType", StringValue("RowFirst"));
		break;
	case MeshRouteClass::HEXAGON:
		initialAlloc->Add(Vector(0, 0, 0));
		for (int i = 1; i < l_NodeNum; i++)
		{
			if (s == r * 6) r++;
			s = i - r * (r - 1) * 3;
			t = (s - 1) / r;
			u = s - t * r - 1;
			if (u == 0)
			{
				fx = r * m_Step * cos(DEGREES_TO_RADIANS(60 * t));
				fy = r * m_Step * sin(DEGREES_TO_RADIANS(60 * t));
				sx = r * m_Step * cos(DEGREES_TO_RADIANS(60 * (t + 1)));
				sy = r * m_Step * sin(DEGREES_TO_RADIANS(60 * (t + 1)));
			}
			x = (sx - fx) / r;
			y = (sy - fy) / r;
			NS_LOG_INFO("R:" << i << ",s:" << s << ",t:" << t << ",u:" << u << ",r:" << r << ",R:" << x * x + y * y);
			initialAlloc->Add(Vector(fx + x * u, fy + y * u, 0.));
			mobility.SetPositionAllocator(initialAlloc);
		}
		break;
	default:
		break;
	}
	mobility.Install(l_Nodes);
}

// ��װ����Э��ջ
void MeshRouteClass::InstallInternetStack()
{
	// ��ȫ������ϰ�װInternetЭ��ջ
	InternetStackHelper internetStack;
	internetStack.Install(l_Nodes);
	// ����IP��ַ���豸�ӿ�
	Ipv4AddressHelper address;
	address.SetBase("192.168.1.0", "255.255.255.0");
	l_Interfaces = address.Assign(l_NetDevices);
}

// ��װӦ�ò�
void MeshRouteClass::InstallApplicationRandom()
{
	// ��ʹ�����Ӧ�ò�����
	if (m_ApplicationNum == 0)
	{
		if (m_SourceNum < 0 || m_SourceNum >= l_NodeNum) m_SourceNum = 0;
		if (m_DestinationNum < 0 || m_DestinationNum >= l_NodeNum || m_DestinationNum == m_SourceNum) m_DestinationNum = l_NodeNum - 1;
		m_SourceNum = m_Size + 1;
		m_DestinationNum = m_Size * (m_Size - 1) - 2;

		BulkSendHelper source("ns3::TcpSocketFactory", InetSocketAddress(l_Interfaces.GetAddress(m_DestinationNum), 49002));
		source.SetAttribute("MaxBytes", UintegerValue(m_MaxBytes));
		ApplicationContainer sourceApps = source.Install(l_Nodes.Get(m_SourceNum));
		sourceApps.Start(Seconds(MIN_APPLICATION_TIME));
		sourceApps.Stop(Seconds(m_TotalTime - END_APPLICATION_TIME));

		PacketSinkHelper sink("ns3::TcpSocketFactory", InetSocketAddress(l_Interfaces.GetAddress(m_DestinationNum), 49002));
		ApplicationContainer sinkApps = sink.Install(l_Nodes.Get(m_DestinationNum));
		sinkApps.Start(Seconds(MIN_APPLICATION_TIME));
		sinkApps.Stop(Seconds(m_TotalTime - END_APPLICATION_TIME));

		if (m_ProtocolType == MY11S_PMTMGMP)
		{
			DynamicCast<my11s::PmtmgmpProtocol>(DynamicCast<WmnPointDevice>(l_Nodes.Get(m_SourceNum)->GetDevice(0))->GetRoutingProtocol())->SetNodeType(my11s::PmtmgmpProtocol::Mesh_Access_Point);
			DynamicCast<my11s::PmtmgmpProtocol>(DynamicCast<WmnPointDevice>(l_Nodes.Get(m_DestinationNum)->GetDevice(0))->GetRoutingProtocol())->SetNodeType(my11s::PmtmgmpProtocol::Mesh_Portal);
		}
		return;
	}

	// ����Ӧ�ò����
	Config::SetDefault("ns3::OnOffApplication::PacketSize", UintegerValue(m_PacketSize));
	Config::SetDefault("ns3::OnOffApplication::DataRate", StringValue(m_DataRate));

	Ptr<UniformRandomVariable> rand_Nodes = CreateObject<UniformRandomVariable>();
	rand_Nodes->SetAttribute("Min", DoubleValue(0));
	rand_Nodes->SetAttribute("Max", DoubleValue(l_NodeNum));

	Ptr<UniformRandomVariable> rand_Port = CreateObject<UniformRandomVariable>();
	rand_Port->SetAttribute("Min", DoubleValue(49000));
	rand_Port->SetAttribute("Max", DoubleValue(49100));

	Ptr<UniformRandomVariable> rand_StarTime = CreateObject<UniformRandomVariable>();
	rand_StarTime->SetAttribute("Min", DoubleValue(MIN_APPLICATION_TIME));
	rand_StarTime->SetAttribute("Max", DoubleValue(m_TotalTime - END_APPLICATION_TIME));

	Ptr<ExponentialRandomVariable>rand_DurationTime = CreateObject<ExponentialRandomVariable>();
	rand_DurationTime->SetAttribute("Mean", DoubleValue(30));

	// ʹ�����Ӧ�ò�����
	for (int i = 0; i < m_ApplicationNum; i++){
		ApplicationContainer apps;
		int appStartTime = rand_StarTime->GetValue();
		int appDurationTime = rand_DurationTime->GetValue() + 1;
		int appStopTime = 0;

		// Ӧ����ֹʱ��
		if ((appStartTime + appDurationTime) >(m_TotalTime - 10)){
			appStopTime = m_TotalTime - 10;
		}
		else{
			appStopTime = appStartTime + appDurationTime;
		}

		// Ӧ��������
		char num[2];
		char onoff[7];
		char sink[6];
		strcpy(onoff, "onoff");
		strcpy(sink, "sink");
		sprintf(num, "%d", i);
		strcat(onoff, num);
		strcat(sink, num);

		//���ѡ��Դ�ڵ��Ŀ����
		int sourceNum = rand_Nodes->GetValue();
		int destinationNUM = rand_Nodes->GetValue();
		while (sourceNum == destinationNUM) // ����ѡ����ͬ�ڵ�
		{
			destinationNUM = rand_Nodes->GetValue();
		}
		uint32_t destination_Port = rand_Port->GetInteger();

		// ��������Ϣ
		NS_LOG_INFO("\n\t Node " << sourceNum << " to " << destinationNUM);
		NS_LOG_INFO("\n Start_time: " << appStartTime << "s");
		NS_LOG_INFO("\n Stop_time: " << appStopTime << "s\n");

		// ����Ӧ�ò�
		OnOffHelper onOffAPP("ns3::UdpSocketFactory", Address(InetSocketAddress(l_Interfaces.GetAddress(destinationNUM), destination_Port)));
		onOffAPP.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
		onOffAPP.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
		apps = onOffAPP.Install(l_Nodes.Get(sourceNum));
		apps.Start(Seconds(appStartTime));
		apps.Stop(Seconds(appStopTime));
		PacketSinkHelper sinkAPP("ns3::UdpSocketFactory", InetSocketAddress(l_Interfaces.GetAddress(destinationNUM), 49001));
		apps = sinkAPP.Install(l_Nodes.Get(destinationNUM));
		apps.Start(Seconds(1.0));
		if (m_ProtocolType == MY11S_PMTMGMP)
		{
			DynamicCast<my11s::PmtmgmpProtocol>(DynamicCast<WmnPointDevice>(l_Nodes.Get(sourceNum)->GetDevice(0))->GetRoutingProtocol())->SetNodeType(my11s::PmtmgmpProtocol::Mesh_Access_Point);
			DynamicCast<my11s::PmtmgmpProtocol>(DynamicCast<WmnPointDevice>(l_Nodes.Get(destinationNUM)->GetDevice(0))->GetRoutingProtocol())->SetNodeType(my11s::PmtmgmpProtocol::Mesh_Portal);
		}
	}
}

// ����ͳ��ģ������
void MeshRouteClass::StatsDataSet()
{
	//�������
	Config::Connect("/NodeList/*/DeviceList/*/RoutingProtocol/RouteDiscoveryTime", MakeCallback(&MeshRouteClass::CallBack_RouteDiscoveryTime, this));
	Config::Connect("/NodeList/*/ApplicationList/*/$ns3::BulkSendApplication/Tx", MakeCallback(&MeshRouteClass::CallBack_ApplicationTX, this));

	//Gnuplotͼ�����
	string probeType = "ns3::PacketProbe";
	string tracePath = "/NodeList/*/ApplicationList/*/$ns3::BulkSendApplication/Tx";

	GnuplotHelper plotHelper;
	plotHelper.ConfigurePlot("Mesh-Route-Test-Packet-Byte-Count", "Packet Byte Count vs. Time", "Time (Seconds)", "Packet Byte Count");
	plotHelper.PlotProbe(probeType, tracePath, "OutputBytes", "Packet Byte Count", GnuplotAggregator::KEY_BELOW);

	FileHelper fileHelper;
	fileHelper.ConfigureFile("Mesh-Route-Test", FileAggregator::FORMATTED);
	fileHelper.Set2dFormat("Time (Seconds) = %.3e\tPacket Byte Count = %.0f");
	fileHelper.WriteProbe(probeType, tracePath, "OutputBytes");
}

// �����������
void MeshRouteClass::FlowMonitorReport()
{
	// ����ͳ��Ӧ�õı���
	int k = 0;
	int totaltxPackets = 0;
	int totalrxPackets = 0;
	double totaltxbytes = 0;
	double totalrxbytes = 0;

	double totaldelay = 0;
	double totalrxbitrate = 0;
	double difftx, diffrx;
	double pdf_value, rxbitrate_value, txbitrate_value, delay_value;
	double pdf_total, rxbitrate_total, delay_total;
	// ���ÿ����������
	l_Monitor->CheckForLostPackets();
	Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(l_Flowmon.GetClassifier());
	std::map<FlowId, FlowMonitor::FlowStats> stats = l_Monitor->GetFlowStats();
	for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin(); i != stats.end(); ++i)
	{
		Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(i->first);
		difftx = i->second.timeLastTxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds();
		diffrx = i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstRxPacket.GetSeconds();
		pdf_value = (double)i->second.rxPackets / (double)i->second.txPackets * 100;
		txbitrate_value = (double)i->second.txBytes * 8 / 1024 / difftx;
		if (i->second.rxPackets != 0){
			rxbitrate_value = (double)i->second.rxPackets * m_PacketSize * 8 / 1024 / diffrx;
			delay_value = (double)i->second.delaySum.GetSeconds() / (double)i->second.rxPackets;
		}
		else{
			rxbitrate_value = 0;
			delay_value = 0;
		}
		// ��������
		if ((!t.destinationAddress.IsSubnetDirectedBroadcast("255.255.255.0")))
		{
			//�����������
			k++;
			NS_LOG_INFO("\nFlow " << k << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n");
			NS_LOG_INFO("PDF: " << pdf_value << " %\n");
			NS_LOG_INFO("Tx Packets: " << i->second.txPackets << "\n");
			NS_LOG_INFO("Rx Packets: " << i->second.rxPackets << "\n");
			NS_LOG_INFO("Lost Packets: " << i->second.lostPackets << "\n");
			NS_LOG_INFO("Dropped Packets: " << i->second.packetsDropped.size() << "\n");
			NS_LOG_INFO("PDF: " << pdf_value << " %\n");
			NS_LOG_INFO("Average delay: " << delay_value << "s\n");
			NS_LOG_INFO("Rx bitrate: " << rxbitrate_value << " kbps\n");
			NS_LOG_INFO("Tx bitrate: " << txbitrate_value << " kbps\n\n");
			// ͳ��ƽ��ֵ
			totaltxPackets += i->second.txPackets;
			totaltxbytes += i->second.txBytes;
			totalrxPackets += i->second.rxPackets;
			totaldelay += i->second.delaySum.GetSeconds();
			totalrxbitrate += rxbitrate_value;
			totalrxbytes += i->second.rxBytes;
		}
	}
	// ƽ��ȫ���������
	if (totaltxPackets != 0){
		pdf_total = (double)totalrxPackets / (double)totaltxPackets * 100;
	}
	else{
		pdf_total = 0;
	}
	if (totalrxPackets != 0){
		rxbitrate_total = totalrxbitrate;
		delay_total = (double)totaldelay / (double)totalrxPackets;
	}
	else{
		rxbitrate_total = 0;
		delay_total = 0;
	}
	// ���ȫ���������
	NS_LOG_INFO("\nTotal PDF: " << pdf_total << " %\n");
	NS_LOG_INFO("Total Rx bitrate: " << rxbitrate_total << " kbps\n");
	NS_LOG_INFO("Total Delay: " << delay_total << " s\n");
	// ���ȫ��������ݵ��ļ�
	std::ostringstream os;
	os << "1_HWMP_PDF.txt";
	std::ofstream of(os.str().c_str(), ios::out | ios::app);
	of << pdf_total << "\n";
	of.close();
	std::ostringstream os2;
	os2 << "1_HWMP_Delay.txt";
	std::ofstream of2(os2.str().c_str(), ios::out | ios::app);
	of2 << delay_total << "\n";
	of2.close();
	std::ostringstream os3;
	os3 << "1_HWMP_Throu.txt";
	std::ofstream of3(os3.str().c_str(), ios::out | ios::app);
	of3 << rxbitrate_total << "\n";
	of3.close();
	Simulator::Destroy();
}

// ��ʼ����
int MeshRouteClass::Run()
{
	// ͨ�����ü����������
	SimulatorSetting();
	// �����ڵ㼰���Э������
	CreateNodes();
	// �ڵ�λ������
#ifndef TEST_LOCATION
	LocateNodes();
#else
	MobilityHelper mobility;

	Ptr<ListPositionAllocator> initialAlloc = CreateObject<ListPositionAllocator>();
	initialAlloc->Add(Vector(0, 0, 0));
	mobility.SetPositionAllocator(initialAlloc);
	initialAlloc->Add(Vector(84, 84, 0.));
	mobility.SetPositionAllocator(initialAlloc);
	initialAlloc->Add(Vector(84, -84, 0.));
	mobility.SetPositionAllocator(initialAlloc);
	for (int i = 3; i < l_NodeNum; i++)
	{
		initialAlloc->Add(Vector(120 * (i - 3) + 168, 0, 0.));
		mobility.SetPositionAllocator(initialAlloc);
	}
	mobility.Install(l_Nodes);
#endif
	// ��װ����Э��ջ
	InstallInternetStack();
	// ��װӦ�ò�
#ifndef NO_APPLICATION
	InstallApplicationRandom();
#else
	if (m_SourceNum < 0 || m_SourceNum >= l_NodeNum) m_SourceNum = 0;
	if (m_DestinationNum < 0 || m_DestinationNum >= l_NodeNum || m_DestinationNum == m_SourceNum) m_DestinationNum = l_NodeNum - 1;
	m_SourceNum = m_Size + 1;
	m_DestinationNum = m_Size * (m_Size - 1) - 2;
	if (m_ProtocolType == MY11S_PMTMGMP)
	{
		/*DynamicCast<my11s::PmtmgmpProtocol>(DynamicCast<WmnPointDevice>(l_Nodes.Get(m_SourceNum)->GetDevice(0))->GetRoutingProtocol())->SetNodeType(my11s::PmtmgmpProtocol::Mesh_Access_Point);*/
		DynamicCast<my11s::PmtmgmpProtocol>(DynamicCast<WmnPointDevice>(l_Nodes.Get(0)->GetDevice(0))->GetRoutingProtocol())->SetNodeType(my11s::PmtmgmpProtocol::Mesh_Access_Point);
		/*DynamicCast<my11s::PmtmgmpProtocol>(DynamicCast<WmnPointDevice>(l_Nodes.Get(m_DestinationNum)->GetDevice(0))->GetRoutingProtocol())->SetNodeType(my11s::PmtmgmpProtocol::Mesh_Portal);*/
	}
#endif
	// ����ͳ��ģ������
	void StatsDataSet();

	//Ptr<WmnPointDevice> wpd = DynamicCast<WmnPointDevice>(l_Nodes.Get(0)->GetDevice(0));
	//Ptr<my11s::PmtmgmpProtocol> pp = DynamicCast<my11s::PmtmgmpProtocol>(wpd->GetRoutingProtocol());

	//�������
	l_Monitor = l_Flowmon.InstallAll();

	float timeStart = clock();
	Simulator::Schedule(Seconds(timeStart), &MeshRouteClass::Report, this);
	Simulator::Stop(Seconds(m_TotalTime));
	Simulator::Run();
	Simulator::Destroy();

	// �����������
	FlowMonitorReport();
	return 0;
}

// �ص�����
void MeshRouteClass::CallBack_RouteDiscoveryTime(string context, Time time)
{
	NS_LOG_INFO("RouteDiscoveryTime:" << time.GetSeconds() << std::endl);
}
void MeshRouteClass::CallBack_ApplicationTX(string context, Ptr<const Packet> packet)
{
	NS_LOG_INFO("Packet Size:" << packet->GetSize() << std::endl);
}

//�������
void MeshRouteClass::Report()
{
	unsigned n(0);
	for (NetDeviceContainer::Iterator i = l_NetDevices.Begin();
		i != l_NetDevices.End(); ++i, ++n)
	{
		std::ostringstream os;
		//os << "mp-report1-" << n << ".xml";
		os << "mp-report.xml";
		std::ofstream of;
		of.open(os.str().c_str(), ios::out | ios::app);
		if (!of.is_open())
		{
			NS_LOG_INFO("Error: Can't open file " << os.str() << "\n");
			return;
		}
		switch (m_ProtocolType)
		{
		case MeshRouteClass::DOT11S_HWMP:
			l_Mesh.Report(*i, of);
			break;
		case MeshRouteClass::MY11S_PMTMGMP:
			l_Pmtmgmp.Report(*i, of);
			break;
		default:
			break;
		}
		of.close();
	}
	n = 0;
}
int main(int argc, char *argv[])
{
#ifdef OUT_TO_FILE
#ifdef WIN32
	ofstream logfile("E:\\Ns-3.23-log.log");
	std::clog.rdbuf(logfile.rdbuf());
#endif
#endif

	//LogComponentEnableAll((LogLevel)(LOG_LEVEL_INFO | LOG_PREFIX_ALL));

	LogComponentEnable("PmtmgmpProtocol", (LogLevel)(LOG_LEVEL_ALL | LOG_PREFIX_ALL));
	//LogComponentEnable("WmnPointDevice", (LogLevel)(LOG_LEVEL_ALL | LOG_PREFIX_ALL));
	LogComponentEnable("PmtmgmpRtable", (LogLevel)(LOG_LEVEL_ALL | LOG_PREFIX_ALL));
	//LogComponentEnable("WmnWifiInterfaceMac", (LogLevel)(LOG_LEVEL_ALL | LOG_PREFIX_ALL));
	//LogComponentEnable("PmtmgmpProtocolMac", (LogLevel)(LOG_LEVEL_ALL | LOG_PREFIX_ALL));
	//LogComponentEnable("PmtmgmpPeerManagementProtocol", (LogLevel)(LOG_LEVEL_ALL | LOG_PREFIX_ALL));
	//LogComponentEnable("HwmpProtocol", (LogLevel)(LOG_LEVEL_ALL | LOG_PREFIX_ALL));
	//LogComponentEnable("MeshPointDevice", (LogLevel)(LOG_LEVEL_ALL | LOG_PREFIX_ALL));

	//LogComponentEnable("BulkSendApplication", (LogLevel)(LOG_LEVEL_ALL | LOG_PREFIX_ALL));

	LogComponentEnable("MeshRouteTest", (LogLevel)(LOG_LEVEL_ALL | LOG_PREFIX_ALL));

	PacketMetadata::Enable();
	MeshRouteClass t;
	t.Configure(argc, argv);
	return t.Run();
}
