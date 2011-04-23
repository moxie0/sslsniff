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

#ifndef __BRIDGE_H__
#define __BRIDGE_H__

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

#include "Logger.hpp"

#define BUFFER_SIZE 4096

using namespace boost::asio;

class Bridge : public boost::enable_shared_from_this<Bridge> {
  
 public:
  typedef boost::shared_ptr<Bridge> ptr;

  virtual void close() = 0;
  virtual ip::tcp::socket& getClientSocket() = 0;
  virtual ip::tcp::socket& getServerSocket() = 0;
  virtual void shuttle() = 0;
  
 private:
  ip::tcp::socket *clientSocket;
  ip::tcp::socket *serverSocket;
  
  void read(ip::tcp::socket *socket, char *buffer) {
    socket->async_read_some(boost::asio::buffer(buffer, BUFFER_SIZE),
			    boost::bind(&Bridge::readComplete,
					shared_from_this(), socket, buffer,
					placeholders::error,
					placeholders::bytes_transferred)); 
  }

  void readComplete(ip::tcp::socket *socket, char *buffer, 
		    const boost::system::error_code& error, size_t bytesRead)
  { 
    if (!error) {

      if (socket == clientSocket)
	processClientInput(buffer, bytesRead);
    
      async_write((socket == clientSocket) ? *serverSocket : *clientSocket,
		  boost::asio::buffer(buffer, bytesRead), 
		  boost::bind(&Bridge::writeComplete, shared_from_this(), 
			      socket, buffer, placeholders::error));
      
    } else if (error != error::operation_aborted) {
      std::stringstream errorStream;
      errorStream << "Read error: " << error;
      std::string error = errorStream.str();
      Logger::logError(error);
      close();
    }
  } 

  void writeComplete(ip::tcp::socket *socket, char *buffer, 
		     const boost::system::error_code &error) 
  {
    if      (!error)                                         read(socket, buffer);
    else if (error != error::operation_aborted) close();
  }

 protected:
  char clientBuffer[BUFFER_SIZE];
  char serverBuffer[BUFFER_SIZE];

  virtual void processClientInput(char *buffer, int length) = 0;

  void shuttle(ip::tcp::socket *clientSocket,
	       ip::tcp::socket *serverSocket)
  {
    this->clientSocket = clientSocket;
    this->serverSocket = serverSocket;

    read(clientSocket, clientBuffer);
    read(serverSocket, serverBuffer);
  }

  ptr getSmartPointer() {
    return shared_from_this();
  }
  
};


#endif
