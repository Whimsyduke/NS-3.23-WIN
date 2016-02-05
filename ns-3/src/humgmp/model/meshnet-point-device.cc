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
 *         Pavel Boyko <boyko@iitp.ru>
 */

#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/meshnet-point-device.h"
#include "ns3/wifi-net-device.h"
#include "ns3/meshnet-wifi-interface-mac.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MeshnetPointDevice");

NS_OBJECT_ENSURE_REGISTERED (MeshnetPointDevice);

TypeId
MeshnetPointDevice::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::MeshnetPointDevice")
    .SetParent<NetDevice> ()
    .SetGroupName ("Dsr")
    .AddConstructor<MeshnetPointDevice> ()
    .AddAttribute ("Mtu", "The MAC-level Maximum Transmission Unit",
                   UintegerValue (0xffff),
                   MakeUintegerAccessor (&MeshnetPointDevice::SetMtu,
                                         &MeshnetPointDevice::GetMtu),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ( "RoutingProtocol",
                    "The meshnet routing protocol used by this meshnet point.",
                    PointerValue (),
                    MakePointerAccessor (
                      &MeshnetPointDevice::GetRoutingProtocol, &MeshnetPointDevice::SetRoutingProtocol),
                    MakePointerChecker<
                      MeshnetL2RoutingProtocol> ());
  return tid;
}

MeshnetPointDevice::MeshnetPointDevice () :
  m_ifIndex (0)
{
  NS_LOG_FUNCTION (this);
  m_channel = CreateObject<BridgeChannel> ();
}

MeshnetPointDevice::~MeshnetPointDevice ()
{
  NS_LOG_FUNCTION (this);
  m_node = 0;
  m_channel = 0;
  m_routingProtocol = 0;
}

void
MeshnetPointDevice::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  for (std::vector<Ptr<NetDevice> >::iterator iter = m_ifaces.begin (); iter != m_ifaces.end (); iter++)
    {
      *iter = 0;
    }
  m_ifaces.clear ();
  m_node = 0;
  m_channel = 0;
  m_routingProtocol = 0;
  NetDevice::DoDispose ();

}

//-----------------------------------------------------------------------------
// NetDevice interface implementation
//-----------------------------------------------------------------------------

void
MeshnetPointDevice::ReceiveFromDevice (Ptr<NetDevice> incomingPort, Ptr<const Packet> packet, uint16_t protocol,
                                    Address const &src, Address const &dst, PacketType packetType)
{
  NS_LOG_FUNCTION (this << incomingPort << packet);
  NS_LOG_DEBUG ("UID is " << packet->GetUid ());
  const Mac48Address src48 = Mac48Address::ConvertFrom (src);
  const Mac48Address dst48 = Mac48Address::ConvertFrom (dst);
  uint16_t& realProtocol = protocol;
  NS_LOG_DEBUG ("SRC=" << src48 << ", DST = " << dst48 << ", I am: " << m_address);
  if (!m_promiscRxCallback.IsNull ())
    {
      m_promiscRxCallback (this, packet, protocol, src, dst, packetType);
    }
  if (dst48.IsGroup ())
    {
      Ptr<Packet> packet_copy = packet->Copy ();
      if (m_routingProtocol->RemoveRoutingStuff (incomingPort->GetIfIndex (), src48, dst48, packet_copy, realProtocol))
        {
          m_rxCallback (this, packet_copy, realProtocol, src);
          Forward (incomingPort, packet, protocol, src48, dst48);

          m_rxStats.broadcastData++;
          m_rxStats.broadcastDataBytes += packet->GetSize ();
        }
      return;
    }
  if (dst48 == m_address)
    {
      Ptr<Packet> packet_copy = packet->Copy ();
      if (m_routingProtocol->RemoveRoutingStuff (incomingPort->GetIfIndex (), src48, dst48, packet_copy, realProtocol))
        {
          m_rxCallback (this, packet_copy, realProtocol, src);
          m_rxStats.unicastData++;
          m_rxStats.unicastDataBytes += packet->GetSize ();
        }
      return;
    }
  else
    Forward (incomingPort, packet->Copy (), protocol, src48, dst48);
}

void
MeshnetPointDevice::Forward (Ptr<NetDevice> inport, Ptr<const Packet> packet, uint16_t protocol,
                          const Mac48Address src, const Mac48Address dst)
{
  // pass through routing protocol
  m_routingProtocol->RequestRoute (inport->GetIfIndex (), src, dst, packet, protocol, MakeCallback (
                                     &MeshnetPointDevice::DoSend, this));
}

void
MeshnetPointDevice::SetIfIndex (const uint32_t index)
{
  NS_LOG_FUNCTION (this);
  m_ifIndex = index;
}

uint32_t
MeshnetPointDevice::GetIfIndex () const
{
  NS_LOG_FUNCTION (this);
  return m_ifIndex;
}

Ptr<Channel>
MeshnetPointDevice::GetChannel () const
{
  NS_LOG_FUNCTION (this);
  return m_channel;
}

Address
MeshnetPointDevice::GetAddress () const
{
  NS_LOG_FUNCTION (this);
  return m_address;
}

