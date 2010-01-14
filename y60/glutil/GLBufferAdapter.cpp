/* __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
//
// Copyright (C) 1993-2008, ART+COM AG Berlin, Germany <www.artcom.de>
//
// These coded instructions, statements, and computer programs contain
// proprietary information of ART+COM AG Berlin, and are copy protected
// by law. They may be used, modified and redistributed under the terms
// of GNU General Public License referenced below.
//
// Alternative licensing without the obligations of the GPL is
// available upon request.
//
// GPL v3 Licensing:
//
// This file is part of the ART+COM Y60 Platform.
//
// ART+COM Y60 is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// ART+COM Y60 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with ART+COM Y60.  If not, see <http://www.gnu.org/licenses/>.
// __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
//
// Description: TODO
//
// Last Review: NEVER, NOONE
//
//  review status report: (perfect, ok, fair, poor, disaster, notapplicable, unknown)
//    usefullness            : unknown
//    formatting             : unknown
//    documentation          : unknown
//    test coverage          : unknown
//    names                  : unknown
//    style guide conformance: unknown
//    technical soundness    : unknown
//    dead code              : unknown
//    readability            : unknown
//    understandabilty       : unknown
//    interfaces             : unknown
//    confidence             : unknown
//    integration            : unknown
//    dependencies           : unknown
//    cheesyness             : unknown
//
//    overall review status  : unknown
//
//    recommendations:
//       - unknown
// __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
*/
// own header
#include "GLBufferAdapter.h"

#include <iostream>

#ifdef _WIN32
#   include <windows.h>
#endif

#if defined(_MSC_VER)
#   pragma warning (push,1)
#endif //defined(_MSC_VER)
#include <paintlib/plpngenc.h>
#include <paintlib/pljpegenc.h>
#include <paintlib/plbmpenc.h>
#include <paintlib/pltiffenc.h>
#include <paintlib/planybmp.h>
#include <paintlib/Filter/plfilterfliprgb.h>
#if defined(_MSC_VER)
#   pragma warning (pop)
#endif //defined(_MSC_VER)

#include <GL/glew.h>

#include <asl/base/string_functions.h>
#include <y60/image/PixelEncoding.h>

#include "GLUtils.h"
#include "PixelEncodingInfo.h"

using namespace std;
using namespace asl;

namespace y60 {

    BufferAdapter::BufferAdapter (unsigned theWidth, unsigned theHeight, unsigned theComponents) :
                _myWidth(theWidth), _myHeight(theHeight), _myComponents(theComponents) {
        //alloc(_myWidth * _myHeight * _myComponents);
    }
    BufferAdapter::~BufferAdapter() {
    }

    void
    BufferAdapter::alloc(const unsigned myMemorySize) {
        _myData.resize(myMemorySize);
    }

    asl::Block &
    BufferAdapter::getBlock() {
        return _myData;
    }

    unsigned
    BufferAdapter::getWidth() const {
        return _myWidth;
    }

    unsigned
    BufferAdapter::getHeight() const {
        return _myHeight;
    }

    unsigned
    BufferAdapter::getComponents() const {
        return _myComponents;
    }

    unsigned
    BufferAdapter::getBufferFormat(GLSourceBuffer theSourceBuffer) const {
        GLenum myFormat = 0;
        switch (theSourceBuffer) {
            case FRAME_BUFFER:
                switch (getComponents()) {
                    case 1:
                        myFormat = GL_LUMINANCE;
                        break;
                    case 2:
                        myFormat = GL_LUMINANCE_ALPHA;
                        break;
                    case 3:
                        myFormat = GL_RGB;
                        break;
                    case 4:
                        myFormat = GL_RGBA;
                        break;
                    default:
                        throw GLBufferAdapterException(std::string("Unsupported number of components: " ) + asl::as_string(getComponents()), PLUS_FILE_LINE);
                }
                break;
            case DEPTH_BUFFER:
                myFormat = GL_DEPTH_COMPONENT;
                break;
            default:
                throw GLBufferAdapterException(std::string("Unknown Sourcebuffer: " ) + asl::as_string(theSourceBuffer), PLUS_FILE_LINE);
        }
        return myFormat;
    }

    void
    BufferAdapter::performAction(GLSourceBuffer theSourceBuffer) {
        GLenum myFormat = getBufferFormat(theSourceBuffer);
        if (theSourceBuffer == FRAME_BUFFER) {
            glReadBuffer(GL_BACK);
        }

        alloc(_myWidth * _myHeight * _myComponents);
        glReadPixels(0, 0, _myWidth, _myHeight, myFormat, GL_UNSIGNED_BYTE, _myData.begin());
    }

    // ----------------------------------------------------------------------------------------
    BufferToFile::BufferToFile(const std::string & theFilename, unsigned theFormat,
            unsigned theWidth, unsigned theHeight, unsigned theComponents) :
        BufferAdapter(theWidth, theHeight, theComponents),
        _myFilename(theFilename), _myFormat(theFormat)
    {
    }

    BufferToFile::~BufferToFile() {
    }

