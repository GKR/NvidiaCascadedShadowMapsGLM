//
// nvImageHdr.cpp - Image support class
//
// The nvImage class implements an interface for a multipurpose image
// object. This class is useful for loading and formating images
// for use as textures. The class supports dds, png, and hdr formats.
//
// This file implements the HDR specific functionality.
//
// Author: Evan Hart
// Email: sdkfeedback@nvidia.com
//
// Copyright (c) NVIDIA Corporation. All rights reserved.
////////////////////////////////////////////////////////////////////////////////

#include "rgbe.h"

#include "nvImage.h"

using std::vector;

namespace nv {

//
//  readHdr
//
//    Image loader function for hdr files. 
////////////////////////////////////////////////////////////
bool Image::readHdr( const char *file, Image& i) {
    int width, height;
    FILE *fp = fopen(file, "rb");
    if (!fp) {
        return false;
    }

    rgbe_header_info header;

    if (RGBE_ReadHeader( fp, &width, &height, &header)) {
        fclose(fp);
        return false;
    }

    GLubyte *data = (GLubyte*)new float[width*height*3];
    
    if (!data) {
        fclose(fp);
        return false;
    }

    if (RGBE_ReadPixels_RLE( fp, (float*)data, width, height)) {
        delete []data;
        fclose(fp);
        return false;
    }

    //set all the parameters
    i._width = width;
    i._height = height;
    i._depth = 0;
    i._levelCount = 1;
    i._type = GL_FLOAT;
    i._format = GL_RGB;
    i._internalFormat = GL_RGB32F_ARB;
    i._faces = 0;
    i._elementSize = 12;
    i._data.push_back( data);

    //hdr images come in upside down
    i.flipSurface( data, i._width, i._height, i._depth);

    fclose(fp);

    return true;
}

};
