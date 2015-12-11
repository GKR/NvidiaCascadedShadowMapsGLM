//
// nvImageDDS.cpp - Image support class
//
// The nvImage class implements an interface for a multipurpose image
// object. This class is useful for loading and formating images
// for use as textures. The class supports dds, png, and hdr formats.
//
// This file implements the DDS specific functionality.
//
// Author: Evan Hart
// Email: sdkfeedback@nvidia.com
//
// Copyright (c) NVIDIA Corporation. All rights reserved.
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <cstring>
#include "nvImage.h"


using std::vector;

namespace nv {

//
//  Structure defines and constants from nvdds
//
//////////////////////////////////////////////////////////////////////

// surface description flags
const unsigned long DDSF_CAPS           = 0x00000001l;
const unsigned long DDSF_HEIGHT         = 0x00000002l;
const unsigned long DDSF_WIDTH          = 0x00000004l;
const unsigned long DDSF_PITCH          = 0x00000008l;
const unsigned long DDSF_PIXELFORMAT    = 0x00001000l;
const unsigned long DDSF_MIPMAPCOUNT    = 0x00020000l;
const unsigned long DDSF_LINEARSIZE     = 0x00080000l;
const unsigned long DDSF_DEPTH          = 0x00800000l;

// pixel format flags
const unsigned long DDSF_ALPHAPIXELS    = 0x00000001l;
const unsigned long DDSF_FOURCC         = 0x00000004l;
const unsigned long DDSF_RGB            = 0x00000040l;
const unsigned long DDSF_RGBA           = 0x00000041l;

// dwCaps1 flags
const unsigned long DDSF_COMPLEX         = 0x00000008l;
const unsigned long DDSF_TEXTURE         = 0x00001000l;
const unsigned long DDSF_MIPMAP          = 0x00400000l;

// dwCaps2 flags
const unsigned long DDSF_CUBEMAP         = 0x00000200l;
const unsigned long DDSF_CUBEMAP_POSITIVEX  = 0x00000400l;
const unsigned long DDSF_CUBEMAP_NEGATIVEX  = 0x00000800l;
const unsigned long DDSF_CUBEMAP_POSITIVEY  = 0x00001000l;
const unsigned long DDSF_CUBEMAP_NEGATIVEY  = 0x00002000l;
const unsigned long DDSF_CUBEMAP_POSITIVEZ  = 0x00004000l;
const unsigned long DDSF_CUBEMAP_NEGATIVEZ  = 0x00008000l;
const unsigned long DDSF_CUBEMAP_ALL_FACES  = 0x0000FC00l;
const unsigned long DDSF_VOLUME          = 0x00200000l;

// compressed texture types
const unsigned long FOURCC_UNKNOWN       = 0;

#ifndef MAKEFOURCC
#define MAKEFOURCC(c0,c1,c2,c3) \
	((unsigned long)(unsigned char)(c0)| \
	((unsigned long)(unsigned char)(c1) << 8)| \
	((unsigned long)(unsigned char)(c2) << 16)| \
	((unsigned long)(unsigned char)(c3) << 24))
#endif

const unsigned long FOURCC_R8G8B8        = 20;
const unsigned long FOURCC_A8R8G8B8      = 21;
const unsigned long FOURCC_X8R8G8B8      = 22;
const unsigned long FOURCC_R5G6B5        = 23;
const unsigned long FOURCC_X1R5G5B5      = 24;
const unsigned long FOURCC_A1R5G5B5      = 25;
const unsigned long FOURCC_A4R4G4B4      = 26;
const unsigned long FOURCC_R3G3B2        = 27;
const unsigned long FOURCC_A8            = 28;
const unsigned long FOURCC_A8R3G3B2      = 29;
const unsigned long FOURCC_X4R4G4B4      = 30;
const unsigned long FOURCC_A2B10G10R10   = 31;
const unsigned long FOURCC_A8B8G8R8      = 32;
const unsigned long FOURCC_X8B8G8R8      = 33;
const unsigned long FOURCC_G16R16        = 34;
const unsigned long FOURCC_A2R10G10B10   = 35;
const unsigned long FOURCC_A16B16G16R16  = 36;

const unsigned long FOURCC_L8            = 50;
const unsigned long FOURCC_A8L8          = 51;
const unsigned long FOURCC_A4L4          = 52;
const unsigned long FOURCC_DXT1          = 0x31545844l; //(MAKEFOURCC('D','X','T','1'))
const unsigned long FOURCC_DXT2          = 0x32545844l; //(MAKEFOURCC('D','X','T','1'))
const unsigned long FOURCC_DXT3          = 0x33545844l; //(MAKEFOURCC('D','X','T','3'))
const unsigned long FOURCC_DXT4          = 0x34545844l; //(MAKEFOURCC('D','X','T','3'))
const unsigned long FOURCC_DXT5          = 0x35545844l; //(MAKEFOURCC('D','X','T','5'))
const unsigned long FOURCC_ATI1          = MAKEFOURCC('A','T','I','1');
const unsigned long FOURCC_ATI2          = MAKEFOURCC('A','T','I','2');

const unsigned long FOURCC_D16_LOCKABLE  = 70;
const unsigned long FOURCC_D32           = 71;
const unsigned long FOURCC_D24X8         = 77;
const unsigned long FOURCC_D16           = 80;

const unsigned long FOURCC_D32F_LOCKABLE = 82;

const unsigned long FOURCC_L16           = 81;

// Floating point surface formats

// s10e5 formats (16-bits per channel)
const unsigned long FOURCC_R16F          = 111;
const unsigned long FOURCC_G16R16F       = 112;
const unsigned long FOURCC_A16B16G16R16F = 113;

// IEEE s23e8 formats (32-bits per channel)
const unsigned long FOURCC_R32F          = 114;
const unsigned long FOURCC_G32R32F       = 115;
const unsigned long FOURCC_A32B32G32R32F = 116;

struct DXTColBlock
{
    GLushort col0;
    GLushort col1;

