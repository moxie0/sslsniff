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

#ifndef __RAW_BRIDGE_H__
#define __RAW_BRIDGE_H__

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

#include "Bridge.hpp"

using namespace boost::asio;

class RawBridge : public Bridge {

private:
  int closed;
  boost::shared_ptr<ip::tcp::socket> clientSocket;
  ip::tcp::socket serverSocket;
  ip::tcp::endpoint destination;

  boost::asio::io_service &io_service;

  RawBridge(boost::shared_ptr<ip::tcp::socket> clientSocket,
	    ip::tcp::endpoint& destination,
	    boost::asio::io_service & io_service) :
    clientSocket(clientSocket), serverSocket(io_service), 
    io_service(io_service), destination(destination), closed(0)
  {}

  void handleConnect(Bridge::ptr bridge, const boost::system::error_code &error) {
    if (!error) Bridge::shuttle(&(*clientSocket), &serverSocket);
    else        close();
  }

protected:
  virtual void processClientInput(char *buffer, int length) {}

public:

  static ptr create(boost::shared_ptr<ip::tcp::socket> clientSocket,
		    ip::tcp::endpoint& destination,
		    boost::asio::io_service & io_service)

  {
    return ptr(new RawBridge(clientSocket, destination, io_service));    
  }

  virtual ip::tcp::socket& getClientSocket() {
    return *clientSocket;
  }

  virtual ip::tcp::socket& getServerSocket() {
    return serverSocket;
  }

  virtual void shuttle() {
    serverSocket.async_connect(destination, boost::bind(&RawBridge::handleConnect,
							this,
							Bridge::getSmartPointer(), 
							placeholders::error));      
  }

  virtual void close() {
    if (!closed) closed = 1;
    
    if (clientSocket->is_open()) {
      clientSocket->cancel();
      clientSocket->close();
    }
    
    if (serverSocket.is_open()) {
      serverSocket.cancel();
      serverSocket.close();
    }
  }
};



#endif
