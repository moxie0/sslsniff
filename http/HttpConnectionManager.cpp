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


#include "HttpConnectionManager.hpp"

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include <iostream>
#include <string>
#include <map>

#include "../util/Destination.hpp"
#include "../util/Util.hpp"
#include "../FingerprintManager.hpp"
#include "../FirefoxUpdater.hpp"
#include "../UpdateManager.hpp"

// Public

using namespace boost::asio;

HttpConnectionManager::HttpConnectionManager(io_service& io_service, int port,
					     CertificateManager &certificateManager,
					     bool denyOCSP) 
  : acceptor_(io_service, ip::tcp::endpoint(ip::tcp::v4(), port)),
    port_(port),
    certificateManager(certificateManager),
    denyOCSP(denyOCSP)
{
  if (port != -1)
    acceptIncomingConnection();

}

void HttpConnectionManager::acceptIncomingConnection() {
  boost::shared_ptr<ip::tcp::socket> socket(new ip::tcp::socket(acceptor_.io_service()));

  acceptor_.async_accept(*socket, boost::bind(&HttpConnectionManager::handleClientConnection,
					      this, socket, placeholders::error));
  
}

void HttpConnectionManager::bridgeHttpRequest(boost::shared_ptr<ip::tcp::socket> socket,
					      ip::tcp::endpoint destination)
{
  Bridge::ptr bridge = HttpBridge::create(socket, acceptor_.io_service(), 
					  FingerprintManager::getInstance());
  
  bridge->getServerSocket().
    async_connect(destination, boost::bind(&HttpConnectionManager::handleServerConnection, 
					   this, bridge, placeholders::error));
}

void HttpConnectionManager::handleClientConnection(boost::shared_ptr<ip::tcp::socket> socket, 
						   const boost::system::error_code& error) 
{
  if (error) {
    socket->close();
    Logger::logError("HTTP Accept Error...");
    return;
  }

  try {    
    ip::tcp::endpoint destination;    
    Destination::getOriginalDestination(*socket, destination);

    if (denyOCSP && certificateManager.isOCSPAddress(destination))
      OCSPDenier::getInstance()->denyOCSPRequest(socket);
    else
      bridgeHttpRequest(socket, destination);

  } catch (IndeterminateDestinationException &exception) {    
    std::cerr << "Error: Could not determine original destination..." << std::endl;    
  }

  acceptIncomingConnection();  
}

void HttpConnectionManager::handleServerConnection(Bridge::ptr bridge, 
						   const boost::system::error_code& error)
{
  if (!error) {
    bridge->shuttle();
  } else {
    Logger::logError("HTTP Connect Error");
    bridge->close();
  }
}

