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

#include "TargetedCertificateManager.hpp"

#include <openssl/pem.h>
#include <openssl/conf.h>
#include <openssl/x509v3.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#include <iostream>
#include <string>
#include <boost/filesystem.hpp>

using namespace boost::filesystem;

TargetedCertificateManager::TargetedCertificateManager(std::string &directory,
						       std::string &chain) 
{
  path certDir(directory);
  path chainPath(chain);
  
  if (!exists(certDir)) throw NoCertificateDirectoryException();

  if (!chain.empty()) {
    Certificate *chain = readCredentialsFromFile(chainPath, false);
    chainList.push_back(chain);
  }
  
  directory_iterator end_itr; 

  for ( directory_iterator itr( certDir ); itr != end_itr; ++itr ) {
    if ( !is_directory(itr->status()) ) {
      Certificate *target = readCredentialsFromFile(itr->path(), true);

      if (target->isWildcard()) certificates.push_back(target);
      else                      certificates.push_front(target);
    }
  }

  if (certificates.empty()) throw NoCertificateDirectoryException();
}

bool TargetedCertificateManager::isOCSPAddress(boost::asio::ip::tcp::endpoint &endpoint) {
  boost::asio::ip::address address      = endpoint.address();

  std::list<Certificate*>::iterator i   = certificates.begin();
  std::list<Certificate*>::iterator end = certificates.end();

  for ( ; i != end; i++) {
    if ((*i)->isOCSPAddress(address))
      return true;
  }

  return false;  
}

bool TargetedCertificateManager::isValidTarget(boost::asio::ip::tcp::endpoint &endpoint,
					       bool wildcardOK) {

  boost::asio::ip::address address      = endpoint.address();
  std::list<Certificate*>::iterator i   = certificates.begin();
  std::list<Certificate*>::iterator end = certificates.end();

  for ( ; i != end; i++) {
    if ((*i)->isValidTarget(address, wildcardOK)) return true;
  }

  return false;
}


void TargetedCertificateManager::getCertificateForTarget(boost::asio::ip::tcp::endpoint &endpoint,
							 bool wildcardOK,
							 X509 *serverCertificate,
							 Certificate **cert,
							 std::list<Certificate*> **chainList)
{
  boost::asio::ip::address address = endpoint.address();
  *chainList                       = &(this->chainList);
  // *chain                           = this->chain;

  std::list<Certificate*>::iterator i   = certificates.begin();
  std::list<Certificate*>::iterator end = certificates.end();

  for ( ; i != end; i++) {
    if ((*i)->isValidTarget(address, wildcardOK)) {
      *cert = (*i);
      return;
    }
  }

  *cert = NULL;
  return;
}


void TargetedCertificateManager::dump() {
  std::list<Certificate*>::iterator i;

  for(i=certificates.begin(); i != certificates.end(); ++i) 
    std::cout << "Certificate: " << (*i)->getCert()->name << std::endl;

}