    void
    BufferToFile::performAction(GLSourceBuffer theSourceBuffer)
    {
        BufferAdapter::performAction(theSourceBuffer);

        Path myPath(_myFilename, UTF8);
        PLAnyPicDecoder myDecoder;
        PLAnyBmp myBmp;
        PixelEncoding myEncoding;

        switch(getComponents()) {
          case 1:
              myEncoding = GRAY;
              break;
          case 3:
              myEncoding = RGB;
              break;
          case 4:
          default:
              myEncoding = RGBA;
              break;
        }

        PLPixelFormat pf;
        if (!mapPixelEncodingToFormat(myEncoding, pf)) {
            throw GLBufferAdapterException(std::string("unsupported pixel format"), PLUS_FILE_LINE);
        }
        myBmp.Create( getWidth(), getHeight(), pf,
                      getBlock().begin() + getWidth() * (getHeight()-1) * getComponents(), -1 * getWidth() * getComponents());

        switch(_myFormat) {
            case PL_FT_PNG:
                {
                    PLPNGEncoder myEncoder;
                    myEncoder.MakeFileFromBmp(myPath.toLocale().c_str(), &myBmp);
                }
                break;
            case PL_FT_JPEG:
                {
                    PLJPEGEncoder myEncoder;
                    myBmp.ApplyFilter(PLFilterFlipRGB());
                    myEncoder.MakeFileFromBmp(myPath.toLocale().c_str(), &myBmp);
                }
                break;
            case PL_FT_WINBMP:
                {
                    PLBmpEncoder myEncoder;
                    myBmp.ApplyFilter(PLFilterFlipRGB());
                    myEncoder.MakeFileFromBmp(myPath.toLocale().c_str(), &myBmp);
                }
                break;
            case PL_FT_TIFF:
                {
                    PLTIFFEncoder myEncoder;
                    myBmp.ApplyFilter(PLFilterFlipRGB());
                    myEncoder.MakeFileFromBmp(myPath.toLocale().c_str(), &myBmp);
                }
                break;
             default:
                throw GLBufferAdapterException(std::string("Unknown target image format: " ) + asl::as_string(_myFormat),
                                               PLUS_FILE_LINE);
        }
    }

    // ----------------------------------------------------------------------------------------
    BufferToTexture::BufferToTexture(TexturePtr theTexture, const asl::Vector2i & theOffset, bool theCopyToImageFlag) :
        BufferAdapter(theTexture->get<TextureWidthTag>(), theTexture->get<TextureHeightTag>(), 4),
        _myTexture(theTexture), _myOffset(theOffset), _myCopyToImage(theCopyToImageFlag)
    {
    }

    BufferToTexture::~BufferToTexture() {
    }

    void
    BufferToTexture::performAction(GLSourceBuffer theSourceBuffer)
    {
        if (theSourceBuffer == FRAME_BUFFER) {
            glReadBuffer(GL_BACK);
        }

        unsigned myWidth = getWidth();
        unsigned myHeight = getHeight();
        GLuint myTextureID = _myTexture->getTextureId();
        AC_DEBUG << "BufferToTexture::performAction '" << _myTexture->get<NameTag>() << "' id=" << _myTexture->get<IdTag>() << " offset=" << _myOffset << " " << myWidth << "x" << myHeight << " texId=" << myTextureID;
        if (_myCopyToImage) {

            ImagePtr myImage = _myTexture->getImage();
            if (!myImage) {
                AC_WARNING << "Texture '" << _myTexture->get<NameTag>() << "' id=" << _myTexture->get<IdTag>() << " has no image associated";
            } else {
                AC_DEBUG << "BufferToTexture::performAction copy to image '" << myImage->get<NameTag>() << "' id=" << myImage->get<IdTag>();

                // copy framebuffer to Image raster
                PixelEncodingInfo myPixelEncodingInfo = getDefaultGLTextureParams(myImage->getRasterEncoding());
                myPixelEncodingInfo.internalformat = asGLTextureInternalFormat(_myTexture->getInternalEncoding());

                glReadPixels(_myOffset[0],_myOffset[1], myWidth,myHeight,
                        myPixelEncodingInfo.externalformat, myPixelEncodingInfo.pixeltype,
                        myImage->getRasterPtr()->pixels().begin());
                CHECK_OGL_ERROR;

                //_myTexture->preload();
            }
        } else if (myTextureID > 0) {
            GLenum myTextureTarget = asGLTextureTarget(_myTexture->getType());
            if (myTextureTarget == GL_TEXTURE_2D) {
                AC_DEBUG << "BufferToTexture::performAction copy to texture";

                // copy framebuffer to texture
                glBindTexture(GL_TEXTURE_2D, myTextureID);
                glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0,0, _myOffset[0],_myOffset[1], myWidth,myHeight);
                CHECK_OGL_ERROR;

                // generate mipmap levels
                if (hasCap("GL_GENERATE_MIPMAP") && _myTexture->get<TextureMipmapTag>()) {
                    AC_TRACE << "BufferToTexture::performAction: generating mipmap levels";
                    glGenerateMipmapEXT(GL_TEXTURE_2D);
                    CHECK_OGL_ERROR;
                }

                glBindTexture(GL_TEXTURE_2D, 0);
            } else {
                AC_WARNING << "Copy to texture only supported for 'texture_2d'";
            }
        } else {
            AC_DEBUG << "BufferToTexture::performAction texture '" << _myTexture->get<NameTag>() << "' is not valid";
        }
    }
}
