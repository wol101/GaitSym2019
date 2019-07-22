#ifndef MD5_H
#define MD5_H

unsigned *md5( const char *msg, int mlen); // returns a static unsigned int[4] containing the hash values
char *hexDigest(const unsigned *uPtr); // converts an unsigned int[4] to a 32 byte hex string + zero terminator (33 bytes statically allocated)

#endif // MD5_H
