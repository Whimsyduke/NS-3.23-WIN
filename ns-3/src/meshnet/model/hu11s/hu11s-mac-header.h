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


#ifndef MESHNET_WIFI_MAC_HEADER_H
#define MESHNET_WIFI_MAC_HEADER_H

#include "ns3/header.h"
#include "ns3/mac48-address.h"

namespace ns3 {
namespace hu11s {
/**
 * \ingroup hu11s
 *
 * \brief Meshnet Control field, see IEEE 802.11s draft 3.0 section 7.1.3.5b
 *
 * Header format: | Meshnet flags: 1 | TTL: 1 | Sequence number: 4 | Address ext.: 0, 6, 12 or 18 |
 */
class MeshnetHeader : public Header
{
public:
  MeshnetHeader ();
  ~MeshnetHeader ();
  static TypeId GetTypeId ();
  virtual TypeId GetInstanceTypeId () const;
  virtual void Print (std::ostream &os) const;

  void SetAddr4 (Mac48Address address);
  void SetAddr5 (Mac48Address address);
  void SetAddr6 (Mac48Address address);
  Mac48Address GetAddr4 () const;
  Mac48Address GetAddr5 () const;
  Mac48Address GetAddr6 () const;

  void SetMeshnetSeqno (uint32_t seqno);
  uint32_t GetMeshnetSeqno () const;

  void SetMeshnetTtl (uint8_t TTL);
  uint8_t GetMeshnetTtl () const;

  void SetAddressExt (uint8_t num_of_addresses);
  uint8_t GetAddressExt () const;

  virtual uint32_t GetSerializedSize () const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
private:
  uint8_t m_meshnetFlags;
  uint8_t m_meshnetTtl;
  uint32_t m_meshnetSeqno;
  Mac48Address m_addr4;
  Mac48Address m_addr5;
  Mac48Address m_addr6;
  friend bool operator== (const MeshnetHeader & a, const MeshnetHeader & b);
};
bool operator== (const MeshnetHeader & a, const MeshnetHeader & b);

} // namespace hu11s
} // namespace ns3
#endif /* MESHNET_WIFI_MAC_HEADER_H */
