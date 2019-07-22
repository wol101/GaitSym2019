/*
 *  TIFFWrite.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 24/08/2005.
 *  Copyright 2005 Bill Sellers. All rights reserved.
 *
 */

#ifndef TIFFWrite_h
#define TIFFWrite_h

#include <stdint.h>

struct TIFFWriteTIFFHeader
{
    uint16_t  endian;
    uint16_t  magicNumber;
    uint32_t  IFDOffset;
};

struct TIFFWriteTIFFIFD
{
    uint16_t  tag;
    uint16_t  type;
    uint32_t  count;
    uint32_t  valueOrOffset;
};

struct TIFFWriteTIFFFileHeader
{
    struct TIFFWriteTIFFHeader theHeader;
    uint16_t  pad1; // needed to keep structs on 32 bit boundaries
    uint16_t  nEntries;   // = 12
    struct TIFFWriteTIFFIFD entries[12];
    uint32_t  nextIFD;    // = 0
    uint32_t  xres[2];
    uint32_t  yres[2];
    uint16_t  pixelsPerSample[4];
};

class TIFFWrite
{
public:
    TIFFWrite();
    virtual ~TIFFWrite();

    void initialiseImage(int32_t width, int32_t height,
                         double xDPI, double yDPI, int32_t samplesPerPixel);
    int processCompressOptions(char* opt);
    void copyRow(uint32_t row, uint8_t *data);
    void writeToFile(char *filename);

private:
#ifndef USE_LIBTIFF
        struct 		TIFFWriteTIFFFileHeader fileHeader;
#endif
    uint8_t       *pixels = nullptr;
    uint32_t      imageWidth = 0;
    uint32_t      imageHeight = 0;
    uint32_t      bytePerPixel = 0;
    uint32_t  	rowSize = 0;
    uint32_t  	imageSize = 0;
    double	xDPI = 0;
    double	yDPI = 0;

#ifdef USE_LIBTIFF
    uint16_t compression;
    uint16_t predictor;
    int quality;
    int jpegcolormode;
#endif
};

#endif

