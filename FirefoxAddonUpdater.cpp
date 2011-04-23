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

#include "FirefoxAddonUpdater.hpp"
#include "UpdateManager.hpp"
#include "Logger.hpp"

std::string FirefoxAddonUpdater::incrementVersion(std::string &version) {
  int dot = version.rfind("."); 

  std::string prefix;
  std::string postfix;
  
  int postfixValue;

  if (dot == std::string::npos) {
    prefix  = "";
    postfix = version;
  } else {
    prefix  = version.substr(0, dot+1);
    postfix = version.substr(dot+1);
  }

  std::istringstream postStream(postfix);
  postStream >> postfixValue;

  postfixValue++;
  
  std::ostringstream incrementedVersion;
  incrementedVersion << prefix << postfixValue;

  return incrementedVersion.str();
}

std::string FirefoxAddonUpdater::truncateVersion(std::string &version) {
  int dot = version.rfind(".");

  if (dot == std::string::npos || dot+1 == version.length()) {
    return version;
  } else {
    return version.substr(0, dot);
  }
}

void FirefoxAddonUpdater::sendMetaUpdateResponse() {
  std::string path       = UpdateManager::getInstance()->getAddonPath();
  std::string sha        = UpdateManager::getInstance()->getAddonHash();

  std::string version    = requestArguments["version"];
  std::string newVersion = incrementVersion(version);

  std::string appVersion = requestArguments["appVersion"];
  std::string minVersion = truncateVersion(appVersion);

  Logger::logAddonUpdate(requestArguments["id"]);
  
  std::ostringstream headerStream;
  std::ostringstream responseStream;

  responseStream << "<?xml version=\"1.0\"?>"
		 << "<RDF:RDF xmlns:RDF=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\" "
		 << "xmlns:em=\"http://www.mozilla.org/2004/em-rdf#\">"
		 << "<RDF:Description about=\"urn:mozilla:extension:" << requestArguments["id"] << "\">"
		 << "<em:updates><RDF:Seq><RDF:li resource=\"urn:mozilla:extension:" << requestArguments["id"] << ":" 
		 << newVersion << "\"/>"
		 << "</RDF:Seq></em:updates></RDF:Description>"
		 << "<RDF:Description about=\"urn:mozilla:extension:" << requestArguments["id"] << ":" << newVersion << "\">"
		 << "<em:version>" << newVersion << "</em:version>"
		 << "<em:targetApplication><RDF:Description><em:id>" << requestArguments["appID"] << "</em:id>"
		 << "<em:minVersion>" << minVersion << "</em:minVersion>"
		 << "<em:maxVersion>" << requestArguments["maxAppVersion"] << "</em:maxVersion>"
		 << "<em:updateLink>" << path << "</em:updateLink>"
		 << "<em:updateInfoURL>http://www.thoughtcrime.org</em:updateInfoURL>"
		 << "<em:updateHash>sha256:" << sha << "</em:updateHash>"
		 << "</RDF:Description></em:targetApplication></RDF:Description></RDF:RDF>";

  std::string response = responseStream.str();

  headerStream << "HTTP/1.0 200 OK\r\n"
	       << "Server: Apache/2.0\r\n"
	       << "Accept-Ranges: bytes\r\n"
	       << "Content-Length: "  << response.length() << "\r\n"
	       << "Connection: close\r\n"
	       << "Content-Type: text/plain; charset=iso-8859-1\r\n\r\n";
  
  std::string header = headerStream.str();

  SSL_write(clientSession, header.c_str(), header.length());  
  SSL_write(clientSession, response.c_str(), response.length());
}

bool FirefoxAddonUpdater::parseArguments(std::string &request, std::vector<std::string> &arguments) {
  int questionIndex = request.find('?');

  if (questionIndex == std::string::npos)
    return false;

  if (questionIndex+1 == request.length())
    return false;

  std::string argumentString = request.substr(questionIndex+1);
  std::string delimiters("&");

  Util::tokenizeString(argumentString, delimiters, arguments);
  
  return true;
}

bool FirefoxAddonUpdater::parseMetaRequest(std::string &request) {
  std::vector<std::string> arguments;

  if (!parseArguments(request, arguments))
    return false;

  if (arguments.size() < 10) 
    return false;

  std::vector<std::string>::iterator iter;

  for (iter = arguments.begin(); iter != arguments.end(); iter++) {
    std::string argument = *iter;
    
    int equalsIndex = argument.find('=');

    if (equalsIndex   == std::string::npos ||
	equalsIndex+1 == argument.length())
      continue;

    std::string key   = argument.substr(0, equalsIndex);
    std::string value = argument.substr(equalsIndex+1);

    requestArguments[key] = value;
  }  

  return true;
}
