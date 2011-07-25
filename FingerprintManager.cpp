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

#include "FingerprintManager.hpp"
#include "util/Util.hpp"

FingerprintManager* FingerprintManager::manager = new FingerprintManager();

FingerprintManager* FingerprintManager::getInstance() {
  return manager;
}

void FingerprintManager::setValidUserAgents(std::string &validAgents) {
  if (!validAgents.empty()) {
    std::string delimiters(" ,");
    Util::tokenizeString(validAgents, delimiters, this->validAgents);
  }
}

void FingerprintManager::setUserAgentFor(ip::address &address, std::string &userAgent) {
  //  std::cerr << "FM: Got Fingerprint (" << address << "):" << userAgent << std::endl;
  fingerprints[address] = userAgent;  
}

bool FingerprintManager::isValidWildcardTarget(ip::address &address) {
  std::string *userAgent = getUserAgentFor(address);

  return 
    isValidTarget(address) &&
    userAgent != NULL      &&
    !(userAgent->empty())  &&
    userAgent->find("Firefox") != std::string::npos;
}

bool FingerprintManager::isValidTarget(ip::address &address) {
  if (validAgents.size() == 0) return true; // We're not fingerprinting.
  
  std::string* userAgent = getUserAgentFor(address);
  if (userAgent == NULL || userAgent->empty()) return false;

  std::vector<std::string>::iterator iter = validAgents.begin();

  while (iter != validAgents.end()) {
    if      (*iter == "ff" && (userAgent->find("Firefox") != std::string::npos))    return true;
    else if (*iter == "ios" && (userAgent->find("iPhone") != std::string::npos))    return true;
    else if (*iter == "ie" && (userAgent->find("MSIE") != std::string::npos))       return true;
    else if (*iter == "safari" && (userAgent->find("Safari") != std::string::npos)) return true;
    else if (*iter == "opera" && (userAgent->find("Opera") != std::string::npos))   return true;

    iter++;
  }

  return false;
}

std::string* FingerprintManager::getUserAgentFor(ip::address &address) {
  std::map<ip::address,std::string>::iterator iter = fingerprints.find(address);
  if( iter != fingerprints.end() ) return &(iter->second);
  else                             return NULL;
}
