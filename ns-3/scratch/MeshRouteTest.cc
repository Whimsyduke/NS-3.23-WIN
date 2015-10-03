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

//������ļ�
//#define OUT_TO_FILE
//��Ӧ�ò�ģʽ
//#define NO_APPLICATION
//����ģʽ
//#define TEST_LOCATION
//��������Э��
#define TEST_ALL
//���Բ�ͬ�߽�
#define TEST_SIDE_ALL
//ʹ��Stats
#define TEST_STATS
#ifdef TEST_STATS
//��ʾ�ص���־
//#define TEST_STATS_SHOW_LOG
#endif

//�Ƕ�ת��
#define DEGREES_TO_RADIANS(__ANGLE__) ((__ANGLE__) * 0.01745329252f) // PI / 180

// ����ǰ·������ʱ��
#define MIN_APPLICATION_TIME 10
// �����ȴ�ʱ��
#define END_APPLICATION_TIME 20

// ���Ӧ������
#define TEST_SET_COUNT 5
// ���Ӧ�����м��
#define TEST_SET_INTERVAL 6
// ���Ӧ�ó���ʱ��
#define TEST_SET_LIFE 26
// ���Ӧ���������
#define TEST_SET_RANDOM 4
// ������״
#define TEST_SET_AREA MeshRouteClass::SQUARE
// Ӧ�ò�������
#define TEST_SET_APP MeshRouteClass::Random
// Э��
#define TEST_SET_PROTOCOL MeshRouteClass::MY11S_PMTMGMP

// ��������С
#define TEST_SET_MAX_SIZE 7
// ������С��С
#define TEST_SET_MIN_SIZE 4
// �����С
#define TEST_SET_SIZE 5
// ������
#define TEST_SET_APPSTEP 3
// ���������С�Ĳ�ֵ
#define TEST_SET_SIZE_APPSTEP 2


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

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE("MeshRouteTest");

struct NodeApplicationInfor
{
	uint32_t SrcIndex;
	uint32_t DstIndex;
	double Start;
	double End;
};
#ifdef TEST_STATS
struct StatsData
{
	Time CurrentTime;
	uint32_t CurrentTX;
	uint32_t CurrentRX;
	StatsData *Next;
	
	StatsData();
	StatsData(Time currentTime,	uint32_t currentTX,	uint32_t currentRX);
};
StatsData::StatsData() : CurrentTime(Seconds(0)), CurrentTX(0), CurrentRX(0), Next(NULL)
{
}
StatsData::StatsData(Time currentTime, uint32_t currentTX, uint32_t currentRX)
{
	CurrentTime = currentTime;
	CurrentTX = currentTX;
	CurrentRX = currentRX;
}
#endif
class MeshRouteClass : public Object
{
public:
	//�����Ͷ���
	enum MeshProtocolType// Mesh·��Э������
	{
		DOT11S_HWMP,// HWMP·��Э��
		DOT11S_FLAME,
		MY11S_PMTMGMP,// PMTMGMP·��Э��
	};

	enum NodeAreaType// �ڵ�������״����
	{
		SQUARE,// ������
		HEXAGON,// ��������
	};

	enum ApplicationAddType// Ӧ�ò���ӷ�ʽ
	{
		Simple,
		Multiple,
		Random
	};

public:
	static TypeId GetTypeId(void);
	// ��ʼ������
	MeshRouteClass();
	// ���ò���
	void SetMeshRouteClass(MeshProtocolType protocolType, vector<NodeApplicationInfor> applications, double totalTime, NodeAreaType areaType, int size, int appStep, ApplicationAddType appType);
	void SetMeshRouteClass(MeshProtocolType protocolType, NodeAreaType areaType, int size, int appStep, ApplicationAddType appType);
	//// ���ò���
	//void Configur/*e(in*/t argc, char ** argv);
	// ��ʼ����
	int Run();

	static string GetProtocolNameString(MeshProtocolType type);

private:
	void DoInitialize(void);
	// ��������
	void SimulatorSetting();
	// �����ڵ㼰���Э������
	void CreateNodes();
	// �ڵ�λ������
	void LocateNodes();
	// ��װ����Э��ջ
	void InstallInternetStack();
	// ��װһ��Ӧ��
	void InstallCoupleApplication(int srcIndex, int dstIndex, int port, double start, double end);
	// ��װӦ�ò�
	void InstallApplicationRandom();
	// �����������
	void FlowMonitorReport();

#ifdef TEST_STATS
private:
	// �ص�����
	void CallBack_ApplicationTX(string context, Ptr<const Packet> packet);
	void CallBack_ApplicationRX(std::string path, Ptr<const Packet> packet, const Address &from);
#ifdef TEST_STATS_SHOW_LOG
	void CallBack_PDF(std::string context, double oldValue, double newValue);
#endif
#endif
	// �������
	void Report();

private:
	// ���������ò���
	MeshProtocolType m_ProtocolType;
	NodeAreaType m_AreaType;
	int m_Size;
	double m_RandomStart;
	uint32_t m_NumIface;
	WifiPhyStandard m_WifiPhyStandard;
	double m_Step;
	uint8_t m_ApplicationStep;
	ApplicationAddType m_ApplicationAddType;
	uint32_t  m_MaxBytes;
	uint16_t  m_PacketSize;
	string m_DataRate;
	double m_TotalTime;
	string m_Root;
	bool m_Pcap;
	double m_PacketInterval;
	vector<NodeApplicationInfor> m_Applications;
	uint32_t m_totalTx;
	uint32_t m_totalRx;
	TracedValue<double> m_PDF;

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

#ifdef TEST_STATS
	// ���в�������
	vector<StatsData> l_statistics;
#endif
};

