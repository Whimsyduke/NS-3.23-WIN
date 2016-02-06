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
 * Authors:     Kirill Andreev <andreev@itp.ru>      
 *              Aleksander Safonov <safa@iitp.ru>
 *              Pavel Boyko <boyko@iitp.ru>
 *
 * This is top level meshnet module description
 */

/**
 * \defgroup meshnet Meshnet Device
 *
 * \brief MAC-layer mobile meshnet networking.
 * \section MeshnetOverview Overview of Layer-2 Meshnet networking protocols
 * 
 * The main goal of this module is to provide MAC-layer routing functionality.
 
 * The main part of MAC-layer routing model is specific type of a network device -- 
 * ns3::MeshnetPointDevice. Being an interface to upper-layer protocols, it provides routing functionality
 * hidden from upper-layer protocols, by means of ns3::MeshnetL2RoutingProtocol. 
 * 
 * Our model supports stations with multiple network devices handled by a single
 * MAC-layer routing protocol. So, ns3::MeshnetPointDevice serves as an umbrella to multiple 
 * network devices ("interfaces")  working under the same MAC-layer routing protocol.
 * 
 * Network devices may be of different types, each with a specific medium access method.
 * So ns3::MeshnetL2RoutingProtocol consists of two parts: the one independent from the network device type, 
 * which we refer to as a routing protocol, and the other one depended on the network device type which
 * we refer to as a plug-in to the routing protocol.
 * 
 * One can imagine a MAC-layer routing as a two-tier model. ns3::MeshnetL2RoutingProtocol and ns3::MeshnetPointDevice 
 * belong to the upper tier. The task of ns3::MeshnetPointDevice is to send, receive, and forward frames, 
 * while the task of ns3::MeshnetL2RoutingProtocol is to resolve routes and keep frames waiting for route resolution. 
 * This functionality is independent from the types of underlying network devices ("interfaces").
 * 
 * The lower tier implements the part of MAC-layer routing, specific for underlying network devices
 * and their medium access control methods. For example, MGMP routing protocol in IEEE802.11s
 * uses its own specific management frames.
 * 
 * At present, two routing protocols are implemented in this module:
 *      - MGMP (default routing protocol for IEEE802.11s standard) + Huper management protocol 
 *      (also described in 802.11s standard draft) which is required by MGMP to manage huper links 
 *      (it works like association mechanism in IEEE802.11).
 *      - FLAME (Forwarding LAyer for MEshing).
 
 * While MGMP only works with 802.11-MAC, FLAME works with all types of network devices, which support
 * 48-bit MAC-addressing scheme.
 * 
 * \subsection Architecture Architecture of MAC-layer routing stack
 * As already mentioned, MAC-layer routing consists of two tiers.
 * An ns3::MeshnetPointDevice which forwards frames by using an attached ns3::MeshnetL2RoutingProtocol forms 
 * the upper tier. The interface between ns3::MeshnetPointDevice and the upper-layer protocols is inherited 
 * from ns3::NetDevice class. The ns3::MeshnetPointDevice interacts with ns3::MeshnetL2RoutingProtocol as follows:
 * ns3::MeshnetPointDevice gives to ns3::MeshnetL2RoutingProtocol a frame with the source and destination addresses,
 * the network device index which the frame is received from, and a callback to be executed when the route is found.
 * The callback is needed because all routing queues are implemented inside ns3::MeshnetL2RoutingProtocol. 
 * When the route is resolved, ns3::MeshnetL2RoutingProtocol returns the frame back to ns3::MeshnetPointDevice with the 
 * network device index which the packet shall be sent to. All additional routing information is stored inside 
 * the frame by means of tags. In the end, when all these routines are done, the frame goes to the lower tier.
 
 * The lower tier is responsible for filling MAC-specific headers. At present, we have only implemented the
 * lower tier which is specific for ns3::WifiNetDevice. This tier is implemented as two base classes: 
 * ns3::MeshnetWifiInterfaceMac and ns3::MeshnetWifiInterfaceMacPlugin. The former is a new kind of WifiMac. If beacon 
 * generation is enabled or disabled, it implements IEEE802.11s meshnet functionality or a simple ad hoc functionality 
 * of the MAC-high part of the WiFi model, respectively. The latter is a plug-in to L2Routing protocol. 
 * It handles all outgoing and incoming frames, fills headers and make decisions to drop a frame or not. Also, it 
 * adds information elements to beacons specific to given L2Routing protocol, if needed.
 * \image html MeshnetArchitecture.png "Overview of the Meshnet MAC-layer routing system"
 * 
 * \subsection NewProtocol Adding a new protocol
 * This module requires all the network devices operate with ns3::Mac48Address addressing scheme.
 * 
 * To add a new L2Routing protocol, one needs to define the following:
 *      - Write an upper part of the protocol inherited from ns3::MeshnetL2Routing.
 *      - If the protocol works only with 802.11 MAC -- write a plug-in inherited from ns3::MeshnetWifiInterfaceMacPlugin
 *      - If the protocol works with other types of network devices -- write your own plug-in (interface for
 *      communication with other types of network devices is not implemented).
 * 
 * When you implement a L2Routing protocol, remember that when you are at L2Routing tier, 
 * you work with a frame without an LLC header, and when you are at plug-in tier using 
 * ns3::MeshnetWifiInterfaceMacPlugin, an LLC header is already attached (by WifiNetDevice)
 * 
 * \attention Note, when you use ns3::MeshnetWifiInterfaceMac, multiple plug-ins may be installed. 
 * 
 * \subsection Statistics
 * Each L2Routing protocol has a structure to capture statistics, Report and ResetStats methods.
 * This gives an opportunity to collect statistic to an *.xml file periodically.
 */
