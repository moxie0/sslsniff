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

#include "SessionCache.hpp"
#include "Logger.hpp"

SessionCache* SessionCache::sessionCache = NULL;
boost::mutex  SessionCache::singletonLock;

SessionCache* SessionCache::getInstance() {
  boost::mutex::scoped_lock lock(singletonLock);

  if (sessionCache == NULL)
    sessionCache = new SessionCache();

  return sessionCache;
}

SessionCache::SessionCache() {
  bzero((void*)&cache, sizeof(SessionCacheBlock));
}

void SessionCache::removeSessionId(unsigned char* id, int idLength) {
  int i;

  for (i=0;i<CACHE_SIZE;i++) {
    if (memcmp(cache.sessions[i].id, id, idLength) == 0) {
      bzero(cache.sessions[i].id, idLength);
    }
  }
}

int SessionCache::setNewSessionId(SSL *s, SSL_SESSION *session) {
  return setNewSessionId(s, session, session->session_id, session->session_id_length);
}

int SessionCache::setNewSessionId(SSL *s, SSL_SESSION *session, 
				  unsigned char *id, int idLength) 
{
  int encodedLength = i2d_SSL_SESSION(session, NULL);

  unsigned char* b;
  int current;

  if (encodedLength > MAX_ENCODING_SIZE) {
    std::stringstream errorStream;
    errorStream << "Encoded Length: " << encodedLength << " too big for session cache, skipping...";
    std::string error = errorStream.str();

    Logger::logError(error);

    return 1;
  }

  if (idLength > MAX_ID_SIZE) {
    std::stringstream errorStream;
    errorStream << "ID Length: " << idLength << " too big for session cache, skipping...";
    std::string error = errorStream.str();

    Logger::logError(error);

    return 1;
  }

  boost::mutex::scoped_lock lock(cacheLock);
  removeSessionId(id, idLength);

  current = cache.current;
  b       = cache.sessions[current].encoding;

  i2d_SSL_SESSION(session, &b);

  memcpy(cache.sessions[current].id, id, idLength);
  cache.sessions[current].encodingLength = encodedLength;
  cache.sessions[current].idLength       = idLength;
  cache.current                          = (current + 1) % CACHE_SIZE;

  return 1;  
}

SSL_SESSION * SessionCache::getSessionId(SSL *s, unsigned char *id, int idLength, int *ref) {
  int i;
  unsigned char *b;

  *ref = 0;

  boost::mutex::scoped_lock lock(cacheLock);

  for (i=0;i<CACHE_SIZE;i++) {
    if (memcmp(cache.sessions[i].id, id, idLength) == 0) {
      b = (unsigned char*)malloc(cache.sessions[i].encodingLength);
      memcpy(b, cache.sessions[i].encoding, cache.sessions[i].encodingLength);
      return d2i_SSL_SESSION(NULL, (const unsigned char **)&b, 
			     cache.sessions[i].encodingLength);
    }
  }

  return NULL;  
}


// Trampoline Functions.  Yay C.

SSL_SESSION * SessionCache::getSessionIdTramp(SSL *s, unsigned char *id, int idLength, int *ref) {
  return SessionCache::getInstance()->getSessionId(s, id, idLength, ref);
}

int SessionCache::setNewSessionIdTramp(SSL *s, SSL_SESSION *session) {
  return SessionCache::getInstance()->setNewSessionId(s, session);
}