void
MeshnetPointDevice::SetAddress (Address a)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_WARN ("Manual changing meshnet point address can cause routing errors.");
  m_address = Mac48Address::ConvertFrom (a);
}

bool
MeshnetPointDevice::SetMtu (const uint16_t mtu)
{
  NS_LOG_FUNCTION (this);
  m_mtu = mtu;
  return true;
}

uint16_t
MeshnetPointDevice::GetMtu () const
{
  NS_LOG_FUNCTION (this);
  return m_mtu;
}

bool
MeshnetPointDevice::IsLinkUp () const
{
  NS_LOG_FUNCTION (this);
  return true;
}

void
MeshnetPointDevice::AddLinkChangeCallback (Callback<void> callback)
{
  NS_LOG_FUNCTION (this);
  // do nothing
  NS_LOG_WARN ("AddLinkChangeCallback does nothing");
}

bool
MeshnetPointDevice::IsBroadcast () const
{
  NS_LOG_FUNCTION (this);
  return true;
}

Address
MeshnetPointDevice::GetBroadcast () const
{
  NS_LOG_FUNCTION (this);
  return Mac48Address ("ff:ff:ff:ff:ff:ff");
}

bool
MeshnetPointDevice::IsMulticast () const
{
  NS_LOG_FUNCTION (this);
  return true;
}

Address
MeshnetPointDevice::GetMulticast (Ipv4Address multicastGroup) const
{
  NS_LOG_FUNCTION (this << multicastGroup);
  Mac48Address multicast = Mac48Address::GetMulticast (multicastGroup);
  return multicast;
}

bool
MeshnetPointDevice::IsPointToPoint () const
{
  NS_LOG_FUNCTION (this);
  return false;
}

bool
MeshnetPointDevice::IsBridge () const
{
  NS_LOG_FUNCTION (this);
  return false;
}

bool
MeshnetPointDevice::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (this);
  const Mac48Address dst48 = Mac48Address::ConvertFrom (dest);
  return m_routingProtocol->RequestRoute (m_ifIndex, m_address, dst48, packet, protocolNumber, MakeCallback (
                                            &MeshnetPointDevice::DoSend, this));
}

bool
MeshnetPointDevice::SendFrom (Ptr<Packet> packet, const Address& src, const Address& dest,
                           uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (this);
  const Mac48Address src48 = Mac48Address::ConvertFrom (src);
  const Mac48Address dst48 = Mac48Address::ConvertFrom (dest);
  return m_routingProtocol->RequestRoute (m_ifIndex, src48, dst48, packet, protocolNumber, MakeCallback (
                                            &MeshnetPointDevice::DoSend, this));
}

Ptr<Node>
MeshnetPointDevice::GetNode () const
{
  NS_LOG_FUNCTION (this);
  return m_node;
}

void
MeshnetPointDevice::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this);
  m_node = node;
}

bool
MeshnetPointDevice::NeedsArp () const
{
  NS_LOG_FUNCTION (this);
  return true;
}

void
MeshnetPointDevice::SetReceiveCallback (NetDevice::ReceiveCallback cb)
{
  NS_LOG_FUNCTION (this);
  m_rxCallback = cb;
}

void
MeshnetPointDevice::SetPromiscReceiveCallback (NetDevice::PromiscReceiveCallback cb)
{
  NS_LOG_FUNCTION (this);
  m_promiscRxCallback = cb;
}

bool
MeshnetPointDevice::SupportsSendFrom () const
{
  NS_LOG_FUNCTION (this);
  return false; // don't allow to bridge meshnet network with something else.
}

Address
MeshnetPointDevice::GetMulticast (Ipv6Address addr) const
{
  NS_LOG_FUNCTION (this << addr);
  return Mac48Address::GetMulticast (addr);
}

//-----------------------------------------------------------------------------
// Interfaces
//-----------------------------------------------------------------------------
uint32_t
MeshnetPointDevice::GetNInterfaces () const
{
  NS_LOG_FUNCTION (this);
  return m_ifaces.size ();
}

