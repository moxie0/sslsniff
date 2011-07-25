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

#include "SSLBridge.hpp"
#include <string.h>

using namespace boost::asio;

X509* SSLBridge::getServerCertificate() {
  return SSL_get_peer_certificate(serverSession);
}

void SSLBridge::buildClientContext(SSL_CTX *context, Certificate *leaf, std::list<Certificate*> *chain) {

  SSL_CTX_sess_set_new_cb(context, &SessionCache::setNewSessionIdTramp);
  SSL_CTX_sess_set_get_cb(context, &SessionCache::getSessionIdTramp);

  SSL_CTX_use_certificate(context, leaf->getCert());
  SSL_CTX_use_PrivateKey(context, leaf->getKey());

  if (SSL_CTX_check_private_key(context) == 0) {
    std::cerr << "*** Assertion Failed - Generated PrivateKey Doesn't Work." << std::endl;
    throw SSLConnectionError();
  }

  std::list<Certificate*>::iterator i   = chain->begin();
  std::list<Certificate*>::iterator end = chain->end();

  for (;i != end; i++) {
    SSL_CTX_add_extra_chain_cert(context, (*i)->getCert());
  }

  // if (chain != NULL)
  //   SSL_CTX_add_extra_chain_cert(context, chain->getCert());

  SSL_CTX_set_mode(context, SSL_MODE_AUTO_RETRY);
}

ip::tcp::endpoint SSLBridge::getRemoteEndpoint() {
  return serverSocket->remote_endpoint();
}

void SSLBridge::setServerName() {
  X509 *serverCertificate    = getServerCertificate();
  X509_NAME *serverNameField = X509_get_subject_name(serverCertificate);
  char *serverNameStr        = X509_NAME_oneline(serverNameField, NULL, 0);

  this->serverName = std::string((const char*)serverNameStr);
  int commonNameIndex;

  if ((commonNameIndex = this->serverName.find("CN=")) != std::string::npos)
    this->serverName = this->serverName.substr(commonNameIndex+3);
  
  free(serverNameStr);
}

void SSLBridge::handshakeWithClient(CertificateManager &manager, bool wildcardOK) {
  Certificate *leaf;
  std::list<Certificate*> *chain;

  ip::tcp::endpoint endpoint = getRemoteEndpoint();
  manager.getCertificateForTarget(endpoint, wildcardOK, getServerCertificate(), &leaf, &chain);
  
  setServerName();
  
  SSL_CTX *clientContext = SSL_CTX_new(SSLv23_server_method());
  buildClientContext(clientContext, leaf, chain);

  SSL *clientSession = SSL_new(clientContext);
  SSL_set_fd(clientSession, clientSocket->native());

  if (SSL_accept(clientSession) == 0) {
    Logger::logError("SSL Accept Failed!");
    throw SSLConnectionError();
  }

  this->clientSession = clientSession;
}

void SSLBridge::handshakeWithServer() {
  int bogus;

  ip::address_v4 serverAddress = serverSocket->remote_endpoint().address().to_v4();
  SSL_CTX *serverCtx           = SSL_CTX_new(SSLv23_client_method());;
  SSL *serverSession           = SSL_new(serverCtx);;
  SSL_SESSION *sessionId       = cache->getSessionId(serverSession, 
						     serverAddress.to_bytes().data(), 
						     serverAddress.to_bytes().size(),
						     &bogus);

  if (sessionId != NULL) {
    SSL_set_session(serverSession, sessionId);
    SSL_SESSION_free(sessionId);
  }

  SSL_set_connect_state(serverSession);
  SSL_set_fd(serverSession, serverSocket->native());
  SSL_set_options(serverSession, SSL_OP_ALL);
  
  if (SSL_connect(serverSession) < 0) {
    Logger::logError("Error on SSL Connect.");
    throw SSLConnectionError();
  }

  cache->setNewSessionId(serverSession, SSL_get1_session(serverSession), 
			 serverAddress.to_bytes().data(), 
			 serverAddress.to_bytes().size());

  this->serverSession = serverSession;
}

void SSLBridge::shuttleData() {
  struct pollfd fds[2] = {{clientSocket->native(), POLLIN | POLLPRI | POLLHUP | POLLERR, 0},
			  {serverSocket->native(), POLLIN | POLLPRI | POLLHUP | POLLERR, 0}};

  for (;;) {
    if (poll(fds, 2, -1) < 0)        return;
    if (isAvailable(fds[0].revents)) if (!readFromClient()) return;
    if (isAvailable(fds[1].revents)) if (!readFromServer()) return;
    if (isClosed(fds[0].revents))    return;
    if (isClosed(fds[1].revents))    return;
  }

}

int SSLBridge::isAvailable(int revents) {
  return revents & POLLIN || revents & POLLPRI;
}

int SSLBridge::isClosed(int revents) {
  return revents & POLLERR || revents & POLLHUP;
}

bool SSLBridge::readFromClient() {
  char buf[4096];
  int bytesRead;
  int bytesWritten;
  
  do {
    if ((bytesRead = SSL_read(clientSession, buf, sizeof(buf))) <= 0)   
      return SSL_get_error(clientSession, bytesRead) == SSL_ERROR_WANT_READ ? true : false;

    if ((bytesWritten = SSL_write(serverSession, buf, bytesRead)) <= 0) 
      return false; // FIXME

    Logger::logFromClient(serverName, buf, bytesRead);

  } while (SSL_pending(clientSession));

  return true;
}

bool SSLBridge::readFromServer() {
  char buf[4096];
  int bytesRead;
  int bytesWritten;

  do {
    if ((bytesRead    = SSL_read(serverSession, buf, sizeof(buf))) <= 0)       
      return SSL_get_error(serverSession, bytesRead) == SSL_ERROR_WANT_READ ? true : false;

    if ((bytesWritten = SSL_write(clientSession, buf, bytesRead)) < bytesRead) 
      return false; // FIXME

    Logger::logFromServer(serverName, buf, bytesRead);
  } while (SSL_pending(serverSession));

  return true;
}

void SSLBridge::close() {
  if (closed)        return;
  else               closed = true;

  if (serverSession) SSL_free(serverSession);
  if (clientSession) SSL_free(clientSession);
  
  clientSocket->close();
  serverSocket->close();
}

