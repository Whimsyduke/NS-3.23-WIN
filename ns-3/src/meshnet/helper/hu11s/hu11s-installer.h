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
 */

#ifndef HU11S_STACK_INSTALLER_H
#define HU11S_STACK_INSTALLER_H

#include "ns3/meshnet-stack-installer.h"

namespace ns3 {

/**
 * \brief Helper class to allow easy installation of 802.11s stack.
 */
class Hu11sStack : public MeshnetStack
{
public:
  
  static TypeId GetTypeId ();

  /**
   * Create a Hu11sStack() installer helper.
   */
  Hu11sStack ();

  /**
   * Destroy a Hu11sStack() installer helper.
   */
  ~Hu11sStack ();

  /**
   * Break any reference cycles in the installer helper.  Required for ns-3
   * Object support.
   */
  void DoDispose ();

  /**
   * \brief Install an 802.11s stack.
   * \param mp The Ptr<MeshnetPointDevice> to use when setting up the PMP.
   */
  bool InstallStack (Ptr<MeshnetPointDevice> mp);
 
  /**
   * \brief Iterate through the referenced devices and protocols and print
   * their statistics
   */
  void Report (const Ptr<MeshnetPointDevice> mp, std::ostream&);

  /**
   * \brief Reset the statistics on the referenced devices and protocols.
   */
  void ResetStats (const Ptr<MeshnetPointDevice> mp);
private:
  Mac48Address m_root;
};

} // namespace ns3

#endif

