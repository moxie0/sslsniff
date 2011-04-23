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

#ifndef __CONNECTION_MANAGER_H__
#define __CONNECTION_MANAGER_H__

#include "certificate/CertificateManager.hpp"
#include "http/HttpConnectionManager.hpp"

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

using namespace boost::asio;

class SSLConnectionManager {

private:
  static const short HTTPS_PORT = 443;

  CertificateManager &certificateManager;
  ip::tcp::acceptor acceptor;

  void acceptIncomingConnection();

  void handleClientConnection(boost::shared_ptr<ip::tcp::socket> socket, 
			      const boost::system::error_code &error);

  void interceptSSL(boost::shared_ptr<ip::tcp::socket> clientSocket,
		    ip::tcp::endpoint &destination,
		    bool wildcardOK);

  void interceptUpdate(boost::shared_ptr<ip::tcp::socket> clientSocket,
		       ip::tcp::endpoint &destination,
		       bool wildcardOK);

  void interceptAddon(boost::shared_ptr<ip::tcp::socket> clientSocket,
		      ip::tcp::endpoint &destination,
		      bool wildcardOK);

  void interceptConnection(boost::shared_ptr<ip::tcp::socket> clientSocket,
			   ip::tcp::endpoint destination,
			   bool wildcardOK);

  void shuttleConnection(boost::shared_ptr<ip::tcp::socket> clientSocket,
			 ip::tcp::endpoint &destination);

public:

  SSLConnectionManager(io_service &io_service, 
		       CertificateManager &certificateManager, 
		       int sslPort);
  
};


#endif
