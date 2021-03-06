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
* Author: Whimsy Duke <whimsyduke@163.com>
*/

#ifndef PMTMGMP_UNUSED_MY_CODE
#ifndef PMTMGMP_WIFI_PGEN_INFORMATION_ELEMENT_H
#define PMTMGMP_WIFI_PGEN_INFORMATION_ELEMENT_H

#include "ns3/mac48-address.h"
#include "ns3/wmn-information-element-vector.h"
#include "ns3/pmtmgmp-protocol.h"
#include "ns3/pmtmgmp-rtable.h"

namespace ns3 {
	namespace my11s {
		class IePgen : public WifiInformationElement
		{
		public:
			IePgen();
			IePgen(Ptr<PmtmgmpRoutePath> path);
			~IePgen();

			Ptr<PmtmgmpRoutePath> GetPathInfo();
			
			// Setters for fields:
			void SetMTERPaddress(Mac48Address originator_address);
			void SetMSECPaddress(Mac48Address mSECP_address);
			void SetMSECPindex(uint8_t index);
			void SetPathGenerationSequenceNumber(uint32_t seq_number);
			void SetNodeType(PmtmgmpProtocol::NodeType nodeType);
			void SetHopcount(uint8_t hopcount);
			void SetTTL(uint8_t ttl);
			void SetMetric(uint32_t metric);

			// Getters for fields:
			Mac48Address GetMTERPaddress() const;
			Mac48Address GetMSECPaddress() const;
			uint8_t GetMSECPindex() const;
			uint32_t GetPathGenerationSequenceNumber() const;
			PmtmgmpProtocol::NodeType GetNodeType() const;
			uint8_t  GetHopCount() const;
			uint8_t  GetTtl() const;
			uint32_t GetMetric() const;
			Ptr<PmtmgmpRoutePath> GetPathInfo() const;
			
			// Inherited from WifiInformationElement
			virtual WifiInformationElementId ElementId() const;
			virtual void SerializeInformationField(Buffer::Iterator i) const;
			virtual uint8_t DeserializeInformationField(Buffer::Iterator i, uint8_t length);
			virtual uint8_t GetInformationFieldSize() const;
			virtual void Print(std::ostream& os) const;
		private:
			Ptr<PmtmgmpRoutePath> m_PathInfo;

			friend bool operator== (const IePgen & a, const IePgen & b);
		};
		bool operator== (const IePgen & a, const IePgen & b);
		std::ostream &operator << (std::ostream &os, const IePgen &pgen);
	}
}
#endif
#endif