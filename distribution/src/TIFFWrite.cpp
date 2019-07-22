/*
 *  TIFFWrite.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 24/08/2005.
 *  Copyright 2005 Bill Sellers. All rights reserved.
 *
 */


#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef BYTE_ORDER_BIG_ENDIAN
#define SHORT(x) (x * 65536) // shift value to leftmost bytes of LONG
#else
#ifdef BYTE_ORDER_LITTLE_ENDIAN
#define SHORT(x) (x)
#else
#error BYTE_ORDER_BIG_ENDIAN or BYTE_ORDER_LITTLE_ENDIAN must be defined
#endif
#endif

#ifdef USE_LIBTIFF
#include "tiffio.h"

#if defined(_WINDOWS) || defined(MSDOS)
#define BINMODE "b"
#else
#define	BINMODE
#endif

#define	streq(a,b)	(strcmp(a,b) == 0)
#define	strneq(a,b,n)	(strncmp(a,b,n) == 0)
#endif

#include "TIFFWrite.h"

TIFFWrite::TIFFWrite()
{
#ifdef USE_LIBTIFF
#ifdef USE_TIFF_LZW
    compression = COMPRESSION_LZW;
#else
    compression = COMPRESSION_PACKBITS;
#endif
    predictor = 0;
    quality = 75;	/* JPEG quality */
    jpegcolormode = JPEGCOLORMODE_RGB;
#endif
}

TIFFWrite::~TIFFWrite()
{
    if (pixels) free(pixels);
}

// intialise the image
// set samplesPerPixel to 1 for grey scale images and 3 for RGB images
void TIFFWrite::initialiseImage(int32_t width, int32_t height,
                                double xDPIIn, double yDPIIn, int32_t samplesPerPixel)
{
    if (samplesPerPixel != 1) samplesPerPixel = 3;

    imageWidth = width;
    imageHeight = height;
    bytePerPixel = samplesPerPixel;
    rowSize = imageWidth * bytePerPixel;
    imageSize = rowSize * imageHeight;
    pixels = (uint8_t *)malloc(imageSize);
    xDPI = xDPIIn;
    yDPI = yDPIIn;

#ifndef USE_LIBTIFF
#ifdef BYTE_ORDER_BIG_ENDIAN
    fileHeader.theHeader.endian = 0x4d4d;
#else
    fileHeader.theHeader.endian = 0x4949;
#endif
    fileHeader.theHeader.magicNumber = 42;
    fileHeader.theHeader.IFDOffset = (ptrdiff_t)(&(fileHeader.nEntries)) - (ptrdiff_t)(&fileHeader);
    fileHeader.nEntries = 12;
    fileHeader.nextIFD = 0;


    fileHeader.entries[0].tag = 0x100; // width
    fileHeader.entries[0].type = 4;
    fileHeader.entries[0].count = 1;
    fileHeader.entries[0].valueOrOffset = imageWidth;

    fileHeader.entries[1].tag = 0x101; // height
    fileHeader.entries[1].type = 4;
    fileHeader.entries[1].count = 1;
    fileHeader.entries[1].valueOrOffset = imageHeight;

    fileHeader.entries[2].tag = 0x102; // bits per sample
    fileHeader.entries[2].type = 3;
    if (samplesPerPixel == 1)
    {
        fileHeader.entries[2].count = 1;
        fileHeader.entries[2].valueOrOffset = SHORT(8);
        fileHeader.pixelsPerSample[0] = fileHeader.pixelsPerSample[1] = fileHeader.pixelsPerSample[2] = 0;
    }
    else
    {
        fileHeader.entries[2].count = 3;
        fileHeader.entries[2].valueOrOffset = (ptrdiff_t)(&(fileHeader.pixelsPerSample[0])) - (ptrdiff_t)(&fileHeader);
        fileHeader.pixelsPerSample[0] = fileHeader.pixelsPerSample[1] = fileHeader.pixelsPerSample[2] = 8;
    }

    fileHeader.entries[3].tag = 0x103; // compression
    fileHeader.entries[3].type = 3;
    fileHeader.entries[3].count = 1;
    fileHeader.entries[3].valueOrOffset = SHORT(1); // none

    fileHeader.entries[4].tag = 0x106; // photometric interpretation
    fileHeader.entries[4].type = 3;
    fileHeader.entries[4].count = 1;
    if (samplesPerPixel == 1)
        fileHeader.entries[4].valueOrOffset = SHORT(1); // black is zero
    else
        fileHeader.entries[4].valueOrOffset = SHORT(2); // RGB

    fileHeader.entries[5].tag = 0x111; // Strip offsets
    fileHeader.entries[5].type = 4;
    fileHeader.entries[5].count = 1;
    fileHeader.entries[5].valueOrOffset = sizeof(fileHeader);

    fileHeader.entries[6].tag = 0x115; // SamplesPerPixel
    fileHeader.entries[6].type = 3;
    fileHeader.entries[6].count = 1;
    fileHeader.entries[6].valueOrOffset = SHORT(samplesPerPixel);

    fileHeader.entries[7].tag = 0x116; // rows per strip
    fileHeader.entries[7].type = 4;
    fileHeader.entries[7].count = 1;
    fileHeader.entries[7].valueOrOffset = imageHeight;   // one strip

    fileHeader.entries[8].tag = 0x117; // strip byte counts
    fileHeader.entries[8].type = 4;
    fileHeader.entries[8].count = 1;
    fileHeader.entries[8].valueOrOffset = imageHeight * imageWidth;

    fileHeader.entries[9].tag = 0x11a; // X resolution
    fileHeader.entries[9].type = 5;
    fileHeader.entries[9].count = 1;
    fileHeader.entries[9].valueOrOffset = (ptrdiff_t)(&(fileHeader.xres[0])) - (ptrdiff_t)(&fileHeader);
    fileHeader.xres[0] = (int32_t)(100 * xDPI);
    fileHeader.xres[1] = 100;

    fileHeader.entries[10].tag = 0x11b; // Y resolution
    fileHeader.entries[10].type = 5;
    fileHeader.entries[10].count = 1;
    fileHeader.entries[10].valueOrOffset = (ptrdiff_t)(&(fileHeader.yres[0])) - (ptrdiff_t)(&fileHeader);
    fileHeader.yres[0] = (int32_t)(100 * yDPI);
    fileHeader.yres[1] = 100;

    fileHeader.entries[11].tag = 0x128; // units
    fileHeader.entries[11].type = 3;
    fileHeader.entries[11].count = 1;
    fileHeader.entries[11].valueOrOffset = SHORT(2); // inches
#endif
}

