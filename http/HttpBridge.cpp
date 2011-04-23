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

#include "HttpBridge.hpp"

#include <boost/enable_shared_from_this.hpp>
#include <iostream>

using namespace boost::asio;

void HttpBridge::shuttle() {
  this->closed = 0;
  Bridge::shuttle(&(*clientSocket), &serverSocket);
}

void HttpBridge::setFinishedWithHeaders() {
  state = SHUFFLING_DATA;
  std::string userAgent("User-Agent");
  std::string &agentValue = headers.getHeader(userAgent);
  ip::address address = clientSocket->remote_endpoint().address();

  listener->setUserAgentFor(address, agentValue);
}

void HttpBridge::processClientInput(char *buffer, int length) {
  if (state == READING_HEADERS)
    if (headers.process(buffer, length))
      setFinishedWithHeaders();
}

void HttpBridge::close() {
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