TypeId MeshRouteClass::GetTypeId(void)
{
	static TypeId tid = TypeId("ns3::MeshRoute")
		.AddConstructor<MeshRouteClass>()
		.SetParent<Object>()
		.AddTraceSource("PDF",
			"Packet Delvery Fraction",
			MakeTraceSourceAccessor(&MeshRouteClass::m_PDF),
			"ns3::TracedValue::DoubleCallback")
		;
	return tid;
}

// ��ʼ������
MeshRouteClass::MeshRouteClass() :
	m_RandomStart(1),
	m_NumIface(1),
	m_WifiPhyStandard(WIFI_PHY_STANDARD_80211g),
	m_Step(100),
	m_MaxBytes(0),
	m_PacketSize(1024),
	m_DataRate("150kbps"),
	m_TotalTime(20),
	m_Root("00:00:00:00:00:01"),
	m_Pcap(false),
	m_PacketInterval(0.1),
	m_totalTx(0),
	m_totalRx(0)
{
	// ��·�㼰�����Э������
	m_PDF = 0;
	l_Mesh = MeshHelper::Default();
	l_Pmtmgmp = WmnHelper::Default();
#ifdef TEST_STATS
	l_statistics = vector<StatsData>();
#endif
}

// ���ò���
void MeshRouteClass::SetMeshRouteClass(MeshProtocolType protocolType, vector<NodeApplicationInfor> applications, double totalTime, NodeAreaType areaType, int size, int appStep, ApplicationAddType appType)
{
	m_ProtocolType = protocolType;
	m_Applications = applications;
	m_TotalTime = totalTime;
	m_AreaType = areaType;
	m_Size = size;
	m_ApplicationStep = appStep;
	m_ApplicationAddType = appType;
}

void MeshRouteClass::SetMeshRouteClass(MeshProtocolType protocolType, NodeAreaType areaType, int size, int appStep, ApplicationAddType appType)
{
	m_ProtocolType = protocolType;
	m_AreaType = areaType;
	m_Size = size;
	m_ApplicationStep = appStep;
	m_ApplicationAddType = appType;
}

void MeshRouteClass::DoInitialize(void)
{
	NS_LOG_FUNCTION(this);
}

// ͨ�����ü����������
void MeshRouteClass::SimulatorSetting()
{
	//������ʱ��
	m_TotalTime += MIN_APPLICATION_TIME + END_APPLICATION_TIME;

	// ����·��
	if (m_ProtocolType != DOT11S_FLAME)
	{
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
			Config::SetDefault("ns3::my11s::PmtmgmpRouteTree::MSECPnumForMTERP", UintegerValue(3));
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
		Config::SetDefault(l_AttributePath_RouteProtocol + "MaxTtl", UintegerValue(100));

		// ����Ӧ�ò����
		Config::SetDefault("ns3::OnOffApplication::PacketSize", UintegerValue(m_PacketSize));
		Config::SetDefault("ns3::OnOffApplication::DataRate", StringValue(m_DataRate));
	}
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

	NS_ASSERT(l_NodeNum < 255);

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
		l_Mesh.SetStandard(m_WifiPhyStandard);
		if (!Mac48Address(m_Root.c_str()).IsBroadcast())
		{
			l_Mesh.SetStackInstaller("ns3::Dot11sStack", "Root", Mac48AddressValue(Mac48Address(m_Root.c_str())));
		}
		else
		{
			l_Mesh.SetStackInstaller("ns3::Dot11sStack");
		}
		l_Mesh.SetMacType("RandomStart", TimeValue(Seconds(m_RandomStart)));
		l_Mesh.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode", StringValue("ErpOfdmRate9Mbps"), "RtsCtsThreshold", UintegerValue(2500));
		l_Mesh.SetNumberOfInterfaces(m_NumIface);
		l_Mesh.SetSpreadInterfaceChannels(MeshHelper::ZERO_CHANNEL);
		l_NetDevices = l_Mesh.Install(wifiPhy, l_Nodes);
		break;
	case MeshRouteClass::DOT11S_FLAME:
		l_Mesh.SetStandard(m_WifiPhyStandard);
		l_Mesh.SetStackInstaller("ns3::FlameStack");
		l_Mesh.SetMacType("RandomStart", TimeValue(Seconds(m_RandomStart)));
		l_Mesh.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode", StringValue("ErpOfdmRate9Mbps"), "RtsCtsThreshold", UintegerValue(2500));
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
		l_Pmtmgmp.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode", StringValue("ErpOfdmRate9Mbps"), "RtsCtsThreshold", UintegerValue(2500));
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
			//NS_LOG_INFO("�����:" << i << ",��ǰ�������:" << s << ",�������:" << t << ",�������:" << u << ",����:" << r << ",�뾶:" << x * x + y * y);
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
	Ipv4InterfaceContainer interfaces = address.Assign(l_NetDevices);
}

