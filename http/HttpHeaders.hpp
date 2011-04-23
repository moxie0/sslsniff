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

#ifndef __HTTP_HEADERS_H__
#define __HTTP_HEADERS_H__

#define READING_ACTION 0
#define READING_KEY    1
#define READING_VALUE  2
#define READING_POST   3
#define READING_DONE   4

#include <string>
#include <map>
#include <iostream>

class HttpHeaderException : public std::exception {
public:
  virtual const char* what() const throw() {
    return "HTTP Action Malformed...";
  }
};

class HttpHeaders {

private:
  int foundCr;
  int foundLf;

  int state;

  std::string action;
  std::string method;
  std::string request;

  std::string postData;
  std::string key;
  std::string value;
  std::map<std::string, std::string> headers;

  int readLine(char *buffer, int *offset, int length);
  int readAction(char *buffer, int length);
  int readValue(char *buffer, int offset, int length);
  int readKey(char *buffer, int offset, int length);
  void readPostData(char *buffer, int offset, int length);
  void parseAction();
  int getContentLength();

public:
  HttpHeaders()
    : state(READING_ACTION),
      foundCr(0),
      foundLf(0) {}      

  bool process(char * buffer, int length);
  bool isPost();

  std::map<std::string, std::string>& getHeaders();
  std::string& getHeader(std::string& header);
  std::string& getMethod();
  std::string& getRequest();  
  std::string& getPostData();  
};


#endif
