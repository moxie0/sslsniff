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

#ifndef __HTTP_FINGERPRINTER_H__
#define __HTTP_FINGERPRINTER_H__

#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <string>
#include <set>
#include <vector>
#include "HttpBridge.hpp"
#include "OCSPDenier.hpp"

#include "../certificate/CertificateManager.hpp"

using namespace boost::asio;

class HttpConnectionManager {

 private:
  ip::tcp::acceptor acceptor_;
  int port_;

  CertificateManager &certificateManager;
  bool denyOCSP;

  void acceptIncomingConnection();

  void bridgeHttpRequest(boost::shared_ptr<ip::tcp::socket> socket,
			 ip::tcp::endpoint destination);

  void handleClientConnection(boost::shared_ptr<ip::tcp::socket> socket, 
			      const boost::system::error_code& error);

  void handleServerConnection(HttpBridge::ptr bridge, const boost::system::error_code& error);

 public:
  HttpConnectionManager(io_service& io_service, int port, 
			CertificateManager &certificateManager, bool denyOCSP);

};

#endif