    GLubyte row[4];
};

struct DXT3AlphaBlock
{
    GLushort row[4];
};

struct DXT5AlphaBlock
{
    GLubyte alpha0;
    GLubyte alpha1;
    
    GLubyte row[6];
};

struct DDS_PIXELFORMAT
{
    unsigned long dwSize;
    unsigned long dwFlags;
    unsigned long dwFourCC;
    unsigned long dwRGBBitCount;
    unsigned long dwRBitMask;
    unsigned long dwGBitMask;
    unsigned long dwBBitMask;
    unsigned long dwABitMask;
};

struct DDS_HEADER
{
    unsigned long dwSize;
    unsigned long dwFlags;
    unsigned long dwHeight;
    unsigned long dwWidth;
    unsigned long dwPitchOrLinearSize;
    unsigned long dwDepth;
    unsigned long dwMipMapCount;
    unsigned long dwReserved1[11];
    DDS_PIXELFORMAT ddspf;
    unsigned long dwCaps1;
    unsigned long dwCaps2;
    unsigned long dwReserved2[3];
};

//
//
////////////////////////////////////////////////////////////
bool Image::readDDS( const char *file, Image& i) {

    // open file
    FILE *fp = fopen(file, "rb");
    if (fp == NULL)
        return false;

    // read in file marker, make sure its a DDS file
    char filecode[4];
    fread(filecode, 1, 4, fp);
    if (strncmp(filecode, "DDS ", 4) != 0)
    {
        fclose(fp);
        return false;
    }

    // read in DDS header
    DDS_HEADER ddsh;
    fread(&ddsh, sizeof(DDS_HEADER), 1, fp);

    // check if image is a volume texture
    if ((ddsh.dwCaps2 & DDSF_VOLUME) && (ddsh.dwDepth > 0))
        i._depth = ddsh.dwDepth;
    else
        i._depth = 0;

    // There are flags that are supposed to mark these fields as valid, but some dds files don't set them properly
    i._width = ddsh.dwWidth;
    i._height = ddsh.dwHeight;
    
    if (ddsh.dwFlags & DDSF_MIPMAPCOUNT) {
        i._levelCount = ddsh.dwMipMapCount;
    }
    else
        i._levelCount = 1;

    //check cube-map faces
    if ( ddsh.dwCaps2 & DDSF_CUBEMAP) {
        //this is a cubemap, count the faces
        i._faces = 0;
        i._faces += (ddsh.dwCaps2 & DDSF_CUBEMAP_POSITIVEX) ? 1 : 0;
        i._faces += (ddsh.dwCaps2 & DDSF_CUBEMAP_NEGATIVEX) ? 1 : 0;
        i._faces += (ddsh.dwCaps2 & DDSF_CUBEMAP_POSITIVEY) ? 1 : 0;
        i._faces += (ddsh.dwCaps2 & DDSF_CUBEMAP_NEGATIVEY) ? 1 : 0;
        i._faces += (ddsh.dwCaps2 & DDSF_CUBEMAP_POSITIVEZ) ? 1 : 0;
        i._faces += (ddsh.dwCaps2 & DDSF_CUBEMAP_NEGATIVEZ) ? 1 : 0;

        //check for a complete cubemap
        if ( (i._faces != 6) || (i._width != i._height) ) {
            fclose(fp);
            return false;
        }
    }
    else {
        //not a cubemap
        i._faces = 0;
    }

    bool btcCompressed = false;
    int bytesPerElement = 0;

    // figure out what the image format is
    if (ddsh.ddspf.dwFlags & DDSF_FOURCC) 
    {
        switch(ddsh.ddspf.dwFourCC)
        {
            case FOURCC_DXT1:
                i._format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
                i._internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
                i._type = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
                bytesPerElement = 8;
                btcCompressed = true;
                break;

            case FOURCC_DXT2:
            case FOURCC_DXT3:
                i._format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
                i._internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
                i._type = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
                bytesPerElement = 16;
                btcCompressed = true;
                break;

            case FOURCC_DXT4:
            case FOURCC_DXT5:
                i._format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
                i._internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
                i._type = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
                bytesPerElement = 16;
                btcCompressed = true;
                break;

			case FOURCC_ATI1:
                i._format = GL_COMPRESSED_LUMINANCE_LATC1_EXT;
                i._internalFormat = GL_COMPRESSED_LUMINANCE_LATC1_EXT;
                i._type = GL_COMPRESSED_LUMINANCE_LATC1_EXT;
                bytesPerElement = 8;
                btcCompressed = true;
                break;

			case FOURCC_ATI2:
                i._format = GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT;
                i._internalFormat = GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT;
                i._type = GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT;
                bytesPerElement = 16;
                btcCompressed = true;
                break;

            case FOURCC_R8G8B8:
                i._format = GL_BGR;
                i._internalFormat = GL_RGB8;
                i._type = GL_UNSIGNED_BYTE;
                bytesPerElement = 3;
                break;

            case FOURCC_A8R8G8B8:
                i._format = GL_BGRA;
                i._internalFormat = GL_RGBA8;
                i._type = GL_UNSIGNED_BYTE;
                bytesPerElement = 4;
                break;

            case FOURCC_X8R8G8B8:
                i._format = GL_BGRA;
                i._internalFormat = GL_RGB8;
                i._type = GL_UNSIGNED_INT_8_8_8_8;
                bytesPerElement = 4;
                break;

            case FOURCC_R5G6B5:
                i._format = GL_BGR;
                i._internalFormat = GL_RGB5;
                i._type = GL_UNSIGNED_SHORT_5_6_5;
                bytesPerElement = 2;
                break;

            case FOURCC_A8:
                i._format = GL_ALPHA;
                i._internalFormat = GL_ALPHA8;
                i._type = GL_UNSIGNED_BYTE;
                bytesPerElement = 1;
                break;

            case FOURCC_A2B10G10R10:
                i._format = GL_RGBA;
                i._internalFormat = GL_RGB10_A2;
                i._type = GL_UNSIGNED_INT_10_10_10_2;
                bytesPerElement = 4;
                break;

            case FOURCC_A8B8G8R8:
                i._format = GL_RGBA;
                i._internalFormat = GL_RGBA8;
                i._type = GL_UNSIGNED_BYTE;
                bytesPerElement = 4;
                break;

            case FOURCC_X8B8G8R8:
                i._format = GL_RGBA;
                i._internalFormat = GL_RGB8;
                i._type = GL_UNSIGNED_INT_8_8_8_8;
                bytesPerElement = 4;
                break;

            case FOURCC_A2R10G10B10:
                i._format = GL_BGRA;
                i._internalFormat = GL_RGB10_A2;
                i._type = GL_UNSIGNED_INT_10_10_10_2;
                bytesPerElement = 4;
                break;

            case FOURCC_A16B16G16R16:
                i._format = GL_RGBA;
                i._internalFormat = GL_RGBA16;
                i._type = GL_UNSIGNED_SHORT;
                bytesPerElement = 8;
                break;

            case FOURCC_L8:
                i._format = GL_LUMINANCE;
                i._internalFormat = GL_LUMINANCE8;
                i._type = GL_UNSIGNED_BYTE;
                bytesPerElement = 1;
                break;

            case FOURCC_A8L8:
                i._format = GL_LUMINANCE_ALPHA;
                i._internalFormat = GL_LUMINANCE8_ALPHA8;
                i._type = GL_UNSIGNED_BYTE;
                bytesPerElement = 2;
                break;

            case FOURCC_L16:
                i._format = GL_LUMINANCE;
                i._internalFormat = GL_LUMINANCE16;
                i._type = GL_UNSIGNED_SHORT;
                bytesPerElement = 2;
                break;

            case FOURCC_R16F:
                i._format = GL_LUMINANCE; //should use red, once it is available
                i._internalFormat = GL_LUMINANCE16F_ARB; 
                i._type = GL_HALF_FLOAT_ARB;
                bytesPerElement = 2;
                break;

            case FOURCC_A16B16G16R16F:
                i._format = GL_RGBA;
                i._internalFormat = GL_RGBA16F_ARB;
                i._type = GL_HALF_FLOAT_ARB;
                bytesPerElement = 8;
                break;

            case FOURCC_R32F:
                i._format = GL_LUMINANCE; //should use red, once it is available
                i._internalFormat = GL_LUMINANCE32F_ARB; 
                i._type = GL_FLOAT;
                bytesPerElement = 4;
                break;

            case FOURCC_A32B32G32R32F:
                i._format = GL_RGBA;
                i._internalFormat = GL_RGBA32F_ARB;
                i._type = GL_FLOAT;
                bytesPerElement = 16;
                break;

            case FOURCC_UNKNOWN:
            case FOURCC_X1R5G5B5:
            case FOURCC_A1R5G5B5:
            case FOURCC_A4R4G4B4:
            case FOURCC_R3G3B2:
            case FOURCC_A8R3G3B2:
            case FOURCC_X4R4G4B4:
            case FOURCC_A4L4:
            case FOURCC_D16_LOCKABLE:
            case FOURCC_D32:
            case FOURCC_D24X8:
            case FOURCC_D16:
            case FOURCC_D32F_LOCKABLE:
            case FOURCC_G16R16:
            case FOURCC_G16R16F:
            case FOURCC_G32R32F:
                //these are unsupported for now
            default:
                fclose(fp);
                return false;
        }
    }
    else if (ddsh.ddspf.dwFlags == DDSF_RGBA && ddsh.ddspf.dwRGBBitCount == 32)
    {
        i._format = GL_BGRA;
        i._internalFormat = GL_RGBA8;
        i._type = GL_UNSIGNED_BYTE;
        bytesPerElement = 4;
    }
    else if (ddsh.ddspf.dwFlags == DDSF_RGB  && ddsh.ddspf.dwRGBBitCount == 32)
    {
        i._format = GL_BGR;
        i._internalFormat = GL_RGBA8;
        i._type = GL_UNSIGNED_BYTE;
        bytesPerElement = 4;
    }
    else if (ddsh.ddspf.dwFlags == DDSF_RGB  && ddsh.ddspf.dwRGBBitCount == 24)
    {
        i._format = GL_BGR;
        i._internalFormat = GL_RGB8;
        i._type = GL_UNSIGNED_BYTE;
        bytesPerElement = 3;
    }
	else if (ddsh.ddspf.dwRGBBitCount == 8)
	{
		i._format = GL_LUMINANCE; 
        i._internalFormat = GL_LUMINANCE8; 
        i._type = GL_UNSIGNED_BYTE;
		bytesPerElement = 1;
	}
    else 
    {
        fclose(fp);
        return false;
    }

    i._elementSize = bytesPerElement;

    for (int face = 0; face < ((i._faces) ? i._faces : 1); face++) {
        int w = i._width, h = i._height, d = (i._depth) ? i._depth : 1;
        for (int level = 0; level < i._levelCount; level++) {
            int bw = (btcCompressed) ? (w+3)/4 : w;
            int bh = (btcCompressed) ? (h+3)/4 : h;
            int size = bw*bh*d*bytesPerElement;

            GLubyte *data = new GLubyte[size];

            fread( data, size, 1, fp);

            i._data.push_back(data);

            if (i._faces != 6)
                i.flipSurface( data, w, h, d);

            //reduce mip sizes
            w = ( w > 1) ? w >> 1 : 1;
            h = ( h > 1) ? h >> 1 : 1;
            d = ( d > 1) ? d >> 1 : 1;
        }
    }
/*
    //reverse cube map y faces
    if (i._faces == 6) {
        for (int level = 0; level < i._levelCount; level++) {
            GLubyte *temp = i._data[2*i._levelCount + level];
            i._data[2*i._levelCount + level] = i._data[3*i._levelCount + level];
            i._data[3*i._levelCount + level] = temp;
        }
    }
  */  

    fclose(fp);
    return true;
}

//
// flip a DXT1 color block
////////////////////////////////////////////////////////////
void Image::flip_blocks_dxtc1(GLubyte *ptr, unsigned int numBlocks)
{
    DXTColBlock *curblock = (DXTColBlock*)ptr;
    GLubyte temp;

    for (unsigned int i = 0; i < numBlocks; i++) {
        temp = curblock->row[0];
        curblock->row[0] = curblock->row[3];
        curblock->row[3] = temp;
        temp = curblock->row[1];
        curblock->row[1] = curblock->row[2];
        curblock->row[2] = temp;

        curblock++;
    }
}

//
// flip a DXT3 color block
////////////////////////////////////////////////////////////
void Image::flip_blocks_dxtc3(GLubyte *ptr, unsigned int numBlocks)
{
    DXTColBlock *curblock = (DXTColBlock*)ptr;
    DXT3AlphaBlock *alphablock;
    GLushort tempS;
    GLubyte tempB;

    for (unsigned int i = 0; i < numBlocks; i++)
    {
        alphablock = (DXT3AlphaBlock*)curblock;

        tempS = alphablock->row[0];
        alphablock->row[0] = alphablock->row[3];
        alphablock->row[3] = tempS;
        tempS = alphablock->row[1];
        alphablock->row[1] = alphablock->row[2];
        alphablock->row[2] = tempS;

        curblock++;

        tempB = curblock->row[0];
        curblock->row[0] = curblock->row[3];
        curblock->row[3] = tempB;
        tempB = curblock->row[1];
        curblock->row[1] = curblock->row[2];
        curblock->row[2] = tempB;

        curblock++;
    }
}

//
// flip a DXT5 alpha block
////////////////////////////////////////////////////////////
void flip_dxt5_alpha(DXT5AlphaBlock *block)
{
    GLubyte gBits[4][4];
    
    const unsigned long mask = 0x00000007;          // bits = 00 00 01 11
    unsigned long bits = 0;
    memcpy(&bits, &block->row[0], sizeof(unsigned char) * 3);

    gBits[0][0] = (GLubyte)(bits & mask);
    bits >>= 3;
    gBits[0][1] = (GLubyte)(bits & mask);
    bits >>= 3;
    gBits[0][2] = (GLubyte)(bits & mask);
    bits >>= 3;
    gBits[0][3] = (GLubyte)(bits & mask);
    bits >>= 3;
    gBits[1][0] = (GLubyte)(bits & mask);
    bits >>= 3;
    gBits[1][1] = (GLubyte)(bits & mask);
    bits >>= 3;
    gBits[1][2] = (GLubyte)(bits & mask);
    bits >>= 3;
    gBits[1][3] = (GLubyte)(bits & mask);

    bits = 0;
    memcpy(&bits, &block->row[3], sizeof(GLubyte) * 3);

    gBits[2][0] = (GLubyte)(bits & mask);
    bits >>= 3;
    gBits[2][1] = (GLubyte)(bits & mask);
    bits >>= 3;
    gBits[2][2] = (GLubyte)(bits & mask);
    bits >>= 3;
    gBits[2][3] = (GLubyte)(bits & mask);
    bits >>= 3;
    gBits[3][0] = (GLubyte)(bits & mask);
    bits >>= 3;
    gBits[3][1] = (GLubyte)(bits & mask);
    bits >>= 3;
    gBits[3][2] = (GLubyte)(bits & mask);
    bits >>= 3;
    gBits[3][3] = (GLubyte)(bits & mask);

    // clear existing alpha bits
    memset(block->row, 0, sizeof(GLubyte) * 6);

    unsigned long *pBits = ((unsigned long*) &(block->row[0]));

    *pBits = *pBits | (gBits[3][0] << 0);
    *pBits = *pBits | (gBits[3][1] << 3);
    *pBits = *pBits | (gBits[3][2] << 6);
    *pBits = *pBits | (gBits[3][3] << 9);

    *pBits = *pBits | (gBits[2][0] << 12);
    *pBits = *pBits | (gBits[2][1] << 15);
    *pBits = *pBits | (gBits[2][2] << 18);
    *pBits = *pBits | (gBits[2][3] << 21);

    pBits = ((unsigned long*) &(block->row[3]));

    *pBits = *pBits | (gBits[1][0] << 0);
    *pBits = *pBits | (gBits[1][1] << 3);
    *pBits = *pBits | (gBits[1][2] << 6);
    *pBits = *pBits | (gBits[1][3] << 9);

    *pBits = *pBits | (gBits[0][0] << 12);
    *pBits = *pBits | (gBits[0][1] << 15);
    *pBits = *pBits | (gBits[0][2] << 18);
    *pBits = *pBits | (gBits[0][3] << 21);
}

//
// flip a DXT5 color block
////////////////////////////////////////////////////////////
void Image::flip_blocks_dxtc5(GLubyte *ptr, unsigned int numBlocks)
{
    DXTColBlock *curblock = (DXTColBlock*)ptr;
    DXT5AlphaBlock *alphablock;
    GLubyte temp;
    
    for (unsigned int i = 0; i < numBlocks; i++)
    {
        alphablock = (DXT5AlphaBlock*)curblock;
        
        flip_dxt5_alpha(alphablock);

        curblock++;

        temp = curblock->row[0];
        curblock->row[0] = curblock->row[3];
        curblock->row[3] = temp;
        temp = curblock->row[1];
        curblock->row[1] = curblock->row[2];
        curblock->row[2] = temp;

        curblock++;
    }
}


};