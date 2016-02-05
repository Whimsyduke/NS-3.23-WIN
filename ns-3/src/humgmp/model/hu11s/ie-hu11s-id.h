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

#ifndef MESHNET_HU11S_ID_H
#define MESHNET_HU11S_ID_H

#include <stdint.h>
#include "ns3/buffer.h"
#include "ns3/meshnet-information-element-vector.h"

namespace ns3 {
namespace hu11s {
/**
 * \brief a IEEE 802.11s Meshnet ID 7.3.287 of 802.11s draft 3.0
 * \see attribute_IeMeshnetId
 */
class IeMeshnetId : public WifiInformationElement
{
public:
  // broadcast meshnetId
  IeMeshnetId ();
  IeMeshnetId (std::string s);

  bool IsEqual (IeMeshnetId const &o) const;
  bool IsBroadcast (void) const;
  //uint32_t GetLength (void) const;
  char *PeekString (void) const;

  // Inherited from WifiInformationElement
  virtual WifiInformationElementId ElementId () const;
  virtual void SerializeInformationField (Buffer::Iterator i) const;
  virtual uint8_t DeserializeInformationField (Buffer::Iterator start, uint8_t length);
  virtual void Print (std::ostream& os) const;
  virtual uint8_t GetInformationFieldSize () const;

private:
  uint8_t m_meshnetId[33];
  friend bool operator== (const IeMeshnetId & a, const IeMeshnetId & b);

};

std::ostream &operator << (std::ostream &os, const IeMeshnetId &meshnetId);

ATTRIBUTE_HELPER_HEADER (IeMeshnetId);
  
} // namespace hu11s
} // namespace ns3
#endif /* MESHNET_ID_H */