// ��װһ��Ӧ��
void MeshRouteClass::InstallCoupleApplication(int srcIndex, int dstIndex, int port, double start, double end)
{
	OnOffHelper onOffAPP("ns3::UdpSocketFactory", Address(InetSocketAddress(l_Interfaces.GetAddress(dstIndex), port)));
	onOffAPP.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
	onOffAPP.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
	ApplicationContainer sourceApps = onOffAPP.Install(l_Nodes.Get(srcIndex));
	sourceApps.Start(Seconds(MIN_APPLICATION_TIME + start));
	sourceApps.Stop(Seconds(MIN_APPLICATION_TIME + end));

	PacketSinkHelper sink("ns3::UdpSocketFactory", InetSocketAddress(l_Interfaces.GetAddress(dstIndex), port));
	ApplicationContainer sinkApps = sink.Install(l_Nodes.Get(dstIndex));
	sinkApps.Start(Seconds(MIN_APPLICATION_TIME + start));
	sinkApps.Stop(Seconds(MIN_APPLICATION_TIME + end + END_APPLICATION_TIME));

	if (m_ProtocolType == MY11S_PMTMGMP)
	{
		DynamicCast<my11s::PmtmgmpProtocol>(DynamicCast<WmnPointDevice>(l_Nodes.Get(srcIndex)->GetDevice(0))->GetRoutingProtocol())->SetNodeType(my11s::PmtmgmpProtocol::Mesh_Access_Point);
		DynamicCast<my11s::PmtmgmpProtocol>(DynamicCast<WmnPointDevice>(l_Nodes.Get(dstIndex)->GetDevice(0))->GetRoutingProtocol())->SetNodeType(my11s::PmtmgmpProtocol::Mesh_Portal);
	}
#ifdef TEST_STATS
	string str;
	char chsrc[3];
	char chdst[3];
	sprintf(chsrc, "%d", srcIndex);
	str = chsrc;
	Config::Connect("/NodeList/" + str + "/ApplicationList/*/$ns3::OnOffApplication/Tx", MakeCallback(&MeshRouteClass::CallBack_ApplicationTX, this));
	sprintf(chdst, "%d", dstIndex);
	str = chdst;
	Config::Connect("/NodeList/" + str + "/ApplicationList/*/$ns3::PacketSink/Rx", MakeCallback(&MeshRouteClass::CallBack_ApplicationRX, this));
	
#endif
}

// ��װӦ�ò�
void MeshRouteClass::InstallApplicationRandom()
{
	m_ApplicationStep++;
	switch (m_ApplicationAddType)
	{
	case MeshRouteClass::Simple:
		switch (m_AreaType)
		{
		case MeshRouteClass::SQUARE:
		{
			InstallCoupleApplication(m_Size + 1, m_Size * (m_Size - 1) - 2, 49001, 0, m_TotalTime);
		}
		break;
		case MeshRouteClass::HEXAGON:
		{
			if (m_Size == 2)
			{
				InstallCoupleApplication(1, 4, 49001, 0, m_TotalTime);
			}
			else
			{
				InstallCoupleApplication(l_NodeNum + 18 - 12 * m_Size, l_NodeNum + 12 - 9 * m_Size, 49001, 0, m_TotalTime);
			}
		}
		break;
		default:
			break;
		}
		break;
	case MeshRouteClass::Multiple:
		switch (m_AreaType)
		{
		case MeshRouteClass::SQUARE:
		{
			if (m_ApplicationStep < 1)
			{
				NS_LOG_ERROR("Ӧ�ü������С��0");
			}
			if (m_Size <= m_ApplicationStep)
			{
				// �ڵ���������ʹ�ö�Ӧ��
				InstallCoupleApplication(m_Size + 1, m_Size * (m_Size - 1) - 2, 49001, 0, m_TotalTime);
			}
			int start = ((m_Size - 1) % m_ApplicationStep) / 2;
			bool end = false;
			int count = 0;
			for (int y = start; y < m_Size; y += m_ApplicationStep)
			{
				for (int x = start; x < m_Size; x += m_ApplicationStep)
				{
					int i = y * m_Size + x;
					if (i > (l_NodeNum - 1) / 2)
					{
						end = true;
						break;
					}
					if (count % 2 == 0)
					{
						InstallCoupleApplication(y * m_Size + x, l_NodeNum - y * m_Size - x - 1, 49001 + i, i, m_TotalTime);
					}
					else
					{
						InstallCoupleApplication(l_NodeNum - y * m_Size - x - 1, y * m_Size + x, 49001 + i, i, m_TotalTime);
					}
					count++;
				}
				if (end)
				{
					break;
				}
			}
		}
		break;
		case MeshRouteClass::HEXAGON:
		{
			if (m_ApplicationStep < 1)
			{
				NS_LOG_ERROR("Ӧ�ü������С��0");
			}
			if (m_Size <= m_ApplicationStep)
			{
				// �ڵ���������ʹ�ö�Ӧ��
				if (m_Size == 2)
				{
					InstallCoupleApplication(1, 4, 49001, 0, m_TotalTime);
				}
				else
				{
					InstallCoupleApplication(l_NodeNum + 18 - 12 * m_Size, l_NodeNum + 12 - 9 * m_Size, 49001, 0, m_TotalTime);
				}
			}
			else
			{
				int n = 0;
				for (int r = m_ApplicationStep - 1; r < m_Size; r = r + m_ApplicationStep)
				{
					int s = r * (r - 1) * 3 + 1;
					for (int i = 0; i < r * 3; i = i + m_ApplicationStep - 1)
					{
						if (n % 2 == 0)
						{
							InstallCoupleApplication(s + i, s + r * 3 + i, 49000 + n, n, m_TotalTime);
						}
						else
						{
							InstallCoupleApplication(s + r * 3 + i, s + i, 49000 + n, n, m_TotalTime);
						}
						n++;
					}
				}
			}
		}
		break;
		default:
			break;
		}
		break;
	case MeshRouteClass::Random:
	{
		int port = 0;
		for (vector<NodeApplicationInfor>::iterator iter = m_Applications.begin(); iter != m_Applications.end(); iter++)
		{
			port++;
			InstallCoupleApplication(iter->SrcIndex, iter->DstIndex, 49000 + port, iter->Start, iter->End);
		}
	}
	break;
	default:
		break;
	}
}

