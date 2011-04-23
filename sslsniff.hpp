#ifndef __SSLSNIFF_HPP__
#define __SSLSNIFF_HPP__

#include <openssl/ssl.h>

typedef struct {
  std::string updateLocation;
  std::string addonLocation;
  std::string addonHash;
  std::string chainLocation;
  std::string certificateLocation;
  std::string logLocation;
  std::string fingerprintList;
  bool denyOCSP;
  bool postOnly;
  bool targetedMode;
  int sslListenPort;
  int httpListenPort;
} Options;

#endif
