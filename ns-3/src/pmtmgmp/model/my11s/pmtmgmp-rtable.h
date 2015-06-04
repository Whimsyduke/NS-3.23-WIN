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

			uint8_t GetPartPathRepeatability(PmtmgmpRPpath compare);

			void SetMTERPaddress(Mac48Address address);
			void SetMSECPaddress(Mac48Address address);
			void SetMetric(uint32_t metric);

			Mac48Address GetMTERPaddress() const;
			Mac48Address GetMSECPaddress() const;
			std::vector<Mac48Address> GetPartPath() const;
			uint32_t GetMetric() const;

			////��ӽڵ�
			void AddPartPathNode(Mac48Address address);
			////��ȡ·������ַ
			Mac48Address GetNodeMac(int8_t index);
			////��ȡ·�����������
			uint8_t GetMaxNodeListNum();
			////��ȡ·����ǰ�������
			uint8_t GetCurrentNodeListNum();
		private:
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
		private:
			std::vector<Mac48Address>  m_partPath;
			Mac48Address m_MTERPaddress;
			Mac48Address m_MSECPaddress; 
			uint32_t m_metric;

			////�������Ĳ���·������
			uint8_t m_PMTMGMPpathNodeListNum;
		};
		////·�ɱ���
		class PmtmgmpRPtree : public Object
		{
		public:
			PmtmgmpRPtree();
			~PmtmgmpRPtree();

			void SetMTERPaddress(Mac48Address address);

			Mac48Address GetMTERPaddress() const;

			Ptr<PmtmgmpRPpath> GetPathByMACaddress(Mac48Address mterp, Mac48Address msecp);
			std::vector<Ptr<PmtmgmpRPpath>> GetBestPath();
		private:
			////·�ɱ���������
			struct PmtmgmpRPtree_Finder
			{
				PmtmgmpRPtree_Finder(Mac48Address mterp, Mac48Address msecp) :m_mterp(mterp), m_msecp(msecp){};
				bool operator()(Ptr<PmtmgmpRPpath> p)
				{
					return m_mterp == p->GetMTERPaddress() && m_msecp == p->GetMSECPaddress();
				}
				Mac48Address m_mterp;
				Mac48Address m_msecp;
			};
		private:
			std::vector<Ptr<PmtmgmpRPpath>>  m_partTree;
			Mac48Address m_MTERPaddress;
		};
		////·�ɱ�
		class PmtmgmpRPRouteTable : public Object
		{
		public:
			PmtmgmpRPRouteTable();
			~PmtmgmpRPRouteTable();
		private:
			////·�ɱ�·��������
			struct PmtmgmpRPtree_Finder
			{
				PmtmgmpRPtree_Finder(Mac48Address address) :m_address(address) {};
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
