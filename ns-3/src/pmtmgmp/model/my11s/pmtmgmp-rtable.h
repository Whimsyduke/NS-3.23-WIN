/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008,2009 IITP RAS
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
 * Author: Kirill Andreev <andreev@iitp.ru>
 */

#ifndef PMTMGMP_RTABLE_H
#define PMTMGMP_RTABLE_H

#include <map>
#include "ns3/nstime.h"
#include "ns3/mac48-address.h"
#include "ns3/pmtmgmp-protocol.h"
#include "ns3/traced-value.h"
namespace ns3 {
	namespace my11s {

#ifndef PMTMGMP_UNUSED_MY_CODE
		////·�ɱ�·��
		class PmtmgmpRPpath : public Object
		{
		public:
			PmtmgmpRPpath();
			~PmtmgmpRPpath(); 
			static TypeId GetTypeId();

			////·��·��״̬
			enum RouteInformationStatus
			{
				////����
				Expired,
				////�ȴ�
				Waited,
				////ȷ��
				Confirmed,
			};

			////��ѡ��Ϣ
			void AddCandidateRouteInformaiton(Ptr<PmtmgmpRPpath> path);
			void ClearCandidateRouteInformaiton();
			std::vector<Ptr<PmtmgmpRPpath>> GetCandidateRouteInformaiton();

#ifdef ROUTE_USE_PART_PATH ////��ʹ�ò���·��
			std::vector<Mac48Address> GetPathInfo() const;
#endif
			void SetMTERPaddress(Mac48Address address);
			void SetMSECPaddress(Mac48Address address);
			void SetPathGenerationSequenceNumber(uint32_t seq_number);
			void SetNodeType(PmtmgmpProtocol::NodeType nodeType);
			void SetHopcount(uint8_t hopcount);
			void SetMetric(double metric);
			void SetFromNode(Mac48Address address);
			void SetStatus(RouteInformationStatus status);

			Mac48Address GetMTERPaddress() const;
			Mac48Address GetMSECPaddress() const;
			uint32_t GetPathGenerationSequenceNumber() const;
			PmtmgmpProtocol::NodeType GetNodeType() const;
			uint8_t  GetHopCount() const;
			double GetMetric() const;
			Mac48Address GetFromNode() const;
			RouteInformationStatus GetStatus() const;

#ifdef ROUTE_USE_PART_PATH ////��ʹ�ò���·��
			////��ȡ·���ظ���
			uint8_t GetPartPathRepeatability(PmtmgmpRPpath compare);
			////��ӽڵ�
			void AddPartPathNode(Mac48Address address);
			////��ȡ·������ַ
			Mac48Address GetNodeMac(int8_t index);
			////��ȡ·�����������
			uint8_t GetMaxNodeListNum();
			////��ȡ·����ǰ�������
			uint8_t GetCurrentNodeListNum();
#endif

			////������
			Ptr<PmtmgmpRPpath> GetCopy();
		private:
#ifdef ROUTE_USE_PART_PATH ////��ʹ�ò���·��
			////·�ɱ�·��������
			struct PmtmgmpRPpath_Finder
			{
				PmtmgmpRPpath_Finder(Mac48Address address) :m_address(address) {};
				bool operator()(Mac48Address p)
				{
					return m_address == p;
				}
				Mac48Address m_address;
			};
#endif
		private:
			Mac48Address m_MTERPaddress;
			Mac48Address m_MSECPaddress;
			uint32_t m_PathGenerationSeqNumber;
			PmtmgmpProtocol::NodeType m_NodeType;
			uint8_t  m_hopCount;
			double m_metric;
#ifdef ROUTE_USE_PART_PATH ////��ʹ�ò���·��
			std::vector<Mac48Address>  m_PathInfo;

			////�������Ĳ���·������
			uint8_t m_PMTMGMPpathNodeListNum;
#endif
			////��ѡ·����Ϣ�б�
			std::vector<Ptr<PmtmgmpRPpath>>  m_CandidateRouteInformaiton;
			RouteInformationStatus m_InformationStatus;
			
			////��Դ�ڵ�MAC
			Mac48Address m_FormNode;
		};

		////·�ɱ���
		class PmtmgmpRPtree : public Object
		{
		public:
			PmtmgmpRPtree();
			~PmtmgmpRPtree();
			static TypeId GetTypeId();
			std::vector<Ptr<PmtmgmpRPpath>> GetPartTree();

			void SetMTERPaddress(Mac48Address address);
			void SetAcceptCandidateRouteInformaitonEvent(EventId id);

			Mac48Address GetMTERPaddress() const;
			EventId GetAcceptCandidateRouteInformaitonEvent() const;
			Time GetAcceptInformaitonDelay() const;
			
			////��ȡMTERP��MSECP��Ӧ��Path
			Ptr<PmtmgmpRPpath> GetPathByMACaddress(Mac48Address msecp);
			////��ȡ������С��·��
			std::vector<Ptr<PmtmgmpRPpath>> GetBestPath();
			////��ȡ������С��·��
			std::vector<Ptr<PmtmgmpRPpath>> GetBestPath(std::vector<Ptr<PmtmgmpRPpath>> pathList);
			////�����·��
			void AddNewPath(Ptr<PmtmgmpRPpath> path);
			////��ȡ��ǰ·�����������˳���
			uint32_t GetTreeMaxGenerationSeqNumber();
			////ȫ��·����Ϊ����
			void SetAllStatusExpired();
			////��ȡ�ظ���
			uint8_t GetRepeatability(Mac48Address from);
			////�����ظ��ȼ���
			void RepeatabilityIncrease(Mac48Address from);
			////����ȫ���ظ���
			void RepeatabilityReset();
			////����ĳ����ѡ��Ϣ
			void AcceptCandidateRouteInformaiton(Mac48Address address);
		private:
			////·�ɱ���������
			struct PmtmgmpRPtree_Finder
			{
				PmtmgmpRPtree_Finder(Mac48Address msecp) :m_msecp(msecp){};
				bool operator()(Ptr<PmtmgmpRPpath> p)
				{
					return m_msecp == p->GetMSECPaddress();
				}
				Mac48Address m_msecp;
			};
		private:
			std::vector<Ptr<PmtmgmpRPpath>>  m_partTree;
			Mac48Address m_MTERPaddress;

