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

#ifndef __CERTIFICATE_H__
#define __CERTIFICATE_H__

#include <openssl/pem.h>
#include <openssl/conf.h>
#include <openssl/x509v3.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#include <boost/asio.hpp>

#include <iostream>
#include <string>

#include "../util/Util.hpp"
#include "../Logger.hpp"

class BadCertificateException : public std::exception {
public:
  virtual const char* what() const throw() {
    return "Could not parse certificate...";
  }
};

class Certificate {

private:
  X509 *cert;
  EVP_PKEY *key;
  std::string name;

  std::list<std::string> ocspHosts;
  std::list<boost::asio::ip::address> ips;
  std::list<boost::asio::ip::address> ocspIps;

  std::string parseNameFromOCSPUrl(std::string &url) {
    int forwardSlash = url.find('/', 7);

    if (forwardSlash) forwardSlash -= 7;
    else              forwardSlash  = std::string::npos;

    std::string name = url.substr(7, forwardSlash);	
    
    return name;
  }

  void parseOCSPUrls(X509 *cert) {
    int crit = 0;
    int idx  = 0;

    STACK_OF(ACCESS_DESCRIPTION) *ads = 
      (STACK_OF(ACCESS_DESCRIPTION)*)X509_get_ext_d2i(cert, NID_info_access, &crit, &idx); 

    int adsnum = sk_ACCESS_DESCRIPTION_num(ads);

    for(int i=0; i<adsnum; i++) { 
      ACCESS_DESCRIPTION *ad = sk_ACCESS_DESCRIPTION_value(ads, i); 
      
      if (!ad) continue;
      
      if(OBJ_obj2nid(ad->method) == NID_ad_OCSP) { 
	unsigned char * data = ASN1_STRING_data(ad->location->d.ia5);

	std::string url(strdup((const char*)data));
	std::string name = parseNameFromOCSPUrl(url);

	this->ocspHosts.push_back(name);

	Logger::logInit("Added OCSP URL: " + name);
      } 
    }    
  }

  void parseCommonName(X509 *cert) {
    std::string distinguishedName(cert->name);
    std::string::size_type cnIndex = distinguishedName.find("CN=");

    if (cnIndex == std::string::npos) throw BadCertificateException();

    std::string commonName           = distinguishedName.substr(cnIndex+3);    
    std::string::size_type nullIndex = commonName.find("\\x00");
    
    if (nullIndex != std::string::npos) this->name = commonName.substr(0, nullIndex);
    else                                this->name = commonName;
  }

public:
  Certificate() {}

  Certificate(X509 *cert, EVP_PKEY* key, bool resolve) {
    this->cert = cert;
    this->key  = key;
    
    parseCommonName(cert);
    parseOCSPUrls(cert);
    
    if (resolve) {

      if (!isWildcard())
	Util::resolveName(name, ips);
      
      std::list<std::string>::iterator i   = ocspHosts.begin();
      std::list<std::string>::iterator end = ocspHosts.end();
      
      for ( ; i != end ; i++) {
	Util::resolveName(*i, ocspIps);
      }
    }

    Logger::logInit("Certificate Ready: " + this->name);
  }

  bool isOCSPAddress(boost::asio::ip::address &address) {
    std::list<boost::asio::ip::address>::iterator i   = ocspIps.begin();
    std::list<boost::asio::ip::address>::iterator end = ocspIps.end();

    for ( ; i != end ; i++) {
      if ((*i) == address) return true;
    }

    return false;    
  }

  bool isValidTarget(boost::asio::ip::address &address, bool wildcardOK) {
    if (wildcardOK && isWildcard()) {
      return true;
    }

    std::list<boost::asio::ip::address>::iterator i   = ips.begin();
    std::list<boost::asio::ip::address>::iterator end = ips.end();

    for ( ; i != end ; i++) {
      if ((*i) == address) return true;
    }

    return false;
  }

  X509* getCert() {
    return cert;
  }

  EVP_PKEY* getKey() {
    return key;
  }

  void setCert(X509 *cert) {
    this->cert = cert;
  }

  void setKey(EVP_PKEY *key) {
    this->key = key;
  }

  std::string& getName() {
    return name;
  }

  bool isWildcard() {
    return name == "*";
  }
};


#endif
