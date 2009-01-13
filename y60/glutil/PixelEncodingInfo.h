//============================================================================
//
// Copyright (C) 2002-2005, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//============================================================================

#ifndef INCL_TEXTURE_COMPRESSION_TYPE
#define INCL_TEXTURE_COMPRESSION_TYPE

#include "y60_glutil_settings.h"

#include <asl/base/string_functions.h>
#include <asl/base/Exception.h>
#include <y60/image/PixelEncoding.h>


#include <string>
#include <stdio.h>
#include <iostream>

#include "GLUtils.h"

namespace y60 {

    DEFINE_EXCEPTION(GLUtilException, asl::Exception);

    struct PixelEncodingInfo {
        PixelEncodingInfo(GLenum theInternalFormat,  float theBytesPerPixel,
                          bool theCompressedFlag, GLenum theExternalFormat, 
                          GLenum thePixelType=GL_UNSIGNED_BYTE) :
            externalformat(theExternalFormat),
            bytesPerPixel(theBytesPerPixel),
            compressedFlag(theCompressedFlag),
            internalformat(theInternalFormat),
            pixeltype(thePixelType)
        {}

        GLenum   externalformat;
        GLenum   internalformat;
        GLenum   pixeltype;
        float    bytesPerPixel;
        bool     compressedFlag;
    };
    Y60_GLUTIL_EXPORT PixelEncodingInfo getDefaultGLTextureParams(PixelEncoding theEncoding);
    Y60_GLUTIL_EXPORT std::string getGLEnumString(GLenum theFormat);
    inline
    std::ostream & operator<<(std::ostream & os, const PixelEncodingInfo & theInfo) {
        os << "externalformat: " << getGLEnumString(theInfo.externalformat);
        os << ", internalformat: " << getGLEnumString(theInfo.internalformat);
        os << ", pixeltype: " << getGLEnumString(theInfo.pixeltype);
        os << ", bytesPerPixel: " << theInfo.bytesPerPixel;
        os << ", compressedFlag: " << theInfo.compressedFlag;
        return os;
    }
 }
#endif
