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

#ifndef __FIREFOX_ADDON_UPDATER_H__
#define __FIREFOX_ADDON_UPDATER_H__

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

#include "FirefoxUpdater.hpp"

using namespace boost::asio;

class FirefoxAddonUpdater : public FirefoxUpdater {

private:
  std::map<std::string, std::string> requestArguments;

  std::string incrementVersion(std::string &version);

  bool parseArguments(std::string &request, std::vector<std::string> &arguments);
  std::string truncateVersion(std::string &version);

protected:
  virtual bool parseMetaRequest(std::string &request);
  
public:

  virtual void sendMetaUpdateResponse();

  FirefoxAddonUpdater(boost::shared_ptr<ip::tcp::socket> clientSocket,
		      ip::tcp::endpoint &destination)
    : FirefoxUpdater(clientSocket, destination) {}

};

#endif
