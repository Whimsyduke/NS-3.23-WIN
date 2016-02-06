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
 * Author: Kirill Andreev <andreev@iitp.ru>
 */

#include "ns3/assert.h"
#include "ns3/address-utils.h"
#include "hu11s-mac-header.h"
#include "ns3/packet.h"

namespace ns3 {
namespace hu11s {
/***********************************************************
 *  Here Meshnet Mac Header functionality is defined.
 ***********************************************************/
TypeId
MeshnetHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Hu11sMacHeader")
    .SetParent<Header> ()
    .SetGroupName ("Meshnet")
    .AddConstructor<MeshnetHeader> ();
  return tid;
}
MeshnetHeader::MeshnetHeader () :
  m_meshnetFlags (0), m_meshnetTtl (0), m_meshnetSeqno (0), m_addr4 (Mac48Address ()), m_addr5 (Mac48Address ()),
  m_addr6 (Mac48Address ())
{
}
MeshnetHeader::~MeshnetHeader ()
{
}
TypeId
MeshnetHeader::GetInstanceTypeId () const
{
  return GetTypeId ();
}
void
MeshnetHeader::SetAddr4 (Mac48Address address)
{
  m_addr4 = address;
}
void
MeshnetHeader::SetAddr5 (Mac48Address address)
{
  m_addr5 = address;
}
void
MeshnetHeader::SetAddr6 (Mac48Address address)
{
  m_addr6 = address;
}
Mac48Address
MeshnetHeader::GetAddr4 () const
{
  return m_addr4;
}
Mac48Address
MeshnetHeader::GetAddr5 () const
{
  return m_addr5;
}
Mac48Address
MeshnetHeader::GetAddr6 () const
{
  return m_addr6;
}
void
MeshnetHeader::SetMeshnetSeqno (uint32_t seqno)
{
  m_meshnetSeqno = seqno;
}
uint32_t
MeshnetHeader::GetMeshnetSeqno () const
{
  return m_meshnetSeqno;
}
void
MeshnetHeader::SetMeshnetTtl (uint8_t TTL)
{
  m_meshnetTtl = TTL;
}
uint8_t
MeshnetHeader::GetMeshnetTtl () const
{
  return m_meshnetTtl;
}
void
MeshnetHeader::SetAddressExt (uint8_t num_of_addresses)
{
  NS_ASSERT (num_of_addresses <= 3);
  m_meshnetFlags |= 0x03 & num_of_addresses;
}
uint8_t
MeshnetHeader::GetAddressExt () const
{
  return (0x03 & m_meshnetFlags);
}
uint32_t
MeshnetHeader::GetSerializedSize () const
{
  return 6 + GetAddressExt () * 6;
}
void
MeshnetHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteU8 (m_meshnetFlags);
  i.WriteU8 (m_meshnetTtl);
  i.WriteHtolsbU32 (m_meshnetSeqno);
  uint8_t addresses_to_add = GetAddressExt ();
  //Writing Address extensions:
  if ((addresses_to_add == 1) || (addresses_to_add == 3))
    {
      WriteTo (i, m_addr4);
    }
  if (addresses_to_add > 1)
    {
      WriteTo (i, m_addr5);
    }
  if (addresses_to_add > 1)
    {
      WriteTo (i, m_addr6);
    }
}
uint32_t
MeshnetHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  uint8_t addresses_to_read = 0;
  m_meshnetFlags = i.ReadU8 ();
  m_meshnetTtl = i.ReadU8 ();
  m_meshnetSeqno = i.ReadLsbtohU32 ();
  addresses_to_read = m_meshnetFlags & 0x03;
  if ((addresses_to_read == 1) || (addresses_to_read == 3))
    {
      ReadFrom (i, m_addr4);
    }
  if (addresses_to_read > 1)
    {
      ReadFrom (i, m_addr5);
    }
  if (addresses_to_read > 1)
    {
      ReadFrom (i, m_addr6);
    }
  return i.GetDistanceFrom (start);
}
void
MeshnetHeader::Print (std::ostream &os) const
{
  os << "flags = " << (uint16_t) m_meshnetFlags << std::endl << "ttl = " << (uint16_t) m_meshnetTtl 
     << std::endl << "seqno = " << m_meshnetSeqno << std::endl<< "addr4 = " << m_addr4 << std::endl
     << "addr5 = " << m_addr5 << std::endl << "addr6 = " << m_addr6 << std::endl;
}
bool
operator== (const MeshnetHeader & a, const MeshnetHeader & b)
{
  return ((a.m_meshnetFlags == b.m_meshnetFlags) && (a.m_meshnetTtl == b.m_meshnetTtl)
          && (a.m_meshnetSeqno == b.m_meshnetSeqno) && (a.m_addr4 == b.m_addr4) && (a.m_addr5 == b.m_addr5)
          && (a.m_addr6 == b.m_addr6));
}
} // namespace hu11s
} // namespace ns3
