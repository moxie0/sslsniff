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

#include "UpdateManager.hpp"
#include "util/Util.hpp"

#include <boost/filesystem.hpp>

#define BUILD_ID    "2010011112"
#define UPDATE_ADDRESS "aus2.mozilla.org"
#define ADDONS_ADDRESS "versioncheck.addons.mozilla.org"

using namespace boost::asio;

UpdateManager* UpdateManager::updateManager = new UpdateManager();

UpdateManager* UpdateManager::getInstance() {
  return updateManager;
}

void UpdateManager::initialize(std::string &updatePath, std::string &addonPath, std::string &addonHash) {
  this->updatePath = updatePath;
  this->addonPath  = addonPath;
  this->addonHash  = addonHash;
  
  std::string updateUrl(UPDATE_ADDRESS);
  std::string addonsUrl(ADDONS_ADDRESS);

  Util::resolveName(updateUrl, updateAddresses);
  Util::resolveName(addonsUrl, addonsAddresses);
}

bool UpdateManager::isUpdatableRequest(std::string &buildId, std::string &buildTarget) {
  return 
    buildId != BUILD_ID &&
    boost::filesystem::exists(updatePath + "/" + buildTarget + ".xml");
}

bool UpdateManager::isAddressInList(ip::tcp::endpoint &endpoint, std::list<boost::asio::ip::address> addresses) {
  ip::address address                  = endpoint.address();
  std::list<ip::address>::iterator i   = addresses.begin();
  std::list<ip::address>::iterator end = addresses.end();
  
  for ( ; i != end ; i++) {
    if ((*i) == address) return true;
  }
  
  return false;  
}

bool UpdateManager::isAddonTarget(ip::tcp::endpoint &endpoint) {
  if (addonPath.empty()) return false;

  return isAddressInList(endpoint, addonsAddresses);
}

bool UpdateManager::isUpdateTarget(ip::tcp::endpoint &endpoint) {
  if (updatePath.empty()) return false;
  
  return isAddressInList(endpoint, updateAddresses);
}

void UpdateManager::getUpdatePath(std::string &buildTarget, std::string &result) {
  result = updatePath + "/" + buildTarget + ".xml";
}

std::string UpdateManager::getAddonPath() {
  return addonPath;
}

std::string UpdateManager::getAddonHash() {
  return addonHash;
}
