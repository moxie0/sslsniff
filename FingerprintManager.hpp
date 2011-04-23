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

#ifndef __FINGERPRINT_MANAGER_H__
#define __FINGERPRINT_MANAGER_H__

#include <string>
#include <map>
#include <vector>
#include <boost/asio.hpp>

#include "http/HttpBridge.hpp"

using namespace boost::asio;

class FingerprintManager : public HttpBridgeListener {

private:
  static FingerprintManager *manager;

  std::map<ip::address, std::string> fingerprints;
  std::vector<std::string> validAgents;

  std::string* getUserAgentFor(ip::address &address);

public:
  
  static FingerprintManager* getInstance();

  void setValidUserAgents(std::string &validAgents);
  virtual void setUserAgentFor(ip::address &address, std::string &userAgent);
  bool isValidTarget(ip::address &address);

  bool isValidWildcardTarget(ip::address &address);
};


#endif
