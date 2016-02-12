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
 * Authors: Kirill Andreev <andreev@iitp.ru>
 *          Aleksey Kovalenko <kovalenko@iitp.ru>
 */

#include "ie-hu11s-configuration.h"
#include "ns3/packet.h"
namespace ns3 {
namespace hu11s {

Hu11sMeshnetCapability::Hu11sMeshnetCapability () :
  acceptHuperLinks (true), MCCASupported (false), MCCAEnabled (false), forwarding (true), beaconTimingReport (
    true), TBTTAdjustment (true), powerSaveLevel (false)
{
}
uint8_t
Hu11sMeshnetCapability::GetSerializedSize () const
{
  return 1; 
}
uint8_t  
Hu11sMeshnetCapability::GetUint8 () const  //IEEE 802.11-2012 8.4.2.100.8 Meshnet Capability 
{
  uint8_t result = 0;  
  if (acceptHuperLinks)
    {
      result |= 1 << 0; //The Accepting Additional Meshnet Huperings subfield is set to 1 if the meshnet STA is willing to establish additional meshnet huperings   with other meshnet STAs and set to 0 otherwise
    }
  if (MCCASupported) // The MCCA Supported subfield is set to 1 if the meshnet STA implements MCCA and set to 0 otherwise
    {
      result |= 1 << 1;
    }
  if (MCCAEnabled)
    {
      result |= 1 << 2;
    }
  if (forwarding)
    {
      result |= 1 << 3;
    }
  if (beaconTimingReport)
    {
      result |= 1 << 4;
    }
  if (TBTTAdjustment)
    {
      result |= 1 << 5;
    }
  if (powerSaveLevel)
    {
      result |= 1 << 6;
    }
  return result;
}
Buffer::Iterator
Hu11sMeshnetCapability::Serialize (Buffer::Iterator i) const
{
  i.WriteU8 (GetUint8 ());
  return i;
}
Buffer::Iterator
Hu11sMeshnetCapability::Deserialize (Buffer::Iterator i)
{
  uint8_t cap = i.ReadU8 ();
  acceptHuperLinks = Is (cap, 0);
  MCCASupported = Is (cap, 1);
  MCCAEnabled = Is (cap, 2);
  forwarding = Is (cap, 3);
  beaconTimingReport = Is (cap, 4);
  TBTTAdjustment = Is (cap, 5);
  powerSaveLevel = Is (cap, 6);
  return i;
}
bool
Hu11sMeshnetCapability::Is (uint8_t cap, uint8_t n) const
{
  uint16_t mask = 1 << n;
  return (cap & mask);
}
WifiInformationElementId
IeConfiguration::ElementId () const
{
  return IE11S_MESHNET_CONFIGURATION;
}

IeConfiguration::IeConfiguration () :
  m_APSPId (PROTOCOL_MGMP), m_APSMId (METRIC_AIRTIME), m_CCMId (CONGESTION_NULL), m_SPId (
    SYNC_NEIGHBOUR_OFFSET), m_APId (AUTH_NULL), m_neighbors (0)
{
}
uint8_t
IeConfiguration::GetInformationFieldSize () const
{
   return 0   // Version
         + 1  // APSPId
         + 1 // APSMId
         + 1 // CCMId
         + 1 // SPId
         + 1 // APId
         + 1 // Meshnet formation info (see 7.3.2.86.6 of 802.11s draft 3.0)
         + m_meshnetCap.GetSerializedSize ();
}
void
IeConfiguration::SerializeInformationField (Buffer::Iterator i) const
{
  // Active Path Selection Protocol ID:
  i.WriteU8 (m_APSPId);
  // Active Path Metric ID:
  i.WriteU8 (m_APSMId);
  // Congestion Control Mode ID:
  i.WriteU8 (m_CCMId);
  // Sync:
  i.WriteU8 (m_SPId);
  // Auth:
  i.WriteU8 (m_APId);
  i.WriteU8 (m_neighbors << 1);
  m_meshnetCap.Serialize (i);
}
uint8_t
IeConfiguration::DeserializeInformationField (Buffer::Iterator i, uint8_t length)
{
  Buffer::Iterator start = i;
  // Active Path Selection Protocol ID:
  m_APSPId = (hu11sPathSelectionProtocol) i.ReadU8 ();
  // Active Path Metric ID:
  m_APSMId = (hu11sPathSelectionMetric) i.ReadU8 ();
  // Congestion Control Mode ID:
  m_CCMId = (hu11sCongestionControlMode) i.ReadU8 ();
  m_SPId = (hu11sSynchronizationProtocolIdentifier) i.ReadU8 ();
  m_APId = (hu11sAuthenticationProtocol) i.ReadU8 ();
  m_neighbors = (i.ReadU8 () >> 1) & 0xF;
  i = m_meshnetCap.Deserialize (i);
  return i.GetDistanceFrom (start);
}
void
IeConfiguration::Print (std::ostream& os) const
{
  os << std::endl << "<information_element id=" << ElementId () << ">" << std::endl;
  os << "Number of neighbors:               = " << (uint16_t) m_neighbors
     << std::endl << "Active Path Selection Protocol ID: = " << (uint32_t) m_APSPId
     << std::endl << "Active Path Selection Metric ID:   = " << (uint32_t) m_APSMId
     << std::endl << "Congestion Control Mode ID:        = " << (uint32_t) m_CCMId
     << std::endl << "Synchronize protocol ID:           = " << (uint32_t) m_SPId
     << std::endl << "Authentication protocol ID:        = " << (uint32_t) m_APId
     << std::endl << "Capabilities:                      = " << m_meshnetCap.GetUint8 () << std::endl;
  os << "</information_element>" << std::endl;
}
void
IeConfiguration::SetRouting (hu11sPathSelectionProtocol routingId)
{
  m_APSPId = routingId;
}
void
IeConfiguration::SetMetric (hu11sPathSelectionMetric metricId)
{
  m_APSMId = metricId;
}
bool
IeConfiguration::IsMGMP ()
{
  return (m_APSPId == PROTOCOL_MGMP);
}
bool
IeConfiguration::IsAirtime ()
{
  return (m_APSMId == METRIC_AIRTIME);
}
void
IeConfiguration::SetNeighborCount (uint8_t neighbors)
{
  m_neighbors = (neighbors > 31) ? 31 : neighbors;
}
uint8_t
IeConfiguration::GetNeighborCount ()
{
  return m_neighbors;
}
Hu11sMeshnetCapability const&
IeConfiguration::MeshnetCapability ()
{
  return m_meshnetCap;
}
bool
operator== (const Hu11sMeshnetCapability & a, const Hu11sMeshnetCapability & b)
{
  return ((a.acceptHuperLinks == b.acceptHuperLinks) && (a.MCCASupported == b.MCCASupported) && (a.MCCAEnabled
                                                                                               == b.MCCAEnabled) && (a.forwarding == b.forwarding) && (a.beaconTimingReport == b.beaconTimingReport)
          && (a.TBTTAdjustment == b.TBTTAdjustment) && (a.powerSaveLevel == b.powerSaveLevel));
}
bool
operator== (const IeConfiguration & a, const IeConfiguration & b)
{
  return ((a.m_APSPId == b.m_APSPId) && (a.m_APSMId == b.m_APSMId) && (a.m_CCMId == b.m_CCMId) && (a.m_SPId
                                                                                                   == b.m_SPId) && (a.m_APId == b.m_APId) && (a.m_neighbors == b.m_neighbors) && (a.m_meshnetCap
                                                                                                                                                                                  == b.m_meshnetCap));
}
std::ostream &
operator << (std::ostream &os, const IeConfiguration &a)
{
  a.Print (os);
  return os;
}
} // namespace hu11s
} // namespace ns3
