#ifndef MD5_H
#define MD5_H

#include <stdint.h>

uint32_t *md5(const char *msg, int mlen); // returns a static uint32_t int[4] containing the hash values
char *hexDigest(const uint32_t *uPtr); // converts an uint32_t int[4] to a 32 byte hex string + zero terminator (33 bytes statically allocated)

#endif // MD5_H
