#ifndef MD5_H
#define MD5_H

#include <stdint.h>
#include <string>
#include <vector>

std::vector<uint32_t> md5(const char *msg, int mlen); // returns a uint32_t int[4] containing the hash values
std::string hexDigest(const uint32_t *md5); // converts an uint32_t int[4] to a 32 byte hex string + zero terminator
std::string hexDigest(const std::vector<uint32_t> &md5); // converts an uint32_t int[4] to a 32 byte hex string + zero terminator

#endif // MD5_H
