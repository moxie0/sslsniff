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

#ifndef __LOGGER_H__
#define __LOGGER_H__

#include "http/HttpHeaders.hpp"
#include <string>

class Logger {

public:
  static void initialize(std::string &path, bool postOnly);
  static void logFromServer(std::string &name, char *buf, int len);
  static void logFromClient(std::string &name, char* buf, int len);
  static void logFromClient(std::string &name, HttpHeaders &headers);
  static void logError(std::string error);
  static void logUpdateRequest(std::string &product, std::string &version, 
			      std::string &buildId, std::string &buildTarget,
			      std::string &locale, std::string &channel,
			      std::string &filename);
  static void logAddonUpdate(std::string &appId);
  static void logInit(std::string message);
};

#endif
