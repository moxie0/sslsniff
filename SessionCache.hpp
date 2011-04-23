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

#ifndef __SESSION_CACHE_H__
#define __SESSION_CACHE_H__

#include <openssl/pem.h>
#include <openssl/conf.h>
#include <openssl/x509v3.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <boost/thread.hpp>
#include <boost/bind.hpp>

#define CACHE_SIZE 20
#define MAX_ENCODING_SIZE 4096
#define MAX_ID_SIZE 512

typedef struct {
  int idLength;
  unsigned char id[MAX_ID_SIZE];
  int encodingLength;
  unsigned char encoding[MAX_ENCODING_SIZE];
} EncodedSession;

typedef struct {
  int current;
  EncodedSession sessions[CACHE_SIZE];
} SessionCacheBlock;

class SessionCache {

public:
  static SessionCache* getInstance();
  static SSL_SESSION * getSessionIdTramp(SSL *s, unsigned char *id, int idLength, int *ref);
  static int setNewSessionIdTramp(SSL *s, SSL_SESSION *session);

  int setNewSessionId(SSL *s, SSL_SESSION *session);
  int setNewSessionId(SSL *s, SSL_SESSION *session, unsigned char *id, int idLength);
  SSL_SESSION * getSessionId(SSL *s, unsigned char *id, int idLength, int *ref);

private:
  static SessionCache *sessionCache;
  static boost::mutex singletonLock;

  SessionCacheBlock cache;
  boost::mutex cacheLock;

  SessionCache();
  void removeSessionId(unsigned char* id, int idLength);

};


#endif