void TIFFWrite::copyRow(uint32_t row, uint8_t *data)
{
    memcpy(pixels + row * rowSize, data, rowSize);
}

void TIFFWrite::writeToFile(char *filename)
{
#ifndef USE_LIBTIFF
    FILE *out = fopen(filename, "w");

    fwrite(&fileHeader, sizeof(fileHeader), 1,out);
    fwrite(pixels, imageSize, 1, out);

    fclose(out);
#else
    TIFF *out;
    uint16_t photometric;
    tsize_t linebytes;
    unsigned char *buf = NULL;
    uint32_t rowsperstrip = (uint32_t) -1;
    uint32_t row;


    // code taken from ppm2tiff.c
    out = TIFFOpen(filename, "w");
    if (out == NULL)
        return;
    TIFFSetField(out, TIFFTAG_IMAGEWIDTH,  imageWidth);
    TIFFSetField(out, TIFFTAG_IMAGELENGTH, imageHeight);
    TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, bytePerPixel);
    TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8);
    TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    if (bytePerPixel == 1) photometric = PHOTOMETRIC_MINISBLACK;
    else
    {
        photometric = PHOTOMETRIC_RGB;
        if (compression == COMPRESSION_JPEG && jpegcolormode == JPEGCOLORMODE_RGB)
            photometric = PHOTOMETRIC_YCBCR;
    }
    TIFFSetField(out, TIFFTAG_PHOTOMETRIC, photometric);
    TIFFSetField(out, TIFFTAG_COMPRESSION, compression);
    switch (compression) {
        case COMPRESSION_JPEG:
            TIFFSetField(out, TIFFTAG_JPEGQUALITY, quality);
            TIFFSetField(out, TIFFTAG_JPEGCOLORMODE, jpegcolormode);
            break;
        case COMPRESSION_LZW:
        case COMPRESSION_DEFLATE:
            if (predictor != 0)
                TIFFSetField(out, TIFFTAG_PREDICTOR, predictor);
            break;
    }
    linebytes = bytePerPixel * imageWidth;
    if (TIFFScanlineSize(out) > linebytes)
        buf = (unsigned char *)_TIFFmalloc(linebytes);
    else
        buf = (unsigned char *)_TIFFmalloc(TIFFScanlineSize(out));
    TIFFSetField(out, TIFFTAG_ROWSPERSTRIP,
                 TIFFDefaultStripSize(out, rowsperstrip));
    if (xDPI > 0 && yDPI > 0)
    {
        TIFFSetField(out, TIFFTAG_XRESOLUTION, xDPI);
        TIFFSetField(out, TIFFTAG_YRESOLUTION, yDPI);
        TIFFSetField(out, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
    }
    for (row = 0; row < imageHeight; row++)
    {
        if (TIFFWriteScanline(out, pixels + row * rowSize, row, 0) < 0)
            break;
    }
    (void) TIFFClose(out);
    if (buf)
        _TIFFfree(buf);


#endif
}

// set compression options based on string
/*
 " -c jpeg[:opts]  compress output with JPEG encoding",
 " -c lzw[:opts]	compress output with Lempel-Ziv & Welch encoding",
 "               (no longer supported by default due to Unisys patent enforcement)",
 " -c zip[:opts]	compress output with deflate encoding",
 " -c packbits	compress output with packbits encoding",
 " -c none	use no compression algorithm on output",
 "",
 "JPEG options:",
 " #		set compression quality level (0-100, default 75)",
 " r		output color image as RGB rather than YCbCr",
 "LZW and deflate options:",
 " #		set predictor value",
 "For example, -c lzw:2 to get LZW-encoded data with horizontal differencing"
 */

int
TIFFWrite::processCompressOptions(char* opt)
{
#ifdef USE_LIBTIFF
    if (streq(opt, "none"))
        compression = COMPRESSION_NONE;
    else if (streq(opt, "packbits"))
        compression = COMPRESSION_PACKBITS;
    else if (strneq(opt, "jpeg", 4)) {
        char* cp = strchr(opt, ':');
        if (cp && isdigit(cp[1]))
            quality = atoi(cp+1);
        if (cp && strchr(cp, 'r'))
            jpegcolormode = JPEGCOLORMODE_RAW;
        compression = COMPRESSION_JPEG;
    } else if (strneq(opt, "lzw", 3)) {
        char* cp = strchr(opt, ':');
        if (cp)
            predictor = atoi(cp+1);
        compression = COMPRESSION_LZW;
    } else if (strneq(opt, "zip", 3)) {
        char* cp = strchr(opt, ':');
        if (cp)
            predictor = atoi(cp+1);
        compression = COMPRESSION_DEFLATE;
    } else
        return (0);
    return (1);
#else
    return (0);
#endif
}


