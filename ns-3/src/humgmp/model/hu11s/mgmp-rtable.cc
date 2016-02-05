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

#include "ns3/object.h"
#include "ns3/assert.h"
#include "ns3/simulator.h"
#include "ns3/test.h"
#include "ns3/log.h"

#include "mgmp-rtable.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MgmpRtable");

namespace hu11s {
  
NS_OBJECT_ENSURE_REGISTERED (MgmpRtable);

TypeId
MgmpRtable::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::hu11s::MgmpRtable")
    .SetParent<Object> ()
    .SetGroupName ("Meshnet")
    .AddConstructor<MgmpRtable> ();
  return tid;
}
MgmpRtable::MgmpRtable ()
{
  DeleteProactivePath ();
}
MgmpRtable::~MgmpRtable ()
{
}
void
MgmpRtable::DoDispose ()
{
  m_routes.clear ();
}
void
MgmpRtable::AddReactivePath (Mac48Address destination, Mac48Address retransmitter, uint32_t interface,
                             uint32_t metric, Time lifetime, uint32_t seqnum)
{
  std::map<Mac48Address, ReactiveRoute>::iterator i = m_routes.find (destination);
  if (i == m_routes.end ())
    {
      ReactiveRoute newroute;
      m_routes[destination] = newroute;
    }
  i = m_routes.find (destination);
  NS_ASSERT (i != m_routes.end ());
  i->second.retransmitter = retransmitter;
  i->second.interface = interface;
  i->second.metric = metric;
  i->second.whenExpire = Simulator::Now () + lifetime;
  i->second.seqnum = seqnum;
}
void
MgmpRtable::AddProactivePath (uint32_t metric, Mac48Address root, Mac48Address retransmitter,
                              uint32_t interface, Time lifetime, uint32_t seqnum)
{
  m_root.root = root;
  m_root.retransmitter = retransmitter;
  m_root.metric = metric;
  m_root.whenExpire = Simulator::Now () + lifetime;
  m_root.seqnum = seqnum;
  m_root.interface = interface;
}
void
MgmpRtable::AddPrecursor (Mac48Address destination, uint32_t precursorInterface,
                          Mac48Address precursorAddress, Time lifetime)
{
  Precursor precursor;
  precursor.interface = precursorInterface;
  precursor.address = precursorAddress;
  precursor.whenExpire = Simulator::Now () + lifetime;
  std::map<Mac48Address, ReactiveRoute>::iterator i = m_routes.find (destination);
  if (i != m_routes.end ())
    {
      bool should_add = true;
      for (unsigned int j = 0; j < i->second.precursors.size (); j++)
        {
          //NB: Only one active route may exist, so do not check
          //interface ID, just address
          if (i->second.precursors[j].address == precursorAddress)
            {
              should_add = false;
              i->second.precursors[j].whenExpire = precursor.whenExpire;
              break;
            }
        }
      if (should_add)
        {
          i->second.precursors.push_back (precursor);
        }
    }
}
void
MgmpRtable::DeleteProactivePath ()
{
  m_root.precursors.clear ();
  m_root.interface = INTERFACE_ANY;
  m_root.metric = MAX_METRIC;
  m_root.retransmitter = Mac48Address::GetBroadcast ();
  m_root.seqnum = 0;
  m_root.whenExpire = Simulator::Now ();
}
void
MgmpRtable::DeleteProactivePath (Mac48Address root)
{
  if (m_root.root == root)
    {
      DeleteProactivePath ();
    }
}
void
MgmpRtable::DeleteReactivePath (Mac48Address destination)
{
  std::map<Mac48Address, ReactiveRoute>::iterator i = m_routes.find (destination);
  if (i != m_routes.end ())
    {
      m_routes.erase (i);
    }
}
MgmpRtable::LookupResult
MgmpRtable::LookupReactive (Mac48Address destination)
{
  std::map<Mac48Address, ReactiveRoute>::iterator i = m_routes.find (destination);
  if (i == m_routes.end ())
    {
      return LookupResult ();
    }
  if ((i->second.whenExpire < Simulator::Now ()) && (i->second.whenExpire != Seconds (0)))
    {
      NS_LOG_DEBUG ("Reactive route has expired, sorry.");
      return LookupResult ();
    }
  return LookupReactiveExpired (destination);
}
MgmpRtable::LookupResult
MgmpRtable::LookupReactiveExpired (Mac48Address destination)
{
  std::map<Mac48Address, ReactiveRoute>::iterator i = m_routes.find (destination);
  if (i == m_routes.end ())
    {
      return LookupResult ();
    }
  return LookupResult (i->second.retransmitter, i->second.interface, i->second.metric, i->second.seqnum,
                       i->second.whenExpire - Simulator::Now ());
}
MgmpRtable::LookupResult
MgmpRtable::LookupProactive ()
{
  if (m_root.whenExpire < Simulator::Now ())
    {
      NS_LOG_DEBUG ("Proactive route has expired and will be deleted, sorry.");
      DeleteProactivePath ();
    }
  return LookupProactiveExpired ();
}
MgmpRtable::LookupResult
MgmpRtable::LookupProactiveExpired ()
{
  return LookupResult (m_root.retransmitter, m_root.interface, m_root.metric, m_root.seqnum,
                       m_root.whenExpire - Simulator::Now ());
}
std::vector<MgmpProtocol::FailedDestination>
MgmpRtable::GetUnreachableDestinations (Mac48Address huperAddress)
{
  MgmpProtocol::FailedDestination dst;
  std::vector<MgmpProtocol::FailedDestination> retval;
  for (std::map<Mac48Address, ReactiveRoute>::iterator i = m_routes.begin (); i != m_routes.end (); i++)
    {
      if (i->second.retransmitter == huperAddress)
        {
          dst.destination = i->first;
          i->second.seqnum++;
          dst.seqnum = i->second.seqnum;
          retval.push_back (dst);
        }
    }
  //Lookup a path to root
  if (m_root.retransmitter == huperAddress)
    {
      dst.destination = m_root.root;
      dst.seqnum = m_root.seqnum;
      retval.push_back (dst);
    }
  return retval;
}
MgmpRtable::PrecursorList
MgmpRtable::GetPrecursors (Mac48Address destination)
{
  //We suppose that no duplicates here can be
  PrecursorList retval;
  std::map<Mac48Address, ReactiveRoute>::iterator route = m_routes.find (destination);
  if (route != m_routes.end ())
    {
      for (std::vector<Precursor>::const_iterator i = route->second.precursors.begin ();
           i != route->second.precursors.end (); i++)
        {
          if (i->whenExpire > Simulator::Now ())
            {
              retval.push_back (std::make_pair (i->interface, i->address));
            }
        }
    }
  return retval;
}
bool
MgmpRtable::LookupResult::operator== (const MgmpRtable::LookupResult & o) const
{
  return (retransmitter == o.retransmitter && ifIndex == o.ifIndex && metric == o.metric && seqnum
          == o.seqnum);
}
MgmpRtable::LookupResult::LookupResult (Mac48Address r, uint32_t i, uint32_t m, uint32_t s, Time l) :
  retransmitter (r), ifIndex (i), metric (m), seqnum (s), lifetime (l)
{
}
bool
MgmpRtable::LookupResult::IsValid () const
{
  return !(retransmitter == Mac48Address::GetBroadcast () && ifIndex == INTERFACE_ANY && metric == MAX_METRIC
           && seqnum == 0);
}
} // namespace hu11s
} // namespace ns3
