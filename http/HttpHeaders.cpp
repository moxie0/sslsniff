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

#include "HttpHeaders.hpp"
#include "../util/Util.hpp"
#include "../Logger.hpp"


std::map<std::string, std::string>& HttpHeaders::getHeaders() {
  return headers;
}

std::string& HttpHeaders::getHeader(std::string &header) {
  return headers[header];
}

bool HttpHeaders::isPost() {
  return method == std::string("POST");
}

std::string& HttpHeaders::getMethod() {
  return method;
}

std::string& HttpHeaders::getRequest() {
  return request;
}

std::string& HttpHeaders::getPostData() {
  return postData;
}

int HttpHeaders::readLine(char *buffer, int *offset, int length) {
  int i;

  for (i=*offset;i<length;i++) {
    if      (buffer[i] == '\n')  foundLf = 1;
    else if (buffer[i] == '\r') {foundCr = 1; foundLf = 0;}
    else                        {foundCr = 0; foundLf = 0;}

    if (foundCr && foundLf) {
      foundCr = 0;
      foundLf = 0;
      *offset = i;
      return 1;
    }
  }

  *offset = i;
  return 0;
}

void HttpHeaders::parseAction() {
  Util::trimString(action);
  size_t methodEnd  = action.find(" ");
  size_t requestEnd = action.find_last_of(" ");
  
  if (methodEnd == std::string::npos || requestEnd == std::string::npos)
    throw HttpHeaderException();
  
  method  = action.substr(0, methodEnd);
  request = action.substr(methodEnd+1, requestEnd - methodEnd);
  
  Util::trimString(method);
  Util::trimString(request);

//   std::cerr << "Read Action: " << action << std::endl;
//   std::cerr << "Method: " << method << std::endl;
//   std::cerr << "Request: " << request << std::endl;
}

int HttpHeaders::readAction(char *buffer, int length) {
  int offset   = 0;
  int complete = readLine(buffer, &offset, length);

  action.append(buffer, offset+1);

  if (complete) {
    parseAction();
    this->state = READING_KEY;    
  }

  return offset + 1;
}

int HttpHeaders::readValue(char *buffer, int offset, int length) {
  int eolOffset = offset;
  int complete  = readLine(buffer, &eolOffset, length);

  this->value.append(buffer+offset, (eolOffset - offset));

  if (complete) {
    this->state        = READING_KEY;
    this->headers[key] = value;
    
//     std::cerr << "Found value: " << this->value << std::endl;

    this->key.clear();
    this->value.clear();
  }

  return eolOffset + 1;
}

int HttpHeaders::readKey(char *buffer, int offset, int length) {
  int i;

  if (offset < length && buffer[offset] == '\r') {
    if (isPost()) this->state = READING_POST;
    else          this->state = READING_DONE;
    return offset + 2;
  }

  for (i=offset;i<length;i++) {
    if (buffer[i] == ':') {
      this->state = READING_VALUE;
      break;
    }
  }
  
  this->key.append(buffer+offset, (i-offset));

//   if (this->state == READING_VALUE)
//     std::cerr << "Found Key: " << this->key << " at " << i << std::endl;

  return i+1;
}

int HttpHeaders::getContentLength() {
  std::string contentKey       = "Content-Length";
  std::string contentLengthStr = getHeader(contentKey);
  int contentLength;

  if (!Util::fromString<int>(contentLength, contentLengthStr, std::dec)) {
    Logger::logError("Could not read POST data without Content-Length...");
    return 0;
  }

  return contentLength;
}

void HttpHeaders::readPostData(char *buffer, int offset, int length) {
  int contentLength = getContentLength();
  int contentLeft   = contentLength - postData.length();
  int dataLeft      = length - offset;
  int copyLength    = (dataLeft < contentLeft) ? dataLeft : contentLeft;

  postData.append(buffer, offset, copyLength);

  if (postData.length() >= contentLength) {
    state = READING_DONE;
//     std::cerr << "*** Got POST data: " << postData << std::endl;
  }
}

bool HttpHeaders::process(char *buffer, int length) {
  int offset = 0;

  while (offset < length) {
    switch(state) {
    case READING_ACTION: offset = readAction(buffer, length);        break;
    case READING_KEY:    offset = readKey(buffer, offset, length);   break;
    case READING_VALUE:  offset = readValue(buffer, offset, length); break;
    case READING_POST:   readPostData(buffer, offset, length);       break;
    case READING_DONE:   return true;
    }
  }

  if (state == READING_DONE) return true;
  else                       return false;
}