			////�ն˽ڵ��ӵ�еĸ����ڵ�����
			uint8_t m_MSECPnumForMTERP;

			////·���ظ��ȼ���
			std::map<Mac48Address, uint8_t> m_repeatability;

			////�ӳٵȴ�����·����Ϣ�¼�
			EventId m_AcceptCandidateRouteInformaitonEvent;
			Time m_AcceptInformaitonDelay;
		};

		////·�ɱ�
		class PmtmgmpRPRouteTable : public Object
		{
		public:
			PmtmgmpRPRouteTable();
			~PmtmgmpRPRouteTable();

			std::vector<Ptr<PmtmgmpRPtree>> GetRouteTable();

			////��ȡMTERP��Ӧ��Tree
			Ptr<PmtmgmpRPtree> GetTreeByMACaddress(Mac48Address mterp);
			////��ȡMTERP��MSECP��Ӧ��Path
			Ptr<PmtmgmpRPpath> GetPathByMACaddress(Mac48Address mterp, Mac48Address msecp);
			////�����·��
			void AddNewPath(Ptr<PmtmgmpRPpath> path);
		private:
			////·�ɱ�·��������
			struct PmtmgmpRProuteTable_Finder
			{
				PmtmgmpRProuteTable_Finder(Mac48Address address) :m_address(address) {};
				bool operator()(Ptr<PmtmgmpRPtree> p)
				{
					return m_address == p->GetMTERPaddress();
				}
				Mac48Address m_address;
			};
		private:
			std::vector<Ptr<PmtmgmpRPtree>>  m_partTable;
		};
#endif
		/**
		 * \ingroup my11s
		 *
		 * \brief Routing table for PMTMGMP -- 802.11s routing protocol
		 */
		class PmtmgmpRtable : public Object
		{
		public:
			/// Means all interfaces
			const static uint32_t INTERFACE_ANY = 0xffffffff;
			/// Maximum (the best?) path metric
			const static uint32_t MAX_METRIC = 0xffffffff;

			/// Route lookup result, return type of LookupXXX methods
			struct LookupResult
			{
				Mac48Address retransmitter;
				uint32_t ifIndex;
				uint32_t metric;
				uint32_t seqnum;
				Time lifetime;
				LookupResult(Mac48Address r = Mac48Address::GetBroadcast(),
					uint32_t i = INTERFACE_ANY,
					uint32_t m = MAX_METRIC,
					uint32_t s = 0,
					Time l = Seconds(0.0));
				/// True for valid route
				bool IsValid() const;
				/// Compare route lookup results, used by tests
				bool operator== (const LookupResult & o) const;
			};
			/// Path precursor = {MAC, interface ID}
			typedef std::vector<std::pair<uint32_t, Mac48Address> > PrecursorList;

		public:
			static TypeId GetTypeId();
			PmtmgmpRtable();
			~PmtmgmpRtable();
			void DoDispose();

			///\name Add/delete paths
			//\{
			void AddReactivePath(
				Mac48Address destination,
				Mac48Address retransmitter,
				uint32_t interface,
				uint32_t metric,
				Time  lifetime,
				uint32_t seqnum
				);
			void AddProactivePath(
				uint32_t metric,
				Mac48Address root,
				Mac48Address retransmitter,
				uint32_t interface,
				Time  lifetime,
				uint32_t seqnum
				);
			void AddPrecursor(Mac48Address destination, uint32_t precursorInterface, Mac48Address precursorAddress, Time lifetime);
			PrecursorList GetPrecursors(Mac48Address destination);
			void DeleteProactivePath();
			void DeleteProactivePath(Mac48Address root);
			void DeleteReactivePath(Mac48Address destination);
			//\}

			///\name Lookup
			//\{
			/// Lookup path to destination
			LookupResult LookupReactive(Mac48Address destination);
			/// Return all reactive paths, including expired
			LookupResult LookupReactiveExpired(Mac48Address destination);
			/// Find proactive path to tree root. Note that calling this method has side effect of deleting expired proactive path
			LookupResult LookupProactive();
			/// Return all proactive paths, including expired
			LookupResult LookupProactiveExpired();
			//\}

			/// When peer link with a given MAC-address fails - it returns list of unreachable destination addresses
			std::vector<PmtmgmpProtocol::FailedDestination> GetUnreachableDestinations(Mac48Address peerAddress);

		private:
			/// Route found in reactive mode
			struct Precursor
			{
				Mac48Address address;
				uint32_t interface;
				Time whenExpire;
			};
			struct ReactiveRoute
			{
				Mac48Address retransmitter;
				uint32_t interface;
				uint32_t metric;
				Time whenExpire;
				uint32_t seqnum;
				std::vector<Precursor> precursors;
			};
			/// Route fond in proactive mode
			struct ProactiveRoute
			{
				Mac48Address root;
				Mac48Address retransmitter;
				uint32_t interface;
				uint32_t metric;
				Time whenExpire;
				uint32_t seqnum;
				std::vector<Precursor> precursors;
			};

			/// List of routes
			std::map<Mac48Address, ReactiveRoute>  m_routes;
			/// Path to proactive tree root MP
			ProactiveRoute  m_root;
		};
	} // namespace my11s
} // namespace ns3
#endif