Ptr<NetDevice>
MeshnetPointDevice::GetInterface (uint32_t n) const
{
  NS_LOG_FUNCTION (this << n);
  for (std::vector<Ptr<NetDevice> >::const_iterator i = m_ifaces.begin (); i != m_ifaces.end (); i++)
    {
      if ((*i)->GetIfIndex () == n)
        {
          return (*i);
        }
    }
  NS_FATAL_ERROR ("Meshnet point interface is not found by index");
  return 0;
}
std::vector<Ptr<NetDevice> >
MeshnetPointDevice::GetInterfaces () const
{
  return m_ifaces;
}
void
MeshnetPointDevice::AddInterface (Ptr<NetDevice> iface)
{
  NS_LOG_FUNCTION (this << iface);

  NS_ASSERT (iface != this);
  if (!Mac48Address::IsMatchingType (iface->GetAddress ()))
    {
      NS_FATAL_ERROR ("Device does not support eui 48 addresses: cannot be used as a meshnet point interface.");
    }
  if (!iface->SupportsSendFrom ())
    {
      NS_FATAL_ERROR ("Device does not support SendFrom: cannot be used as a meshnet point interface.");
    }

  // Meshnet point has MAC address of it's first interface
  if (m_ifaces.empty ())
    {
      m_address = Mac48Address::ConvertFrom (iface->GetAddress ());
    }
  Ptr<WifiNetDevice> wifiNetDev = iface->GetObject<WifiNetDevice> ();
  if (wifiNetDev == 0)
    {
      NS_FATAL_ERROR ("Device is not a WiFi NIC: cannot be used as a meshnet point interface.");
    }
  Ptr<MeshnetWifiInterfaceMac> ifaceMac = wifiNetDev->GetMac ()->GetObject<MeshnetWifiInterfaceMac> ();
  if (ifaceMac == 0)
    {
      NS_FATAL_ERROR (
        "WiFi device doesn't have correct MAC installed: cannot be used as a meshnet point interface.");
    }
  ifaceMac->SetMeshnetPointAddress (m_address);

  // Receive frames from this interface
  m_node->RegisterProtocolHandler (MakeCallback (&MeshnetPointDevice::ReceiveFromDevice, this), 0, iface, /*promiscuous = */
                                   true);
  m_ifaces.push_back (iface);
  m_channel->AddChannel (iface->GetChannel ());
}

//-----------------------------------------------------------------------------
// Protocols
//-----------------------------------------------------------------------------

void
MeshnetPointDevice::SetRoutingProtocol (Ptr<MeshnetL2RoutingProtocol> protocol)
{
  NS_LOG_FUNCTION (this << protocol);
  NS_ASSERT_MSG (PeekPointer (protocol->GetMeshnetPoint ()) == this,
                 "Routing protocol must be installed on meshnet point to be useful.");
  m_routingProtocol = protocol;
}

Ptr<MeshnetL2RoutingProtocol>
MeshnetPointDevice::GetRoutingProtocol () const
{
  NS_LOG_FUNCTION (this);
  return m_routingProtocol;
}

void
MeshnetPointDevice::DoSend (bool success, Ptr<Packet> packet, Mac48Address src, Mac48Address dst,
                         uint16_t protocol, uint32_t outIface)
{
  NS_LOG_FUNCTION (this << success << packet << src << dst << protocol << outIface);
  if (!success)
    {
      NS_LOG_DEBUG ("Resolve failed");
      return;
    }

  // Count statistics
  Statistics * stats = ((src == m_address) ? &m_txStats : &m_fwdStats);

  if (dst.IsBroadcast ())
    {
      stats->broadcastData++;
      stats->broadcastDataBytes += packet->GetSize ();
    }
  else
    {
      stats->unicastData++;
      stats->unicastDataBytes += packet->GetSize ();
    }

  // Send
  if (outIface != 0xffffffff)
    {
      GetInterface (outIface)->SendFrom (packet, src, dst, protocol);
    }
  else
    {
      for (std::vector<Ptr<NetDevice> >::iterator i = m_ifaces.begin (); i != m_ifaces.end (); i++)
        {
          (*i)->SendFrom (packet->Copy (), src, dst, protocol);
        }
    }
}
MeshnetPointDevice::Statistics::Statistics () :
  unicastData (0), unicastDataBytes (0), broadcastData (0), broadcastDataBytes (0)
{
  NS_LOG_FUNCTION (this);
}

void
MeshnetPointDevice::Report (std::ostream & os) const
{
  NS_LOG_FUNCTION (this);
  os << "<Statistics" << std::endl <<
  "txUnicastData=\"" << m_txStats.unicastData << "\"" << std::endl <<
  "txUnicastDataBytes=\"" << m_txStats.unicastDataBytes << "\"" << std::endl <<
  "txBroadcastData=\"" << m_txStats.broadcastData << "\"" << std::endl <<
  "txBroadcastDataBytes=\"" << m_txStats.broadcastDataBytes << "\"" << std::endl <<
  "rxUnicastData=\"" << m_rxStats.unicastData << "\"" << std::endl <<
  "rxUnicastDataBytes=\"" << m_rxStats.unicastDataBytes << "\"" << std::endl <<
  "rxBroadcastData=\"" << m_rxStats.broadcastData << "\"" << std::endl <<
  "rxBroadcastDataBytes=\"" << m_rxStats.broadcastDataBytes << "\"" << std::endl <<
  "fwdUnicastData=\"" << m_fwdStats.unicastData << "\"" << std::endl <<
  "fwdUnicastDataBytes=\"" << m_fwdStats.unicastDataBytes << "\"" << std::endl <<
  "fwdBroadcastData=\"" << m_fwdStats.broadcastData << "\"" << std::endl <<
  "fwdBroadcastDataBytes=\"" << m_fwdStats.broadcastDataBytes << "\"" << std::endl <<
  "/>" << std::endl;
}

void
MeshnetPointDevice::ResetStats ()
{
  NS_LOG_FUNCTION (this);
  m_rxStats = Statistics ();
  m_txStats = Statistics ();
  m_fwdStats = Statistics ();
}

} // namespace ns3
