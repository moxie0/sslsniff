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


#include <stdlib.h>
#include <string.h>

#include "OCSPDenier.hpp"
#include "../Logger.hpp"

using namespace boost::asio;

OCSPDenier* OCSPDenier::ocspDenier    = new OCSPDenier();
const char* OCSPDenier::ocspResponse  = "HTTP/1.0 200 Ok\r\ncontent-type: application/ocsp-response\r\ncontent-transfer-encoding: binary\r\ncontent-length: 1\r\nconnection: close\r\n\r\n3";

OCSPDenier* OCSPDenier::getInstance() {
  return ocspDenier;
}

void OCSPDenier::denyOCSPRequest(boost::shared_ptr<ip::tcp::socket> socket) {
  boost::shared_ptr<unsigned char> buffer((unsigned char*)malloc(1024), free);
  boost::shared_ptr<HttpHeaders> headers(new HttpHeaders());

  socket->async_read_some(boost::asio::buffer(buffer.get(), 1024),
			  boost::bind(&OCSPDenier::readComplete, this,
				      socket, buffer, headers,
				      placeholders::error,
				      placeholders::bytes_transferred));
}

void OCSPDenier::readComplete(boost::shared_ptr<ip::tcp::socket> socket,
			      boost::shared_ptr<unsigned char> buffer,
			      boost::shared_ptr<HttpHeaders> headers,
			      const boost::system::error_code &error,
			      size_t bytesRead)
{
  if (error) {
    Logger::logError("Error reading OCSP request.");
    return;
  }

  if (headers->process((char*)buffer.get(), bytesRead)) {
    async_write(*socket, boost::asio::buffer(ocspResponse, strlen(ocspResponse)),
		boost::bind(&OCSPDenier::writeComplete, this, socket, placeholders::error));
  }
}

void OCSPDenier::writeComplete(boost::shared_ptr<ip::tcp::socket> socket, 
			       const boost::system::error_code &error)
{
  Logger::logError("OCSP Denial Sent.");
}
