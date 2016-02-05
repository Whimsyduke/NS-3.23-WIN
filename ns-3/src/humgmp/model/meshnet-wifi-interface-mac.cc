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
 *          Pavel Boyko <boyko@iitp.ru>
 */

#include "ns3/meshnet-wifi-interface-mac.h"
#include "ns3/meshnet-wifi-beacon.h"
#include "ns3/log.h"
#include "ns3/boolean.h"
#include "ns3/wifi-phy.h"
#include "ns3/dcf-manager.h"
#include "ns3/mac-rx-middle.h"
#include "ns3/mac-low.h"
#include "ns3/dca-txop.h"
#include "ns3/random-variable-stream.h"
#include "ns3/simulator.h"
#include "ns3/yans-wifi-phy.h"
#include "ns3/pointer.h"
#include "ns3/double.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/qos-tag.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MeshnetWifiInterfaceMac");

NS_OBJECT_ENSURE_REGISTERED (MeshnetWifiInterfaceMac);

TypeId
MeshnetWifiInterfaceMac::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::MeshnetWifiInterfaceMac")
    .SetParent<RegularWifiMac> ()
    .SetGroupName ("Meshnet")
    .AddConstructor<MeshnetWifiInterfaceMac> ()
    .AddAttribute ( "BeaconInterval",
                    "Beacon Interval",
                    TimeValue (Seconds (0.5)),

                    MakeTimeAccessor (
                      &MeshnetWifiInterfaceMac::m_beaconInterval),
                    MakeTimeChecker ()
                    )
    .AddAttribute ( "RandomStart",
                    "Window when beacon generating starts (uniform random) in seconds",
                    TimeValue (Seconds (0.5)),
                    MakeTimeAccessor (
                      &MeshnetWifiInterfaceMac::m_randomStart),
                    MakeTimeChecker ()
                    )
    .AddAttribute ( "BeaconGeneration",
                    "Enable/Disable Beaconing.",
                    BooleanValue (true),
                    MakeBooleanAccessor (
                      &MeshnetWifiInterfaceMac::SetBeaconGeneration, &MeshnetWifiInterfaceMac::GetBeaconGeneration),
                    MakeBooleanChecker ()
                    )
  ;
  return tid;
}
MeshnetWifiInterfaceMac::MeshnetWifiInterfaceMac () :
  m_standard (WIFI_PHY_STANDARD_80211a)
{
  NS_LOG_FUNCTION (this);

  // Let the lower layers know that we are acting as a mesh node
  SetTypeOfStation (MESH);
  m_coefficient = CreateObject<UniformRandomVariable> ();
}
MeshnetWifiInterfaceMac::~MeshnetWifiInterfaceMac ()
{
  NS_LOG_FUNCTION (this);
}
//-----------------------------------------------------------------------------
// WifiMac inherited
//-----------------------------------------------------------------------------
void
MeshnetWifiInterfaceMac::Enqueue (Ptr<const Packet> packet, Mac48Address to, Mac48Address from)
{
  NS_LOG_FUNCTION (this << packet << to << from);
  ForwardDown (packet, from, to);
}
void
MeshnetWifiInterfaceMac::Enqueue (Ptr<const Packet> packet, Mac48Address to)
{
  NS_LOG_FUNCTION (this << packet << to);
  ForwardDown (packet, m_low->GetAddress (), to);
}
bool
MeshnetWifiInterfaceMac::SupportsSendFrom () const
{
  return true;
}
void
MeshnetWifiInterfaceMac::SetLinkUpCallback (Callback<void> linkUp)
{
  NS_LOG_FUNCTION (this);
  RegularWifiMac::SetLinkUpCallback (linkUp);

  // The approach taken here is that, from the point of view of a meshnet
  // node, the link is always up, so we immediately invoke the
  // callback if one is set
  linkUp ();
}
void
MeshnetWifiInterfaceMac::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_plugins.clear ();
  m_beaconSendEvent.Cancel ();

  RegularWifiMac::DoDispose ();
}
void
MeshnetWifiInterfaceMac::DoInitialize ()
{
  NS_LOG_FUNCTION (this);
  m_coefficient->SetAttribute ("Max", DoubleValue (m_randomStart.GetSeconds ()));
  if (m_beaconEnable)
    {
      Time randomStart = Seconds (m_coefficient->GetValue ());
      // Now start sending beacons after some random delay (to avoid collisions)
      NS_ASSERT (!m_beaconSendEvent.IsRunning ());
      m_beaconSendEvent = Simulator::Schedule (randomStart, &MeshnetWifiInterfaceMac::SendBeacon, this);
      m_tbtt = Simulator::Now () + randomStart;
    }
  else
    {
      // stop sending beacons
      m_beaconSendEvent.Cancel ();
    }
}