// �����������
void MeshRouteClass::FlowMonitorReport()
{
	string typeName;
	switch (m_ProtocolType)
	{
	case MeshRouteClass::DOT11S_HWMP:
		typeName = "DOT11S_HWMP";
		break;
	case MeshRouteClass::DOT11S_FLAME:
		typeName = "DOT11S_FLAME";
		break;
	case MeshRouteClass::MY11S_PMTMGMP:
		typeName = "MY11S_PMTMGMP";
		break;
	default:
		break;
	}
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
		if (i->second.rxPackets != 0) {
			rxbitrate_value = (double)i->second.rxPackets * m_PacketSize * 8 / 1024 / diffrx;
			delay_value = (double)i->second.delaySum.GetSeconds() / (double)i->second.rxPackets;
		}
		else {
			rxbitrate_value = 0;
			delay_value = 0;
		}
		// ��������
		if ((!t.destinationAddress.IsSubnetDirectedBroadcast("255.255.255.0")))
		{
			//�����������
			k++;
			NS_LOG_INFO("-----------------------------");
			NS_LOG_INFO("Protocol: " << typeName << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")");
			NS_LOG_INFO("-----------------------------");
			NS_LOG_INFO("PDF: " << pdf_value << " %");
			NS_LOG_INFO("Tx Packets: " << i->second.txPackets);
			NS_LOG_INFO("Rx Packets: " << i->second.rxPackets);
			NS_LOG_INFO("Lost Packets: " << i->second.lostPackets);
			NS_LOG_INFO("Dropped Packets: " << i->second.packetsDropped.size());
			NS_LOG_INFO("PDF: " << pdf_value << " %");
			NS_LOG_INFO("Average delay: " << delay_value << "s");
			NS_LOG_INFO("Rx bitrate: " << rxbitrate_value << " kbps");
			NS_LOG_INFO("Tx bitrate: " << txbitrate_value << " kbps");
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
	if (totaltxPackets != 0) {
		pdf_total = (double)totalrxPackets / (double)totaltxPackets * 100;
	}
	else {
		pdf_total = 0;
	}
	if (totalrxPackets != 0) {
		rxbitrate_total = totalrxbitrate;
		delay_total = (double)totaldelay / (double)totalrxPackets;
	}
	else {
		rxbitrate_total = 0;
		delay_total = 0;
	}
	// ���ȫ���������
	NS_LOG_INFO("=============================");
	NS_LOG_INFO("Total PDF: " << pdf_total << " %");
	NS_LOG_INFO("Total Rx bitrate: " << rxbitrate_total << " kbps");
	NS_LOG_INFO("Total Delay: " << delay_total << " s");
	NS_LOG_INFO("=============================\n");
	
#ifdef TEST_STATS
	//	���ͳ������
	string fileName;
	char ch[3];
	sprintf(ch, "%d", m_Size);
	fileName = GetProtocolNameString(m_ProtocolType) + "_" + ch + "_PDF.dat";
	char *chName;
	chName = new char[fileName.length() + 1];
	strcpy(chName, fileName.c_str());
	remove(chName);

	std::ostringstream osStats;
	osStats << fileName;
	std::ofstream ofStats(osStats.str().c_str(), ios::out | ios::app);

	double pdf;
	for (vector<StatsData>::iterator iter = l_statistics.begin(); iter != l_statistics.end(); iter++)
	{
		if (iter->CurrentTX == 0)
		{
			pdf = 0;
		}
		else
		{
			pdf = iter->CurrentRX * 100 / iter->CurrentTX;
		}
		ofStats << iter->CurrentTime.GetSeconds() << " " << pdf << "\n";
	}

	ofStats.close();
	Simulator::Destroy();
#endif
}

// ��ʼ����
int MeshRouteClass::Run()
{
#ifdef TEST_STATS_SHOW_LOG
	// ������ʱ
	Config::Connect("/Names/MeshRoute/PDF", MakeCallback(&MeshRouteClass::CallBack_PDF, this));
#endif

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
	if (m_ProtocolType == MY11S_PMTMGMP)
	{
		/*DynamicCast<my11s::PmtmgmpProtocol>(DynamicCast<WmnPointDevice>(l_Nodes.Get(m_Size + 1)->GetDevice(0))->GetRoutingProtocol())->SetNodeType(my11s::PmtmgmpProtocol::Mesh_Access_Point);*/
		DynamicCast<my11s::PmtmgmpProtocol>(DynamicCast<WmnPointDevice>(l_Nodes.Get(0)->GetDevice(0))->GetRoutingProtocol())->SetNodeType(my11s::PmtmgmpProtocol::Mesh_Access_Point);
		/*DynamicCast<my11s::PmtmgmpProtocol>(DynamicCast<WmnPointDevice>(l_Nodes.Get(m_Size * (m_Size - 1) - 2)->GetDevice(0))->GetRoutingProtocol())->SetNodeType(my11s::PmtmgmpProtocol::Mesh_Portal);*/
	}
#endif
	//Ptr<WmnPointDevice> wpd = DynamicCast<WmnPointDevice>(l_Nodes.Get(0)->GetDevice(0));
	//Ptr<my11s::PmtmgmpProtocol> pp = DynamicCast<my11s::PmtmgmpProtocol>(wpd->GetRoutingProtocol());

	//�������
	l_Monitor = l_Flowmon.InstallAll();

	Simulator::Schedule(Seconds(m_TotalTime), &MeshRouteClass::Report, this);
	Simulator::Stop(Seconds(m_TotalTime));
	Simulator::Run();

	// �����������
	FlowMonitorReport();
	return 0;
}
string MeshRouteClass::GetProtocolNameString(MeshProtocolType type)
{
	switch (type)
	{
	case MeshRouteClass::DOT11S_HWMP:
		return "DOT11S_HWMP";
	case MeshRouteClass::DOT11S_FLAME:
		return "DOT11S_FLAME";
	case MeshRouteClass::MY11S_PMTMGMP:
		return "MY11S_PMTMGMP";
	default:
		return "";
	}
}
#ifdef TEST_STATS
// �ص�����
void MeshRouteClass::CallBack_ApplicationTX(string context, Ptr<const Packet> packet)
{
	m_totalTx += packet->GetSize();
#ifdef TEST_STATS_SHOW_LOG
	NS_LOG_INFO("Send Packet, Size:" << packet->GetSize());
#endif
}
void MeshRouteClass::CallBack_ApplicationRX(std::string path, Ptr<const Packet> packet, const Address &from)
{
	m_totalRx += packet->GetSize();
	m_PDF = (double)m_totalRx / (double)m_totalTx * 100;
	StatsData stats = StatsData(Simulator::Now(), m_totalTx, m_totalRx);
	l_statistics.push_back(stats);
#ifdef TEST_STATS_SHOW_LOG
	NS_LOG_INFO("Receive Packet, Size:" << packet->GetSize() << ", PDF:" << m_PDF);
#endif
}
#ifdef TEST_STATS_SHOW_LOG
void MeshRouteClass::CallBack_PDF(std::string context, double oldValue, double newValue)
{
	NS_LOG_INFO("PDF Change:" << oldValue << " to " << newValue << " with context is :" << context);
}
#endif
#endif

//�������
void MeshRouteClass::Report()
{
	unsigned n(0);
	
	for (NetDeviceContainer::Iterator i = l_NetDevices.Begin();
	i != l_NetDevices.End(); ++i, ++n)
	{
		std::ostringstream os;
		//os << "mp-report1-" << n << ".xml";
		stringstream ss;
		string s;
		ss << n;
		ss >> s;
		s = GetProtocolNameString(m_ProtocolType) + "-mp-report-" + s + ".xml";
#ifndef TEST_SIDE_ALL
		remove(s.c_str());
#endif
		os << s;
		std::ofstream of;
		of.open(os.str().c_str(), ios::out | ios::app);
		if (!of.is_open())
		{
			NS_LOG_INFO("Error: Can't open file " << os.str() << "\n");
			return;
		}
#ifdef TEST_SIDE_ALL
		string areatype;
		if (TEST_SET_AREA == MeshRouteClass::HEXAGON)
			areatype = "MeshRouteClass::HEXAGON";
		else if(TEST_SET_AREA == MeshRouteClass::SQUARE)
			areatype = "MeshRouteClass::SQUARE";

		of << "<!--TEST AREA:" << areatype  <<"(" << m_Size << ")-->\n";
#endif
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
#ifdef TEST_STATS
void SetStats(string protocolName, uint8_t size, Ptr<MeshRouteClass> mesh)
{
	//string str;
	//char ch[6];
	//sprintf(ch, "%d", size);
	//str = protocolName + "_" + ch;                                                                                                                                                                                                                                                                                                                             
	//Names::Add("/Names/MeshRoute", mesh);
	//GnuplotHelper plotHelper; 
	
	//plotHelper.ConfigurePlot(str, "Packet Delvery Fraction vs. Time", "Time (Seconds)", "Packet Delvery Fraction (%)", "png");
	//plotHelper.PlotProbe("ns3::DoubleProbe", "/Names/MeshRoute/PDF", "Output", "Packet Delvery Fraction", GnuplotAggregator::KEY_INSIDE);

	//FileHelper fileHelper;
	//fileHelper.ConfigureFile(str, FileAggregator::FORMATTED);
	//fileHelper.Set2dFormat("Time (Seconds) = %.3e\tPacket Byte Count = %.0f");
	//fileHelper.WriteProbe("ns3::DoubleProbe", "/Names/MeshRoute/PDF", "Output");
}
#endif


int main(int argc, char *argv[])
{
	// ����������
	CommandLine cmd;
	cmd.Parse(argc, argv);
	setlocale(LC_ALL, "zh_CN.gbk");
#ifdef OUT_TO_FILE
#ifdef WIN32
	ofstream logfile("Ns-3.23-log.log");
	std::clog.rdbuf(logfile.rdbuf());
#else
#endif
#endif
#ifndef TEST_SIDE_ALL
	//LogComponentEnableAll((LogLevel)(LOG_LEVEL_INFO | LOG_PREFIX_ALL));

	//LogComponentEnable("PmtmgmpProtocol", (LogLevel)(LOG_LEVEL_ALL | LOG_PREFIX_ALL));
	//LogComponentEnable("PmtmgmpProtocolMac", (LogLevel)(LOG_LEVEL_ALL | LOG_PREFIX_ALL));
	//LogComponentEnable("PmtmgmpPeerManagementProtocol", (LogLevel)(LOG_LEVEL_ALL | LOG_PREFIX_ALL));
	//LogComponentEnable("PmtmgmpRtable", (LogLevel)(LOG_LEVEL_ALL | LOG_PREFIX_ALL));
	//LogComponentEnable("WmnPointDevice", (LogLevel)(LOG_LEVEL_ALL | LOG_PREFIX_ALL));
	//LogComponentEnable("WmnWifiInterfaceMac", (LogLevel)(LOG_LEVEL_ALL | LOG_PREFIX_ALL));

	//LogComponentEnable("FlameProtocol", (LogLevel)(LOG_LEVEL_ALL | LOG_PREFIX_ALL)); 
	//LogComponentEnable("FlameProtocolMac", (LogLevel)(LOG_LEVEL_ALL | LOG_PREFIX_ALL));

	//LogComponentEnable("HwmpProtocol", (LogLevel)(LOG_LEVEL_ALL | LOG_PREFIX_ALL));
	//LogComponentEnable("MeshPointDevice", (LogLevel)(LOG_LEVEL_ALL | LOG_PREFIX_ALL));

	//LogComponentEnable("PacketSink", (LogLevel)(LOG_LEVEL_ALL | LOG_PREFIX_ALL));
	//LogComponentEnable("OnOffApplication", (LogLevel)(LOG_LEVEL_ALL | LOG_PREFIX_ALL));
	//LogComponentEnable("BulkSendApplication", (LogLevel)(LOG_LEVEL_ALL | LOG_PREFIX_ALL));
	//LogComponentEnable("UdpServer", (LogLevel)(LOG_LEVEL_ALL | LOG_PREFIX_ALL));
	//LogComponentEnable("UdpClient", (LogLevel)(LOG_LEVEL_ALL | LOG_PREFIX_ALL));
	//LogComponentEnable("UdpTraceClient", (LogLevel)(LOG_LEVEL_ALL | LOG_PREFIX_ALL)); 
	//LogComponentEnable("UdpEchoClientApplication", (LogLevel)(LOG_LEVEL_ALL | LOG_PREFIX_ALL)); 
#endif

	LogComponentEnable("MeshRouteTest", (LogLevel)(LOG_LEVEL_ALL | LOG_PREFIX_ALL));

	PacketMetadata::Enable();
	int totalTime = 0;
	if (TEST_SET_APP == MeshRouteClass::Random)
	{
#ifdef TEST_SIDE_ALL
		stringstream ss;
		string s;
		for (int i = 0; i < TEST_SET_MAX_SIZE * TEST_SET_MAX_SIZE; i++)
		{
			ss << i;
			ss >> s;
			s = "DOT11S_HWMP-mp-report-" + s + ".xml";
			remove(s.c_str());
			s = "MY11S_PMTMGMP-mp-report-" + s + ".xml";
			remove(s.c_str());
		}
		remove("1_HWMP_PDF.txt");
		remove("1_HWMP_Delay.txt");
		remove("1_HWMP_Throu.txt");
#endif
//���������߳�
#ifdef TEST_SIDE_ALL 
//����ȫ��Э��
#ifdef TEST_ALL 
		for (int i = TEST_SET_MIN_SIZE; i <= TEST_SET_MAX_SIZE; i++)
		{
			vector<NodeApplicationInfor> apps;
			int nodeCount;
			switch (TEST_SET_AREA)
			{
			case MeshRouteClass::SQUARE:
				nodeCount = i * i;
				break;
			case MeshRouteClass::HEXAGON:
				nodeCount = 3 * (i - 1) * i + 1;
				break;
			default:
				break;
			}

			Ptr<UniformRandomVariable> randNodes = CreateObject<UniformRandomVariable>();
			randNodes->SetAttribute("Min", DoubleValue(0));
			randNodes->SetAttribute("Max", DoubleValue(nodeCount - 1));

			Ptr<UniformRandomVariable> randTime = CreateObject<UniformRandomVariable>();

			for (int j = 0; j < TEST_SET_COUNT; j++)
			{
				double startTime = TEST_SET_INTERVAL * j + randTime->GetValue(0, TEST_SET_RANDOM);
				uint32_t src = randNodes->GetInteger();
				uint32_t dst;
				do
				{
					dst = randNodes->GetInteger();
				} while (dst == src);
				NodeApplicationInfor newApp = { src, dst, startTime, startTime + TEST_SET_LIFE };
				apps.push_back(newApp);
			}

			totalTime = (TEST_SET_COUNT - 1) * TEST_SET_INTERVAL + TEST_SET_LIFE + TEST_SET_RANDOM;
			NS_LOG_INFO("=============================");
			NS_LOG_INFO("PROTOCOL :MY11S_PMTMGMP   SIZE :" << i);
			NS_LOG_INFO("=============================\n");
			Ptr<MeshRouteClass> pmtmgmp = CreateObject<MeshRouteClass>();
#ifdef TEST_STATS
			SetStats("MY11S_PMTMGMP", i, pmtmgmp);
#endif
			pmtmgmp->SetMeshRouteClass(MeshRouteClass::MY11S_PMTMGMP, apps, totalTime, TEST_SET_AREA, i, i - TEST_SET_SIZE_APPSTEP, TEST_SET_APP);
			pmtmgmp->Run();
			NS_LOG_INFO("=============================");
			NS_LOG_INFO("PROTOCOL :DOT11S_HWMP   SIZE :" << i);
			NS_LOG_INFO("=============================\n"); 
			Ptr<MeshRouteClass> mesh = CreateObject<MeshRouteClass>();
#ifdef TEST_STATS
			SetStats("DOT11S_HWMP", i, mesh);
#endif
			mesh->SetMeshRouteClass(MeshRouteClass::DOT11S_HWMP, apps, totalTime, TEST_SET_AREA, i, i - TEST_SET_SIZE_APPSTEP, TEST_SET_APP);
			mesh->Run();
		}
#else//����ָ��Э��
		string typeName = MeshRouteClass::GetProtocolNameString(TEST_SET_PROTOCOL);
		for (int i = TEST_SET_MIN_SIZE; i <= TEST_SET_MAX_SIZE; i++)
		{
			vector<NodeApplicationInfor> apps;
			int nodeCount;
			switch (TEST_SET_AREA)
			{
			case MeshRouteClass::SQUARE:
				nodeCount = i * i;
				break;
			case MeshRouteClass::HEXAGON:
				nodeCount = 3 * (i - 1) * i + 1;
				break;
			default:
				break;
			}

			Ptr<UniformRandomVariable> randNodes = CreateObject<UniformRandomVariable>();
			randNodes->SetAttribute("Min", DoubleValue(0));
			randNodes->SetAttribute("Max", DoubleValue(nodeCount - 1));

			Ptr<UniformRandomVariable> randTime = CreateObject<UniformRandomVariable>();

			for (int j = 0; j < TEST_SET_COUNT; j++)
			{
				double startTime = TEST_SET_INTERVAL * j + randTime->GetValue(0, TEST_SET_RANDOM);
				uint32_t src = randNodes->GetInteger();
				uint32_t dst;
				do
				{
					dst = randNodes->GetInteger();
				} while (dst == src);
				NodeApplicationInfor newApp = { src, dst, startTime, startTime + TEST_SET_LIFE };
				apps.push_back(newApp);
			}

			totalTime = (TEST_SET_COUNT - 1) * TEST_SET_INTERVAL + TEST_SET_LIFE + TEST_SET_RANDOM;
			NS_LOG_INFO("=============================");
			NS_LOG_INFO("PROTOCOL :" << typeName << "   SIZE :" << i);
			NS_LOG_INFO("=============================\n"); 
			Ptr<MeshRouteClass> test = CreateObject<MeshRouteClass>();
#ifdef TEST_STATS
			SetStats(typeName, i, test);
#endif
			test->SetMeshRouteClass(TEST_SET_PROTOCOL, apps, totalTime, TEST_SET_AREA, i, i - TEST_SET_SIZE_APPSTEP, TEST_SET_APP);
			test->Run();
		}
#endif
//����ָ���߳�
#else
//����ȫ��Э��
#ifdef TEST_SIDE_ALL
		vector<NodeApplicationInfor> apps;
		int nodeCount;
		switch (TEST_SET_AREA)
		{
		case MeshRouteClass::SQUARE:
			nodeCount = TEST_SET_SIZE * TEST_SET_SIZE;
			break;
		case MeshRouteClass::HEXAGON:
			nodeCount = 3 * (TEST_SET_SIZE - 1) * TEST_SET_SIZE + 1;
			break;
		default:
			break;
		}

		Ptr<UniformRandomVariable> randNodes = CreateObject<UniformRandomVariable>();
		randNodes->SetAttribute("Min", DoubleValue(0));
		randNodes->SetAttribute("Max", DoubleValue(nodeCount - 1));

		Ptr<UniformRandomVariable> randTime = CreateObject<UniformRandomVariable>();

		for (int i = 0; i < TEST_SET_COUNT; i++)
		{
			double startTime = TEST_SET_INTERVAL * i + randTime->GetValue(0, TEST_SET_RANDOM);
			uint32_t src = randNodes->GetInteger();
			uint32_t dst;
			do
			{
				dst = randNodes->GetInteger();
			} while (dst == src);
			NodeApplicationInfor newApp = { src, dst, startTime, startTime + TEST_SET_LIFE };
			apps.push_back(newApp);
		}

		totalTime = (TEST_SET_COUNT - 1) * TEST_SET_INTERVAL + TEST_SET_LIFE + TEST_SET_RANDOM;
		//����
		NS_LOG_INFO("=============================");
		NS_LOG_INFO("PROTOCOL :MY11S_PMTMGMP   SIZE :" << TEST_SET_SIZE);
		NS_LOG_INFO("=============================\n");
		Ptr<MeshRouteClass> pmtmgmp = CreateObject<MeshRouteClass>();
#ifdef TEST_STATS
		SetStats("MY11S_PMTMGMP", TEST_SET_SIZE, pmtmgmp);
#endif
		pmtmgmp->SetMeshRouteClass(MeshRouteClass::MY11S_PMTMGMP, apps, totalTime, TEST_SET_AREA, TEST_SET_SIZE, TEST_SET_APPSTEP, TEST_SET_APP);
		pmtmgmp->Run();
		NS_LOG_INFO("=============================");
		NS_LOG_INFO("PROTOCOL :DOT11S_HWMP   SIZE :" << TEST_SET_SIZE);
		NS_LOG_INFO("=============================\n");
		Ptr<MeshRouteClass> mesh = CreateObject<MeshRouteClass>();
#ifdef TEST_STATS
		SetStats("DOT11S_HWMP", TEST_SET_SIZE, mesh);
#endif
		mesh->SetMeshRouteClass(MeshRouteClass::DOT11S_HWMP, apps, totalTime, TEST_SET_AREA, TEST_SET_SIZE, TEST_SET_APPSTEP, TEST_SET_APP);
		mesh->Run();
#else//����ָ��Э��

		string typeName = MeshRouteClass::GetProtocolNameString(TEST_SET_PROTOCOL);
		vector<NodeApplicationInfor> apps;
		int nodeCount;
		switch (TEST_SET_AREA)
		{
		case MeshRouteClass::SQUARE:
			nodeCount = TEST_SET_SIZE * TEST_SET_SIZE;
			break;
		case MeshRouteClass::HEXAGON:
			nodeCount = 3 * (TEST_SET_SIZE - 1) * TEST_SET_SIZE + 1;
			break;
		default:
			break;
		}

		Ptr<UniformRandomVariable> randNodes = CreateObject<UniformRandomVariable>();
		randNodes->SetAttribute("Min", DoubleValue(0));
		randNodes->SetAttribute("Max", DoubleValue(nodeCount - 1));

		Ptr<UniformRandomVariable> randTime = CreateObject<UniformRandomVariable>();

		for (int i = 0; i < TEST_SET_COUNT; i++)
		{
			double startTime = TEST_SET_INTERVAL * i + randTime->GetValue(0, TEST_SET_RANDOM);
			uint32_t src = randNodes->GetInteger();
			uint32_t dst;
			do
			{
				dst = randNodes->GetInteger();
			} while (dst == src);
			NodeApplicationInfor newApp = { src, dst, startTime, startTime + TEST_SET_LIFE };
			apps.push_back(newApp);
		}

		totalTime = (TEST_SET_COUNT - 1) * TEST_SET_INTERVAL + TEST_SET_LIFE + TEST_SET_RANDOM;

		NS_LOG_INFO("=============================");
		NS_LOG_INFO("PROTOCOL :" << typeName << "   SIZE :" << TEST_SET_SIZE);
		NS_LOG_INFO("=============================\n");
		Ptr<MeshRouteClass> test = CreateObject<MeshRouteClass>();
#ifdef TEST_STATS
		SetStats(typeName, TEST_SET_SIZE, test);
#endif
		test->SetMeshRouteClass(TEST_SET_PROTOCOL, apps, totalTime, TEST_SET_AREA, TEST_SET_SIZE, TEST_SET_APPSTEP, TEST_SET_APP);
		test->Run();;
#endif
#endif
	}
	else
	{
#ifdef TEST_ALL
#ifdef TEST_SIDE_ALL
		for (int i = TEST_SET_MIN_SIZE; i <= TEST_SET_MAX_SIZE; i++)
		{
			NS_LOG_INFO("=============================");
			NS_LOG_INFO("PROTOCOL :MY11S_PMTMGMP   SIZE :" << i);
			NS_LOG_INFO("=============================\n");
			Ptr<MeshRouteClass> pmtmgmp = CreateObject<MeshRouteClass>();
#ifdef TEST_STATS
			SetStats("MY11S_PMTMGMP", i, pmtmgmp);
#endif
			pmtmgmp->SetMeshRouteClass(MeshRouteClass::MY11S_PMTMGMP, TEST_SET_AREA, i, i - TEST_SET_SIZE_APPSTEP, TEST_SET_APP);
			pmtmgmp->Run();
			NS_LOG_INFO("=============================");
			NS_LOG_INFO("PROTOCOL :DOT11S_HWMP   SIZE :" << i);
			NS_LOG_INFO("=============================\n");
			Ptr<MeshRouteClass> mesh = CreateObject<MeshRouteClass>();
#ifdef TEST_STATS
			SetStats("DOT11S_HWMP", i, mesh);
#endif
			mesh->SetMeshRouteClass(MeshRouteClass::DOT11S_HWMP, TEST_SET_AREA, i, i - TEST_SET_SIZE_APPSTEP, TEST_SET_APP);
			mesh->Run();
			NS_LOG_INFO("\n");
		};
#else
		NS_LOG_INFO("=============================");
		NS_LOG_INFO("PROTOCOL :MY11S_PMTMGMP   SIZE :" << TEST_SET_SIZE);
		NS_LOG_INFO("=============================\n");
		Ptr<MeshRouteClass> pmtmgmp = CreateObject<MeshRouteClass>();
#ifdef TEST_STATS
		SetStats("MY11S_PMTMGMP", TEST_SET_SIZE, pmtmgmp);
#endif
		pmtmgmp->SetMeshRouteClass(MeshRouteClass::MY11S_PMTMGMP, TEST_SET_AREA, TEST_SET_SIZE, TEST_SET_APPSTEP, TEST_SET_APP);
		pmtmgmp->Run();
		NS_LOG_INFO("=============================");
		NS_LOG_INFO("PROTOCOL :DOT11S_HWMP   SIZE :" << TEST_SET_SIZE);
		NS_LOG_INFO("=============================\n");
		Ptr<MeshRouteClass> mesh = CreateObject<MeshRouteClass>();
#ifdef TEST_STATS
		SetStats("DOT11S_HWMP", TEST_SET_SIZE, mesh);
#endif
		mesh->SetMeshRouteClass(MeshRouteClass::DOT11S_HWMP, TEST_SET_AREA, TEST_SET_SIZE, TEST_SET_APPSTEP, TEST_SET_APP);
		mesh->Run();
#endif
#else
		string typeName = MeshRouteClass::GetProtocolNameString(TEST_SET_PROTOCOL);
		NS_LOG_INFO("=============================");
		NS_LOG_INFO("PROTOCOL :" << typeName << "   SIZE :" << TEST_SET_SIZE);
		NS_LOG_INFO("=============================\n");
		Ptr<MeshRouteClass> test = CreateObject<MeshRouteClass>();
#ifdef TEST_STATS
		SetStats(typeName, TEST_SET_SIZE, test);
#endif
		test->SetMeshRouteClass(TEST_SET_PROTOCOL, TEST_SET_AREA, TEST_SET_SIZE, TEST_SET_APPSTEP, TEST_SET_APP);
		test->Run();
#endif
	}
	return 0;
}
