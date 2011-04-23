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

#ifndef __HTTP_BRIDGE_H__
#define __HTTP_BRIDGE_H__

#define READING_HEADERS 1
#define SHUFFLING_DATA  2

#define BUFFER_SIZE 4096

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <string>

#include "HttpHeaders.hpp"
#include "../Bridge.hpp"

using namespace boost::asio;

class HttpBridgeListener;

class HttpBridge : public Bridge {

public:

  static ptr create(boost::shared_ptr<ip::tcp::socket> clientSocket,
		    io_service& io_service, 
		    HttpBridgeListener *listener) 
  {
    return ptr(new HttpBridge(clientSocket, io_service, listener));
  }

  virtual ip::tcp::socket& getClientSocket() {
    return *clientSocket;
  }

  virtual ip::tcp::socket& getServerSocket() {
    return serverSocket;
  }

  virtual void shuttle();
  virtual void close();

protected:
  virtual void processClientInput(char *buffer, int length);

private:
  int closed;
  io_service& io_service_;
  boost::shared_ptr<ip::tcp::socket> clientSocket;
  ip::tcp::socket serverSocket;
  
  unsigned int state;
  HttpHeaders headers;
  HttpBridgeListener *listener;

  HttpBridge(boost::shared_ptr<ip::tcp::socket> clientSocket,
	     io_service& io_service, HttpBridgeListener *listener)
    : clientSocket(clientSocket),
      serverSocket(io_service),
      io_service_(io_service),
      state(READING_HEADERS) 
  {
    this->listener = listener;
  }

  void setFinishedWithHeaders();  
};

class HttpBridgeListener {
public:
  virtual void setUserAgentFor(ip::address &address, std::string &userAgent) = 0;
};

#endif
