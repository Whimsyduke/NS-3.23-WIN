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
 * Author: Pavel Boyko <boyko@iitp.ru>
 */

#include "meshnet-information-element-vector.h"
#include "ns3/packet.h"
#include <algorithm>
#include "ns3/mgmp-protocol.h"
// All information elements:
#include "ns3/ie-hu11s-beacon-timing.h"
#include "ns3/ie-hu11s-configuration.h"
#include "ns3/ie-hu11s-id.h"
#include "ns3/ie-hu11s-metric-report.h"
#include "ns3/ie-hu11s-huper-management.h"
#include "ns3/ie-hu11s-hupering-protocol.h"
#include "ns3/ie-hu11s-perr.h"
#include "ns3/ie-hu11s-prep.h"
#include "ns3/ie-hu11s-preq.h"
#include "ns3/ie-hu11s-rann.h"


namespace ns3 {

	uint32_t
		MeshnetInformationElementVector::DeserializeSingleIe(Buffer::Iterator start)
	{
		Buffer::Iterator i = start;
		uint8_t id = i.ReadU8();
		uint8_t length = i.ReadU8();
		Ptr<WifiInformationElement> newElement;
		switch (id)
		{
		case IE11S_MESHNET_CONFIGURATION:
			newElement = Create<hu11s::IeConfiguration>();
			break;
		case IE11S_MESHNET_ID:
			newElement = Create<hu11s::IeMeshnetId>();
			break;
		case IE11S_LINK_METRIC_REPORT:
			newElement = Create<hu11s::IeLinkMetricReport>();
			break;
		case IE11S_MGMP_HUPERING_MANAGEMENT:
			newElement = Create<hu11s::IeHuperManagement>();
			break;
		case IE11S_BEACON_TIMING:
			newElement = Create<hu11s::IeBeaconTiming>();
			break;
		case IE11S_RANN:
			newElement = Create<hu11s::IeRann>();
			break;
		case IE11S_PREQ:
			newElement = Create<hu11s::IePreq>();
			break;
		case IE11S_PREP:
			newElement = Create<hu11s::IePrep>();
			break;
		case IE11S_PERR:
			newElement = Create<hu11s::IePerr>();
			break;
		case IE11S_MESHNET_MGMP_HUPERING_PROTOCOL_VERSION:
			newElement = Create<hu11s::IeHuperingProtocol>();
			break;
		default:
			// We peeked at the ID and length, so we need to back up the
			// pointer before deferring to our parent.
			i.Prev(2);
			return WifiInformationElementVector::DeserializeSingleIe(i);
		}
		if (GetSize() + length > m_maxSize)
		{
			NS_FATAL_ERROR("Check max size for information element!");
		}
		newElement->DeserializeInformationField(i, length);
		i.Next(length);
		m_elements.push_back(newElement);
		return i.GetDistanceFrom(start);
	}


} // namespace ns3
