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

#ifndef __OCSP_DENIER_H__
#define __OCSP_DENIER_H__

#include <string>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

#include "HttpHeaders.hpp"

using namespace boost::asio;

class OCSPDenier {

private:
  static OCSPDenier *ocspDenier;
  static const char *ocspResponse;

  OCSPDenier() {}  
  void readComplete(boost::shared_ptr<ip::tcp::socket> socket,
		    boost::shared_ptr<unsigned char> buffer,
		    boost::shared_ptr<HttpHeaders> headers,
		    const boost::system::error_code &error,
		    size_t bytesRead);

  void writeComplete(boost::shared_ptr<ip::tcp::socket> socket, 
		     const boost::system::error_code &error);

public:
  static OCSPDenier* getInstance();
  void denyOCSPRequest(boost::shared_ptr<ip::tcp::socket> socket);
};


#endif
