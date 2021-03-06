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
 * Authors: Kirill Andreev <andreev@iitp.ru>
 */

#include "huper-link-frame.h"
#include "ie-hu11s-huper-management.h"
#include "ns3/meshnet-wifi-interface-mac.h"
#include "ns3/test.h"
#include "ns3/packet.h"

namespace ns3 {
namespace hu11s {
NS_OBJECT_ENSURE_REGISTERED (HuperLinkFrameStart);

HuperLinkFrameStart::HuperLinkFrameStart () :
  m_subtype (255), m_capability (0), m_aid (0), m_rates (SupportedRates ()), m_meshnetId (),
  m_config (IeConfiguration ())
{
}
void
HuperLinkFrameStart::SetPlinkFrameSubtype (uint8_t subtype)
{
  m_subtype = subtype;
}
void
HuperLinkFrameStart::SetPlinkFrameStart (HuperLinkFrameStart::PlinkFrameStartFields fields)
{
  m_subtype = fields.subtype;
  
  if (m_subtype != (uint8_t)(WifiActionHeader::PEER_LINK_CLOSE))
    {
      m_capability = fields.capability;
    }
  if (m_subtype == (uint8_t)(WifiActionHeader::PEER_LINK_CONFIRM))
    {
      m_aid = fields.aid;
    }
  if (m_subtype != (uint8_t)(WifiActionHeader::PEER_LINK_CLOSE))
    {
      m_rates = fields.rates;
    }
  if (m_subtype != (uint8_t)(WifiActionHeader::PEER_LINK_CONFIRM))
    {
      m_meshnetId = fields.meshnetId;
    }
  if (m_subtype != (uint8_t)(WifiActionHeader::PEER_LINK_CLOSE))
    {
      m_config = fields.config;
    }
  else
    {
      //reasonCode not used here
    }
}
HuperLinkFrameStart::PlinkFrameStartFields
HuperLinkFrameStart::GetFields () const
{
  PlinkFrameStartFields retval;
  /// \todo protocol version:
  retval.subtype = m_subtype;
  retval.capability = m_capability;
  retval.aid = m_aid;
  retval.rates = m_rates;
  retval.meshnetId = m_meshnetId;
  retval.config = m_config;
  
  return retval;
}
TypeId
HuperLinkFrameStart::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::hu11s::HuperLinkFrameStart")
    .SetParent<Header> ()
    .SetGroupName ("Meshnet")
    .AddConstructor<HuperLinkFrameStart> ()
  ;
  return tid;
}
TypeId
HuperLinkFrameStart::GetInstanceTypeId () const
{
  return GetTypeId ();
}
void
HuperLinkFrameStart::Print (std::ostream &os) const
{
  os << "subtype = " << (uint16_t) m_subtype << std::endl << "capability = " << m_capability << std::endl << "laid = "
     << (uint16_t) m_aid << std::endl << "rates = " << m_rates << std::endl << "meshnetId = ";
  m_meshnetId.Print (os);
  os << std::endl << "configuration = ";
  m_config.Print (os);
  os << std::endl << "reason code = " << m_reasonCode;
}
uint32_t
HuperLinkFrameStart::GetSerializedSize () const
{
  uint32_t size =0; //Hupering protocol
  NS_ASSERT (m_subtype < 4);
  if ((uint8_t)(WifiActionHeader::PEER_LINK_CLOSE) != m_subtype)
    {
      size += 2; //capability
    }
  if ((uint8_t)(WifiActionHeader::PEER_LINK_CONFIRM) == m_subtype)
    {
      size += 2; //AID of remote huper
    }
  if ((uint8_t)(WifiActionHeader::PEER_LINK_CLOSE) != m_subtype)
    {
      size += m_rates.GetSerializedSize ();
      size += m_rates.extended.GetSerializedSize ();
    }
  if ((uint8_t)(WifiActionHeader::PEER_LINK_CONFIRM) != m_subtype)
    {
      size += m_meshnetId.GetInformationFieldSize () + 2;
    }
  if ((uint8_t)(WifiActionHeader::PEER_LINK_CLOSE) != m_subtype)
    {
      size += m_config.GetInformationFieldSize () + 2;
    }
  else
    {
      //reasonCode not used here
    }
  return size;
}
void
HuperLinkFrameStart::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  NS_ASSERT (m_subtype < 4);
  
  if ((uint8_t)(WifiActionHeader::PEER_LINK_CLOSE) != m_subtype)
    {
      i.WriteHtolsbU16 (m_capability);
    }
  if ((uint8_t)(WifiActionHeader::PEER_LINK_CONFIRM) == m_subtype)
    {
      i.WriteHtolsbU16 (m_aid);
    }
  if ((uint8_t)(WifiActionHeader::PEER_LINK_CLOSE) != m_subtype)
    {
      i = m_rates.Serialize (i);
      i = m_rates.extended.Serialize (i);
    }
  if ((uint8_t)(WifiActionHeader::PEER_LINK_CONFIRM) != m_subtype)
    {
      i = m_meshnetId.Serialize (i);
    }
  if ((uint8_t)(WifiActionHeader::PEER_LINK_CLOSE) != m_subtype)
    {
      i = m_config.Serialize (i);
    }
  else
    {
      //reasonCode not used here
    }
}
uint32_t
HuperLinkFrameStart::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  NS_ASSERT (m_subtype < 4);
  
  if ((uint8_t)(WifiActionHeader::PEER_LINK_CLOSE) != m_subtype)
    {
      m_capability = i.ReadLsbtohU16 ();
    }
  if ((uint8_t)(WifiActionHeader::PEER_LINK_CONFIRM) == m_subtype)
    {
      m_aid = i.ReadLsbtohU16 ();
    }
  if ((uint8_t)(WifiActionHeader::PEER_LINK_CLOSE) != m_subtype)
    {
      i = m_rates.Deserialize (i);
      i = m_rates.extended.DeserializeIfPresent (i);
    }
  if ((uint8_t)(WifiActionHeader::PEER_LINK_CONFIRM) != m_subtype)
    {
      uint8_t id = i.ReadU8 ();
      uint8_t length = i.ReadU8 ();
      m_meshnetId.DeserializeInformationField (i, length);
      if ((m_meshnetId.ElementId () != (WifiInformationElementId) id) || (m_meshnetId.GetInformationFieldSize () != length))
        {
          NS_FATAL_ERROR ("Broken frame: Element ID does not match IE itself!");
        }
      i.Next (m_meshnetId.GetInformationFieldSize ());
    }
  if ((uint8_t)(WifiActionHeader::PEER_LINK_CLOSE) != m_subtype)
    {
      uint8_t id = i.ReadU8 ();
      uint8_t length = i.ReadU8 ();
      m_config.DeserializeInformationField (i, length);
      if ((m_config.ElementId () != (WifiInformationElementId) id) || (m_config.GetInformationFieldSize () != length))
        {
          NS_FATAL_ERROR ("Broken frame: Element ID does not match IE itself!");
        }
      i.Next (m_config.GetInformationFieldSize ());
    }
  else
    {
       //reasonCode not used here
    }
  return i.GetDistanceFrom (start);
}
bool
operator== (const HuperLinkFrameStart & a, const HuperLinkFrameStart & b)
{
  return ((a.m_subtype == b.m_subtype) && (a.m_capability == b.m_capability) && (a.m_aid == b.m_aid)
          && (a.m_meshnetId.IsEqual (b.m_meshnetId)) && (a.m_config == b.m_config));
  
}
} // namespace hu11s
} // namespace ns3

