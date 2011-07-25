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

#ifndef __SSL_BRIDGE_H__
#define __SSL_BRIDGE_H__

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

#include <openssl/pem.h>
#include <openssl/conf.h>
#include <openssl/x509v3.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <list>
#include <map>

#include "util/Util.hpp"
#include "certificate/Certificate.hpp"
#include "certificate/CertificateManager.hpp"

#include "SessionCache.hpp"
#include "Logger.hpp"

using namespace boost::asio;

class SSLConnectionError : public std::exception {
public:
  virtual const char* what() const throw() {
    return "Error with SSL connection...";
  }
};

class SSLBridge {

protected:
  boost::shared_ptr<ip::tcp::socket> clientSocket;
  ip::tcp::socket *serverSocket;
  std::string serverName;

  bool closed;

  SSL *serverSession;
  SSL *clientSession;

  virtual ip::tcp::endpoint getRemoteEndpoint();
  virtual bool readFromClient();

private:
  SessionCache *cache;

  X509* getServerCertificate();
  void buildClientContext(SSL_CTX *context, Certificate *leaf, std::list<Certificate*> *chain);
  int isAvailable(int revents);
  int isClosed(int revents);
  int forwardData(SSL *from, SSL *to);
  void setServerName();
  bool readFromServer();

public:

  SSLBridge(boost::shared_ptr<ip::tcp::socket> clientSocket,
	    ip::tcp::socket *serverSocket) 
    : clientSocket(clientSocket), serverSocket(serverSocket),
      serverSession(), clientSession(), closed(false)
  {
    cache = SessionCache::getInstance();
  }

  ~SSLBridge() {
    close();
  }

  void handshakeWithClient(CertificateManager &manager, bool wildcardOK);
  void handshakeWithServer();
  void shuttleData();
  virtual void close();
};



#endif
