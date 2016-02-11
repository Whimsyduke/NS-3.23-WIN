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

	NS_LOG_COMPONENT_DEFINE("MgmpRtable");

	namespace hu11s {

		NS_OBJECT_ENSURE_REGISTERED(MgmpRtable);

		TypeId
			MgmpRtable::GetTypeId()
		{
			static TypeId tid = TypeId("ns3::hu11s::MgmpRtable")
				.SetParent<Object>()
				.SetGroupName("Meshnet")
				.AddConstructor<MgmpRtable>()
				.AddAttribute("RefreshTime",
					"Refresh level and main path.",
					TimeValue(MicroSeconds(1024 * 5000)),
					MakeTimeAccessor(
						&MgmpRtable::m_refreshTime),
					MakeTimeChecker()
					);
			return tid;
		}
		MgmpRtable::MgmpRtable()
#ifndef HUMGMP_UNUSED_MY_CODE
			:
				m_level(255),
				m_refreshTime(MicroSeconds(1024 * 5000))
#endif
		{
			DeleteProactivePath();
		}
		MgmpRtable::~MgmpRtable()
		{
		}
		void
			MgmpRtable::DoDispose()
		{
			m_routes.clear();
		}
		void
			MgmpRtable::AddReactivePath(Mac48Address destination, Mac48Address retransmitter, uint32_t interface,
				uint32_t metric, Time lifetime, uint32_t seqnum)
		{
			std::map<Mac48Address, ReactiveRoute>::iterator i = m_routes.find(destination);
			if (i == m_routes.end())
			{
				ReactiveRoute newroute;
				m_routes[destination] = newroute;
			}
			i = m_routes.find(destination);
			NS_ASSERT(i != m_routes.end());
			i->second.retransmitter = retransmitter;
			i->second.interface = interface;
			i->second.metric = metric;
			i->second.whenExpire = Simulator::Now() + lifetime;
			i->second.seqnum = seqnum;
		}
		void
			MgmpRtable::AddProactivePath(uint32_t metric, Mac48Address root, Mac48Address retransmitter,
				uint32_t interface, Time lifetime, uint32_t seqnum)
		{
			m_root.root = root;
			m_root.retransmitter = retransmitter;
			m_root.metric = metric;
			m_root.whenExpire = Simulator::Now() + lifetime;
			m_root.seqnum = seqnum;
			m_root.interface = interface;
		}
		void
			MgmpRtable::AddPrecursor(Mac48Address destination, uint32_t precursorInterface,
				Mac48Address precursorAddress, Time lifetime)
		{
			Precursor precursor;
			precursor.interface = precursorInterface;
			precursor.address = precursorAddress;
			precursor.whenExpire = Simulator::Now() + lifetime;
			std::map<Mac48Address, ReactiveRoute>::iterator i = m_routes.find(destination);
			if (i != m_routes.end())
			{
				bool should_add = true;
				for (unsigned int j = 0; j < i->second.precursors.size(); j++)
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
					i->second.precursors.push_back(precursor);
				}
			}
		}
		void
			MgmpRtable::DeleteProactivePath()
		{
			m_root.precursors.clear();
			m_root.interface = INTERFACE_ANY;
			m_root.metric = MAX_METRIC;
			m_root.retransmitter = Mac48Address::GetBroadcast();
			m_root.seqnum = 0;
			m_root.whenExpire = Simulator::Now();
		}
		void
			MgmpRtable::DeleteProactivePath(Mac48Address root)
		{
			if (m_root.root == root)
			{
				DeleteProactivePath();
			}
		}
		void
			MgmpRtable::DeleteReactivePath(Mac48Address destination)
		{
			std::map<Mac48Address, ReactiveRoute>::iterator i = m_routes.find(destination);
			if (i != m_routes.end())
			{
				m_routes.erase(i);
			}
		}
		MgmpRtable::LookupResult
			MgmpRtable::LookupReactive(Mac48Address destination)
		{
			std::map<Mac48Address, ReactiveRoute>::iterator i = m_routes.find(destination);
			if (i == m_routes.end())
			{
				return LookupResult();
			}
			if ((i->second.whenExpire < Simulator::Now()) && (i->second.whenExpire != Seconds(0)))
			{
				NS_LOG_DEBUG("Reactive route has expired, sorry.");
				return LookupResult();
			}
			return LookupReactiveExpired(destination);
		}
		MgmpRtable::LookupResult
			MgmpRtable::LookupReactiveExpired(Mac48Address destination)
		{
			std::map<Mac48Address, ReactiveRoute>::iterator i = m_routes.find(destination);
			if (i == m_routes.end())
			{
				return LookupResult();
			}
			return LookupResult(i->second.retransmitter, i->second.interface, i->second.metric, i->second.seqnum,
				i->second.whenExpire - Simulator::Now());
		}
		MgmpRtable::LookupResult
			MgmpRtable::LookupProactive()
		{
			if (m_root.whenExpire < Simulator::Now())
			{
				NS_LOG_DEBUG("Proactive route has expired and will be deleted, sorry.");
				DeleteProactivePath();
			}
			return LookupProactiveExpired();
		}
		MgmpRtable::LookupResult
			MgmpRtable::LookupProactiveExpired()
		{
			return LookupResult(m_root.retransmitter, m_root.interface, m_root.metric, m_root.seqnum,
				m_root.whenExpire - Simulator::Now());
		}
		std::vector<MgmpProtocol::FailedDestination>
			MgmpRtable::GetUnreachableDestinations(Mac48Address huperAddress)
		{
			MgmpProtocol::FailedDestination dst;
			std::vector<MgmpProtocol::FailedDestination> retval;
			for (std::map<Mac48Address, ReactiveRoute>::iterator i = m_routes.begin(); i != m_routes.end(); i++)
			{
				if (i->second.retransmitter == huperAddress)
				{
					dst.destination = i->first;
					i->second.seqnum++;
					dst.seqnum = i->second.seqnum;
					retval.push_back(dst);
				}
			}
			//Lookup a path to root
			if (m_root.retransmitter == huperAddress)
			{
				dst.destination = m_root.root;
				dst.seqnum = m_root.seqnum;
				retval.push_back(dst);
			}
			return retval;
		}
		MgmpRtable::PrecursorList
			MgmpRtable::GetPrecursors(Mac48Address destination)
		{
			//We suppose that no duplicates here can be
			PrecursorList retval;
			std::map<Mac48Address, ReactiveRoute>::iterator route = m_routes.find(destination);
			if (route != m_routes.end())
			{
				for (std::vector<Precursor>::const_iterator i = route->second.precursors.begin();
				i != route->second.precursors.end(); i++)
				{
					if (i->whenExpire > Simulator::Now())
					{
						retval.push_back(std::make_pair(i->interface, i->address));
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
		MgmpRtable::LookupResult::LookupResult(Mac48Address r, uint32_t i, uint32_t m, uint32_t s, Time l) :
			retransmitter(r), ifIndex(i), metric(m), seqnum(s), lifetime(l)
		{
		}
		bool
			MgmpRtable::LookupResult::IsValid() const
		{
			return !(retransmitter == Mac48Address::GetBroadcast() && ifIndex == INTERFACE_ANY && metric == MAX_METRIC
				&& seqnum == 0);
		}
#ifndef HUMGMP_UNUSED_MY_CODE
		MgmpRtable::ProactiveTree::ProactiveTree()
		{
		}
		MgmpRtable::ProactiveTree::~ProactiveTree()
		{
			m_routes.clear();
		}
		void MgmpRtable::ProactiveTree::AddProactivePath(uint32_t metric, Mac48Address root, Mac48Address retransmitter, uint32_t interface, Time lifetime, uint32_t seqnum, std::vector<Mac48Address> path)
		{
			std::string key = MgmpRtable::GetPathString(root, path);
			std::map<std::string, ProactiveRoute>::iterator route = m_routes.find(key);
			if (route == m_routes.end())
			{
				ProactiveRoute newRoute;
				m_routes[key] = newRoute;
			}
			route->second.root = root;
			route->second.retransmitter = retransmitter;
			route->second.metric = metric;
			route->second.whenExpire = Simulator::Now() + lifetime;
			route->second.seqnum = seqnum;
			route->second.interface = interface;
			route->second.path = path;
		}
		void MgmpRtable::ProactiveTree::DeleteProactivePath(Mac48Address root, std::vector<Mac48Address> path)
		{
			std::string key = MgmpRtable::GetPathString(root, path);
			std::map<std::string, ProactiveRoute>::iterator route = m_routes.find(key);
			if (route != m_routes.end())
			{
				if (&(route->second) == m_mainRoute)
				{
					m_routes.erase(route);
					RefreshMainRoute();
				}
				else
				{
					m_routes.erase(route);
				}
			}
		}
		void MgmpRtable::ProactiveTree::RefreshMainRoute()
		{
			if (m_routes.size() == 0)
			{
				m_mainRoute = nullptr;
				return;
			}
			std::map<std::string, ProactiveRoute>::iterator main = m_routes.begin();
			for (std::map<std::string, ProactiveRoute>::iterator iter = m_routes.begin(); iter != m_routes.end(); iter++)
			{
				if (iter->second.metric < main->second.metric)
				{
					main = iter;
				}
			}
			m_mainRoute = &(main->second);
		}
		Ptr<MgmpRtable::ProactiveRoute> MgmpRtable::ProactiveTree::GetMiniRoute()
		{
			std::map<std::string, ProactiveRoute>::iterator minRoute = m_routes.begin();
			if (minRoute == m_routes.end()) return nullptr;
			uint8_t level = minRoute->second.path.size();
			for (std::map<std::string, ProactiveRoute>::iterator iter = m_routes.begin(); iter != m_routes.end(); iter++)
			{
				if (iter->second.path.size() < level)
				{
					level = iter->second.path.size();
					minRoute = iter;
				}
			}
			return &(minRoute->second);
		}
		MgmpRtable::ProactiveRoute MgmpRtable::ProactiveTree::GetBestRoute()
		{
			std::map<std::string, ProactiveRoute>::iterator minRoute = m_routes.begin();
			if (minRoute == m_routes.end())
			{
				ProactiveRoute newRoute;
				newRoute.precursors.clear();
				newRoute.interface = INTERFACE_ANY;
				newRoute.metric = MAX_METRIC;
				newRoute.retransmitter = Mac48Address::GetBroadcast();
				newRoute.seqnum = 0;
				newRoute.whenExpire = Simulator::Now();
				return newRoute;
			}
			uint32_t metric = minRoute->second.metric;
			for (std::map<std::string, ProactiveRoute>::iterator iter = m_routes.begin(); iter != m_routes.end(); iter++)
			{
				if (iter->second.metric < metric)
				{
					metric = iter->second.metric;
					minRoute = iter;
				}
			}
			return minRoute->second;
		}
		void MgmpRtable::ProactiveTree::CheckExpire()
		{
			bool NotEnd = true;
			while (NotEnd)
			{
				std::map<std::string, ProactiveRoute>::iterator iter = m_routes.begin();
				while (true)
				{
					if (iter == m_routes.end())
					{
						NotEnd = false;
						break;
					}
					if (iter->second.whenExpire < Simulator::Now())
					{
						m_routes.erase(iter);
						break;
					}
					else
					{
						iter++;
					}
				}
			}
		}
		void MgmpRtable::AddProactivePath(uint32_t metric, Mac48Address root, Mac48Address retransmitter, uint32_t interface, Time lifetime, uint32_t seqnum, std::vector<Mac48Address> path)
		{
			std::map<Mac48Address, ProactiveTree>::iterator tree = m_trees.find(root);
			if (tree == m_trees.end())
			{
				ProactiveTree newTree;
				m_trees[root] = newTree;
			}
			tree->second.AddProactivePath(metric, root, retransmitter, interface, lifetime, seqnum, path);
		}
		void MgmpRtable::ClearProactivePath()
		{
			m_trees.clear();
		}
		void MgmpRtable::DeleteProactiveTree(Mac48Address root)
		{
			std::map<Mac48Address, ProactiveTree>::iterator tree = m_trees.find(root);
			if (tree != m_trees.end())
			{
				m_trees.erase(tree);
			}
		}
		void MgmpRtable::DeleteProactivePath(Mac48Address root, std::vector<Mac48Address> path)
		{
			std::map<Mac48Address, ProactiveTree>::iterator tree = m_trees.find(root);
			if (tree != m_trees.end())
			{
				tree->second.DeleteProactivePath(root, path);
			}
		}
		std::string MgmpRtable::GetPathString(Mac48Address root, std::vector<Mac48Address> path)
		{
			std::ostringstream os;
			os << root;
			for (std::vector<Mac48Address>::const_iterator iter = path.begin(); iter != path.end(); iter++)
			{
				os << ":" << *iter;
			}
			return os.str();
		}
		void MgmpRtable::AutoRefresh()
		{
			m_level = 255;
			for (std::map<Mac48Address, ProactiveTree>::iterator iter = m_trees.begin(); iter != m_trees.end(); iter++)
			{
				iter->second.RefreshMainRoute();
				Ptr<ProactiveRoute> route = iter->second.GetMiniRoute();
				if (round == nullptr) continue;
				uint8_t level = route->path.size();
				if (level < m_level)
				{
					m_level = level;
					m_levelRoute = route;
				}
			}
		}
		uint8_t MgmpRtable::GetLevel()
		{
			return m_level;
		}
		MgmpRtable::LookupResult MgmpRtable::LookupProactive(Mac48Address root)
		{
			std::map<Mac48Address, ProactiveTree>::iterator tree = m_trees.find(root);
			if (tree == m_trees.end())
			{
				ProactiveTree newTree;
				m_trees[root] = newTree;
			}
			else
			{
				tree->second.CheckExpire();
			}
			return LookupProactiveExpired(root);
		}
		MgmpRtable::LookupResult MgmpRtable::LookupProactiveExpired(Mac48Address root)
		{
			std::map<Mac48Address, ProactiveTree>::iterator tree = m_trees.find(root);
			if (tree == m_trees.end())
			{
				ProactiveTree newTree;
				m_trees[root] = newTree;
			}
			ProactiveRoute best = tree->second.GetBestRoute();
			return LookupResult(best.retransmitter, best.interface, best.metric, best.seqnum,
				best.whenExpire - Simulator::Now());
		}
#endif
	} // namespace hu11s
} // namespace ns3
