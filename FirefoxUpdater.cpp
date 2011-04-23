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

#include "FirefoxUpdater.hpp"
#include "UpdateManager.hpp"
#include "Logger.hpp"
#include <iostream>
#include <fstream>

#define BUILD_ID       "2010011112"
#define EMPTY_RESPONSE "HTTP/1.0 200 OK\r\nServer: Apache/2.0\r\nAccept-Ranges: bytes\r\nContent-Length: 42\r\nConnection: close\r\nContent-Type: text/plain; charset=iso-8859-1\r\n\r\n<?xml version=\"1.0\"?><updates></updates>"

using namespace boost::asio;


ip::tcp::endpoint FirefoxUpdater::getRemoteEndpoint() {
  return destination;
}

bool FirefoxUpdater::parseMetaRequest(std::string &request) {
  std::vector<std::string> tokens;
  std::string delimiters("/");
  Util::tokenizeString(request, delimiters, tokens);
  
  if (tokens.size() < 9) return false;
  
  this->product     = tokens.at(2);
  this->version     = tokens.at(3);
  this->buildId     = tokens.at(4);
  this->buildTarget = tokens.at(5);
  this->locale      = tokens.at(6);
  this->channel     = tokens.at(7);
  this->filename    = tokens.at(8);

  Logger::logUpdateRequest(product, version, buildId, buildTarget,
			   locale, channel, filename);

  return true;
}

void FirefoxUpdater::sendMetaUpdateResponse() {
  if (UpdateManager::getInstance()->isUpdatableRequest(buildId, buildTarget)) {
    std::string path;
    std::string line;

    UpdateManager::getInstance()->getUpdatePath(buildTarget, path);
    std::ifstream xmlDescriptor(path.c_str(), std::ios::in);

    if( !xmlDescriptor )
      throw FirefoxUpdateException();
    
    int size           = boost::filesystem::file_size(path);
    std::ostringstream headerStream;
    headerStream << "HTTP/1.0 200 OK\r\n"
		 << "Server: Apache/2.0\r\n"
		 << "Accept-Ranges: bytes\r\n"
		 << "Content-Length: "  << size << "\r\n"
		 << "Connection: close\r\n"
		 << "Content-Type: text/plain; charset=iso-8859-1\r\n\r\n";

    std::string header = headerStream.str();

    SSL_write(clientSession, header.c_str(), header.length());

    while(getline(xmlDescriptor,line))
      SSL_write(clientSession, line.c_str(), line.length());
    
    return;
  }
  
  std::string emptyResponse(EMPTY_RESPONSE);
  SSL_write(clientSession, emptyResponse.c_str(), emptyResponse.size());  
}

void FirefoxUpdater::readMetaUpdateRequest() {
  char buf[4096];
  int bytesRead;

  while ((bytesRead = SSL_read(clientSession, buf, sizeof(buf))) >= 0) {
    if (headers.process(buf, bytesRead)) {
      std::string &request = headers.getRequest();

      if (parseMetaRequest(request)) return;
      else                           throw FirefoxUpdateException();
    }
  }
}

void FirefoxUpdater::close() {
  if (closed) return;
  else        closed = true;

  SSL_free(clientSession);
  clientSocket->close();
}
