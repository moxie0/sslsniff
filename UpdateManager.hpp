/*
 * Copyright (c) 2002-2009 Moxie Marlinspike
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#ifndef __UPDATE_MANAGER_H__
#define __UPDATE_MANAGER_H__

#include <string>
#include <list>
#include <boost/asio.hpp>

using namespace boost::asio;

class UpdateManager {

private:
  static UpdateManager* updateManager;
  std::list<boost::asio::ip::address> updateAddresses;  
  std::list<boost::asio::ip::address> addonsAddresses;

  std::string updatePath;
  std::string addonPath;
  std::string addonHash;

  bool isAddressInList(ip::tcp::endpoint &endpoint, std::list<boost::asio::ip::address> addresses);

  UpdateManager() {}

public:
  static UpdateManager* getInstance();
  void initialize(std::string &updatePath, std::string &addonPath, std::string &addonHash);

  bool isUpdatableRequest(std::string &buildId, std::string &buildTarget);
  bool isUpdateTarget(boost::asio::ip::tcp::endpoint &endpoint);
  bool isAddonTarget(ip::tcp::endpoint &endpoint);


  void getUpdatePath(std::string &buildTarget, std::string &result);
  std::string getAddonPath();
  std::string getAddonHash();
};


#endif
