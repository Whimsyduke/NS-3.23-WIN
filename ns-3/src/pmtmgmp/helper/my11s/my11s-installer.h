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

#ifndef MY11S_STACK_INSTALLER_H
#define MY11S_STACK_INSTALLER_H

#include "ns3/wmn-stack-installer.h"

namespace ns3 {

/**
 * \brief Helper class to allow easy installation of 802.11s stack.
 */
class My11sStack : public WmnStack
{
public:
  
  static TypeId GetTypeId ();

  /**
   * Create a My11sStack() installer helper.
   */
  My11sStack ();

  /**
   * Destroy a My11sStack() installer helper.
   */
  ~My11sStack ();

  /**
   * Break any reference cycles in the installer helper.  Required for ns-3
   * Object support.
   */
  void DoDispose ();

  /**
   * \brief Install an 802.11s stack.
   * \param mp The Ptr<WmnPointDevice> to use when setting up the PMP.
   */
  bool InstallStack (Ptr<WmnPointDevice> mp);
 
  /**
   * \brief Iterate through the referenced devices and protocols and print
   * their statistics
   */
  void Report (const Ptr<WmnPointDevice> mp, std::ostream&);

  /**
   * \brief Reset the statistics on the referenced devices and protocols.
   */
  void ResetStats (const Ptr<WmnPointDevice> mp);
private:
  Mac48Address m_root;
};

} // namespace ns3

#endif

