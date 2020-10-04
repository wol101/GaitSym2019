#include "MD5.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

typedef union uwb {
    uint32_t w;
    uint8_t b[4];
} WBunion;

typedef uint32_t Digest[4];

uint32_t f0( uint32_t abcd[] ){
    return ( abcd[1] & abcd[2]) | (~abcd[1] & abcd[3]);}

uint32_t f1( uint32_t abcd[] ){
    return ( abcd[3] & abcd[1]) | (~abcd[3] & abcd[2]);}

uint32_t f2( uint32_t abcd[] ){
    return  abcd[1] ^ abcd[2] ^ abcd[3];}

uint32_t f3( uint32_t abcd[] ){
    return abcd[2] ^ (abcd[1] |~ abcd[3]);}

typedef uint32_t (*DgstFctn)(uint32_t a[]);

uint32_t *calcKs( uint32_t *k)
{
    double s, pwr;
    int i;

    pwr = pow( 2, 32);
    for (i=0; i<64; i++) {
        s = fabs(sin(1+i));
        k[i] = (uint32_t)( s * pwr );
    }
    return k;
}

// ROtate v Left by amt bits
uint32_t rol( uint32_t v, int16_t amt )
{
    uint32_t  msk1 = (1<<amt) -1;
    return ((v>>(32-amt)) & msk1) | ((v<<amt) & ~msk1);
}

uint32_t *md5(const char *msg, int mlen)
{
    static Digest h0 = { 0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476 };
//    static Digest h0 = { 0x01234567, 0x89ABCDEF, 0xFEDCBA98, 0x76543210 };
    static DgstFctn ff[] = { &f0, &f1, &f2, &f3 };
    static int16_t M[] = { 1, 5, 3, 7 };
    static int16_t O[] = { 0, 1, 5, 0 };
    static int16_t rot0[] = { 7,12,17,22};
    static int16_t rot1[] = { 5, 9,14,20};
    static int16_t rot2[] = { 4,11,16,23};
    static int16_t rot3[] = { 6,10,15,21};
    static int16_t *rots[] = {rot0, rot1, rot2, rot3 };
    static uint32_t kspace[64];
    static uint32_t *k;

    static Digest h;
    Digest abcd;
    DgstFctn fctn;
    int16_t m, o, g;
    uint32_t f;
    int16_t *rotn;
    union {
        uint32_t w[16];
        int8_t     b[64];
    }mm;
    int os = 0;
    int grp, grps, q, p;
    uint8_t *msg2;

    if (k==NULL) k= calcKs(kspace);

    for (q=0; q<4; q++) h[q] = h0[q];   // initialize

    {
        grps  = 1 + (mlen+8)/64;
        msg2 = (uint8_t *)malloc( 64*grps);
        memcpy( msg2, msg, mlen);
        msg2[mlen] = (uint8_t)0x80;
        q = mlen + 1;
        while (q < 64*grps){ msg2[q] = 0; q++ ; }
        {
//            uint8_t t;
            WBunion u;
            u.w = 8*mlen;
//            t = u.b[0]; u.b[0] = u.b[3]; u.b[3] = t;
//            t = u.b[1]; u.b[1] = u.b[2]; u.b[2] = t;
            q -= 8;
            memcpy(msg2+q, &u.w, 4 );
        }
    }

    for (grp=0; grp<grps; grp++)
    {
        memcpy( mm.b, msg2+os, 64);
        for(q=0;q<4;q++) abcd[q] = h[q];
        for (p = 0; p<4; p++) {
            fctn = ff[p];
            rotn = rots[p];
            m = M[p]; o= O[p];
            for (q=0; q<16; q++) {
                g = (m*q + o) % 16;
                f = abcd[1] + rol( abcd[0]+ fctn(abcd) + k[q+16*p] + mm.w[g], rotn[q%4]);

                abcd[0] = abcd[3];
                abcd[3] = abcd[2];
                abcd[2] = abcd[1];
                abcd[1] = f;
            }
        }
        for (p=0; p<4; p++)
            h[p] += abcd[p];
        os += 64;
    }

    if( msg2 )
        free( msg2 );

    return h;
}

char *hexDigest(const uint32_t *uPtr)
{
    static char out[33];
    char *ptr = out;
    for (int i = 0; i < 4; i++)
    {
        sprintf(ptr, "%08x", uPtr[i]);
        ptr += 8;
    }
    return out;
}


