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

#ifndef __CERTIFICATE_MANAGER_H__
#define __CERTIFICATE_MANAGER_H__

#include <openssl/pem.h>
#include <openssl/conf.h>
#include <openssl/x509v3.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#include <string>
#include <list>
#include <boost/filesystem.hpp>
#include <boost/asio.hpp>

#include "Certificate.hpp"

using namespace boost::filesystem;

class CertificateManager {

public:
  virtual bool isOCSPAddress(boost::asio::ip::tcp::endpoint &endpoint) = 0;
  virtual bool isValidTarget(boost::asio::ip::tcp::endpoint &endpoint, bool wildcardOK) = 0;
  virtual void getCertificateForTarget(boost::asio::ip::tcp::endpoint &endpoint,
				       bool wildcardOK,
				       X509 *serverCertificate, Certificate **cert,
				       std::list<Certificate*> **chainCerts) = 0;

protected:
  Certificate* readCredentialsFromFile(const path &file, bool resolve);  

private:
  X509* loadCertificateFromFile(const char* file);
  EVP_PKEY* loadKeyFromFile(const char* file);
};


#endif