int64_t
MeshnetWifiInterfaceMac::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  int64_t currentStream = stream;
  m_coefficient->SetStream (currentStream++);
  for (PluginList::const_iterator i = m_plugins.begin (); i < m_plugins.end (); i++)
    {
      currentStream += (*i)->AssignStreams (currentStream);
    }
  return (currentStream - stream);
}

//-----------------------------------------------------------------------------
// Plugins
//-----------------------------------------------------------------------------
void
MeshnetWifiInterfaceMac::InstallPlugin (Ptr<MeshnetWifiInterfaceMacPlugin> plugin)
{
  NS_LOG_FUNCTION (this);

  plugin->SetParent (this);
  m_plugins.push_back (plugin);
}
//-----------------------------------------------------------------------------
// Switch channels
//-----------------------------------------------------------------------------
uint16_t
MeshnetWifiInterfaceMac::GetFrequencyChannel () const
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_phy != 0); // need PHY to set/get channel

  Ptr<YansWifiPhy> phy = m_phy->GetObject<YansWifiPhy> ();
  if (phy != 0)
    {
      return phy->GetChannelNumber ();
    }
  else
    {
      return 0;
    }
}
void
MeshnetWifiInterfaceMac::SwitchFrequencyChannel (uint16_t new_id)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_phy != 0); // need PHY to set/get channel
  /**
   * \todo
   * Correct channel switching is:
   *
   * 1. Interface down, e.g. to stop packets from layer 3
   * 2. Wait before all output queues will be empty
   * 3. Switch PHY channel
   * 4. Interface up
   *
   * Now we use dirty channel switch -- just change frequency
   */
  Ptr<YansWifiPhy> phy = m_phy->GetObject<YansWifiPhy> ();
  phy->SetChannelNumber (new_id);
  // Don't know NAV on new channel
  m_dcfManager->NotifyNavResetNow (Seconds (0));
}
//-----------------------------------------------------------------------------
// Forward frame down
//-----------------------------------------------------------------------------
void
MeshnetWifiInterfaceMac::ForwardDown (Ptr<const Packet> const_packet, Mac48Address from, Mac48Address to)
{
  // copy packet to allow modifications
  Ptr<Packet> packet = const_packet->Copy ();
  WifiMacHeader hdr;
  hdr.SetTypeData ();
  hdr.SetAddr2 (GetAddress ());
  hdr.SetAddr3 (to);
  hdr.SetAddr4 (from);
  hdr.SetDsFrom ();
  hdr.SetDsTo ();
  // Fill QoS fields:
  hdr.SetQosAckPolicy (WifiMacHeader::NORMAL_ACK);
  hdr.SetQosNoEosp ();
  hdr.SetQosNoAmsdu ();
  hdr.SetQosTxopLimit (0);
  // Address 1 is unknwon here. Routing plugin is responsible to correctly set it.
  hdr.SetAddr1 (Mac48Address ());
  // Filter packet through all installed plugins
  ///WINDOWS
  //for (PluginList::const_iterator i = m_plugins.end() - 1; i != m_plugins.begin() - 1; i--)
  //�ο�ns-3-win2.2014.11.30.
#ifndef WIN32
  for (PluginList::const_iterator i = m_plugins.end() - 1; i != m_plugins.begin() - 1; i--)
#else
  for (PluginList::const_reverse_iterator i = m_plugins.rbegin(); i != m_plugins.rend(); ++i)
#endif
	  ///WINDOWS
    {
      bool drop = !((*i)->UpdateOutcomingFrame (packet, hdr, from, to));
      if (drop)
        {
          return; // plugin drops frame
        }
    }
  // Assert that address1 is set. Assert will fail e.g. if there is no installed routing plugin.
  NS_ASSERT (hdr.GetAddr1 () != Mac48Address ());
  // Queue frame
  if (m_stationManager->IsBrandNew (hdr.GetAddr1 ()))
    {
      // in adhoc mode, we assume that every destination
      // supports all the rates we support.
      for (uint32_t i = 0; i < m_phy->GetNModes (); i++)
        {
          m_stationManager->AddSupportedMode (hdr.GetAddr1 (), m_phy->GetMode (i));
        }
      m_stationManager->RecordDisassociated (hdr.GetAddr1 ());
    }
  //Classify: application sets a tag, which is removed here
  // Get Qos tag:
  AcIndex ac = AC_BE;
  QosTag tag;
  if (packet->RemovePacketTag (tag))
    {
      hdr.SetType (WIFI_MAC_QOSDATA);
      hdr.SetQosTid (tag.GetTid ());
      //Aftre setting type DsFrom and DsTo fields are reset.
      hdr.SetDsFrom ();
      hdr.SetDsTo ();
      ac = QosUtilsMapTidToAc (tag.GetTid ());
    }
  m_stats.sentFrames++;
  m_stats.sentBytes += packet->GetSize ();
  NS_ASSERT (m_edca.find (ac) != m_edca.end ());
  m_edca[ac]->Queue (packet, hdr);
}
void
MeshnetWifiInterfaceMac::SendManagementFrame (Ptr<Packet> packet, const WifiMacHeader& hdr)
{
  //Filter management frames:
	WifiMacHeader header = hdr;
	///WINDOWS
	//for (PluginList::const_iterator i = m_plugins.end() - 1; i != m_plugins.begin() - 1; i--)
	//�ο�ns-3-win2.2014.11.30.
#ifndef WIN32
	for (PluginList::const_iterator i = m_plugins.end() - 1; i != m_plugins.begin() - 1; i--)
#else
	for (PluginList::const_reverse_iterator i = m_plugins.rbegin(); i != m_plugins.rend(); ++i)
#endif
		///WINDOWS
    {
      bool drop = !((*i)->UpdateOutcomingFrame (packet, header, Mac48Address (), Mac48Address ()));
      if (drop)
        {
          return; // plugin drops frame
        }
    }
  m_stats.sentFrames++;
  m_stats.sentBytes += packet->GetSize ();
  if ((m_edca.find (AC_VO) == m_edca.end ()) || (m_edca.find (AC_BK) == m_edca.end ()))
    {
      NS_FATAL_ERROR ("Voice or Background queue is not set up!");
    }
  /*
   * When we send a management frame - it is better to enqueue it to
   * priority queue. But when we send a broadcast management frame,
   * like PREQ, little MinCw value may cause collisions during
   * retransmissions (two neighbor stations may choose the same window
   * size, and two packets will be collided). So, broadcast management
   * frames go to BK queue.
   */
  if (hdr.GetAddr1 () != Mac48Address::GetBroadcast ())
    {
      m_edca[AC_VO]->Queue (packet, header);
    }
  else
    {
      m_edca[AC_BK]->Queue (packet, header);
    }
}
SupportedRates
MeshnetWifiInterfaceMac::GetSupportedRates () const
{
  // set the set of supported rates and make sure that we indicate
  // the Basic Rate set in this set of supported rates.
  SupportedRates rates;
  for (uint32_t i = 0; i < m_phy->GetNModes (); i++)
    {
      WifiMode mode = m_phy->GetMode (i);
      rates.AddSupportedRate (mode.GetDataRate ());
    }
  // set the basic rates
  for (uint32_t j = 0; j < m_stationManager->GetNBasicModes (); j++)
    {
      WifiMode mode = m_stationManager->GetBasicMode (j);
      rates.SetBasicRate (mode.GetDataRate ());
    }
  return rates;
}
bool
MeshnetWifiInterfaceMac::CheckSupportedRates (SupportedRates rates) const
{
  for (uint32_t i = 0; i < m_stationManager->GetNBasicModes (); i++)
    {
      WifiMode mode = m_stationManager->GetBasicMode (i);
      if (!rates.IsSupportedRate (mode.GetDataRate ()))
        {
          return false;
        }
    }
  return true;
}
//-----------------------------------------------------------------------------
// Beacons
//-----------------------------------------------------------------------------
void
MeshnetWifiInterfaceMac::SetRandomStartDelay (Time interval)
{
  NS_LOG_FUNCTION (this << interval);
  m_randomStart = interval;
}
void
MeshnetWifiInterfaceMac::SetBeaconInterval (Time interval)
{
  NS_LOG_FUNCTION (this << interval);
  m_beaconInterval = interval;
}
Time
MeshnetWifiInterfaceMac::GetBeaconInterval () const
{
  return m_beaconInterval;
}
void
MeshnetWifiInterfaceMac::SetBeaconGeneration (bool enable)
{
  NS_LOG_FUNCTION (this << enable);
  m_beaconEnable = enable;
}
bool
MeshnetWifiInterfaceMac::GetBeaconGeneration () const
{
  return m_beaconSendEvent.IsRunning ();
}
Time
MeshnetWifiInterfaceMac::GetTbtt () const
{
  return m_tbtt;
}
void
MeshnetWifiInterfaceMac::ShiftTbtt (Time shift)
{
  // User of ShiftTbtt () must take care don't shift it to the past
  NS_ASSERT (GetTbtt () + shift > Simulator::Now ());

  m_tbtt += shift;
  // Shift scheduled event
  Simulator::Cancel (m_beaconSendEvent);
  m_beaconSendEvent = Simulator::Schedule (GetTbtt () - Simulator::Now (), &MeshnetWifiInterfaceMac::SendBeacon,
                                           this);
}
void
MeshnetWifiInterfaceMac::ScheduleNextBeacon ()
{
  m_tbtt += GetBeaconInterval ();
  m_beaconSendEvent = Simulator::Schedule (GetBeaconInterval (), &MeshnetWifiInterfaceMac::SendBeacon, this);
}
void
MeshnetWifiInterfaceMac::SendBeacon ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG (GetAddress () << " is sending beacon");

  NS_ASSERT (!m_beaconSendEvent.IsRunning ());

  // Form & send beacon
  MeshnetWifiBeacon beacon (GetSsid (), GetSupportedRates (), m_beaconInterval.GetMicroSeconds ());

  // Ask all plugins to add their specific information elements to beacon
  for (PluginList::const_iterator i = m_plugins.begin (); i != m_plugins.end (); ++i)
    {
      (*i)->UpdateBeacon (beacon);
    }
  m_dca->Queue (beacon.CreatePacket (), beacon.CreateHeader (GetAddress (), GetMeshnetPointAddress ()));

  ScheduleNextBeacon ();
}
void
MeshnetWifiInterfaceMac::Receive (Ptr<Packet> packet, WifiMacHeader const *hdr)
{
  // Process beacon
  if ((hdr->GetAddr1 () != GetAddress ()) && (hdr->GetAddr1 () != Mac48Address::GetBroadcast ()))
    {
      return;
    }
  if (hdr->IsBeacon ())
    {
      m_stats.recvBeacons++;
      MgtBeaconHeader beacon_hdr;

      packet->PeekHeader (beacon_hdr);

      NS_LOG_DEBUG ("Beacon received from " << hdr->GetAddr2 () << " I am " << GetAddress () << " at "
                                            << Simulator::Now ().GetMicroSeconds () << " microseconds");

      // update supported rates
      if (beacon_hdr.GetSsid ().IsEqual (GetSsid ()))
        {
          SupportedRates rates = beacon_hdr.GetSupportedRates ();

          for (uint32_t i = 0; i < m_phy->GetNModes (); i++)
            {
              WifiMode mode = m_phy->GetMode (i);
              if (rates.IsSupportedRate (mode.GetDataRate ()))
                {
                  m_stationManager->AddSupportedMode (hdr->GetAddr2 (), mode);
                  if (rates.IsBasicRate (mode.GetDataRate ()))
                    {
                      m_stationManager->AddBasicMode (mode);
                    }
                }
            }
        }
    }
  else
    {
      m_stats.recvBytes += packet->GetSize ();
      m_stats.recvFrames++;
    }
  // Filter frame through all installed plugins
  for (PluginList::iterator i = m_plugins.begin (); i != m_plugins.end (); ++i)
    {
      bool drop = !((*i)->Receive (packet, *hdr));
      if (drop)
        {
          return; // plugin drops frame
        }
    }
  // Check if QoS tag exists and add it:
  if (hdr->IsQosData ())
    {
      packet->AddPacketTag (QosTag (hdr->GetQosTid ()));
    }
  // Forward data up
  if (hdr->IsData ())
    {
      ForwardUp (packet, hdr->GetAddr4 (), hdr->GetAddr3 ());
    }

  // We don't bother invoking RegularWifiMac::Receive() here, because
  // we've explicitly handled all the frames we care about. This is in
  // contrast to most classes which derive from RegularWifiMac.
}
uint32_t
MeshnetWifiInterfaceMac::GetLinkMetric (Mac48Address huperAddress)
{
  uint32_t metric = 1;
  if (!m_linkMetricCallback.IsNull ())
    {
      metric = m_linkMetricCallback (huperAddress, this);
    }
  return metric;
}
void
MeshnetWifiInterfaceMac::SetLinkMetricCallback (Callback<uint32_t, Mac48Address, Ptr<MeshnetWifiInterfaceMac> > cb)
{
  m_linkMetricCallback = cb;
}
void
MeshnetWifiInterfaceMac::SetMeshnetPointAddress (Mac48Address a)
{
  m_mpAddress = a;
}
Mac48Address
MeshnetWifiInterfaceMac::GetMeshnetPointAddress () const
{
  return m_mpAddress;
}
//Statistics:
MeshnetWifiInterfaceMac::Statistics::Statistics () :
  recvBeacons (0), sentFrames (0), sentBytes (0), recvFrames (0), recvBytes (0)
{
}
void
MeshnetWifiInterfaceMac::Statistics::Print (std::ostream & os) const
{
  os << "<Statistics "
  /// \todo txBeacons
  "rxBeacons=\"" << recvBeacons << "\" "
  "txFrames=\"" << sentFrames << "\" "
  "txBytes=\"" << sentBytes << "\" "
  "rxFrames=\"" << recvFrames << "\" "
  "rxBytes=\"" << recvBytes << "\"/>" << std::endl;
}
void
MeshnetWifiInterfaceMac::Report (std::ostream & os) const
{
  os << "<Interface "
  "BeaconInterval=\"" << GetBeaconInterval ().GetSeconds () << "\" "
  "Channel=\"" << GetFrequencyChannel () << "\" "
  "Address = \"" << GetAddress () << "\">" << std::endl;
  m_stats.Print (os);
  os << "</Interface>" << std::endl;
}
void
MeshnetWifiInterfaceMac::ResetStats ()
{
  m_stats = Statistics ();
}

void
MeshnetWifiInterfaceMac::FinishConfigureStandard (enum WifiPhyStandard standard)
{
  RegularWifiMac::FinishConfigureStandard (standard);
  m_standard = standard;

  // We use the single DCF provided by WifiMac for the purpose of
  // Beacon transmission. For this we need to reconfigure the channel
  // access parameters slightly, and do so here.
  m_dca->SetMinCw (0);
  m_dca->SetMaxCw (0);
  m_dca->SetAifsn (1);
}
WifiPhyStandard
MeshnetWifiInterfaceMac::GetPhyStandard () const
{
  return m_standard;
}
} // namespace ns3

