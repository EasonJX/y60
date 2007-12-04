//============================================================================
// Copyright (C) 2005-2007, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//============================================================================

#include "OffscreenBuffer.h"
#include "GLUtils.h"
#include "PixelEncodingInfo.h"

#include <y60/Image.h>
#include <y60/Texture.h>
#include <asl/Logger.h>
#include <asl/numeric_functions.h>


using namespace std;
using namespace dom;


namespace y60 {

static void checkFramebufferStatus()
{
    GLenum myStatus;
    myStatus = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);

    bool isOK = false;
    ostringstream os;
    switch(myStatus) {                                          
        case GL_FRAMEBUFFER_COMPLETE_EXT: // Everything's OK
            isOK = true;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
            os << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT" << endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
            os << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT" << endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT:
            os << "GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT" << endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
            os << "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT" << endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
            os << "GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT" << endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
            os << "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT" << endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
            os << "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT" << endl;
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
            os << "GL_FRAMEBUFFER_UNSUPPORTED_EXT" << endl;
            break;
        default:
            /* programming error; will fail on all hardware */
            throw OpenGLException("GL_FRAMEBUFFER_EXT status broken, got "
                        + asl::as_string(myStatus), PLUS_FILE_LINE);
    }

    if (!isOK) {
        throw OffscreenRendererException(os.str(), PLUS_FILE_LINE);
    }
}


OffscreenBuffer::OffscreenBuffer() :
    _myUseFBO(true),
    _myTextureNodeVersion(0),
    _myBlitFilter(GL_NEAREST)
{
    reset();
}

OffscreenBuffer::~OffscreenBuffer() {
    AC_DEBUG << "OffscreenBuffer:dtor called."; 
}

void OffscreenBuffer::setUseFBO(bool theUseFlag)
{
#ifdef GL_EXT_framebuffer_object
    if (theUseFlag && !IS_SUPPORTED(glGenFramebuffersEXT)) {
        AC_WARNING << "OpenGL FBO rendering requested but not supported, "
                   << "falling back to backbuffer rendering";
        theUseFlag = false;
    }
#endif
    _myUseFBO = theUseFlag;
}


void OffscreenBuffer::activate(TexturePtr theTexture, unsigned theSamples, 
                               unsigned theCubmapFace)
{
    unsigned myTextureId = theTexture->getTextureId();
    if (myTextureId == 0) {
        // ensure texture object exists
        myTextureId = theTexture->applyTexture();
    }

    if (_myUseFBO) {
        bindOffscreenFrameBuffer(theTexture, theSamples, theCubmapFace);
    }
}


void OffscreenBuffer::deactivate(TexturePtr theTexture, bool theCopyToImageFlag)
{
    unsigned myTextureId = theTexture->getTextureId();

#ifdef GL_EXT_framebuffer_object
    if (_myUseFBO) {
        if (_myFrameBufferObject[1]) {
            // blit multisample buffer to texture
            glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, _myFrameBufferObject[1]);
            glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, _myFrameBufferObject[0]);
            CHECK_OGL_ERROR;

            unsigned myWidth = theTexture->get<TextureWidthTag>();
            unsigned myHeight = theTexture->get<TextureHeightTag>();
            glBlitFramebufferEXT(0, 0, myWidth, myHeight,
                                 0, 0, myWidth, myHeight,
                                 GL_COLOR_BUFFER_BIT, _myBlitFilter);
            CHECK_OGL_ERROR;

            // restore
            glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, 0);
            glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, 0);
            glDrawBuffer(GL_BACK);
        } else {
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
        }
        CHECK_OGL_ERROR;

        // generate mipmap levels
        if (IS_SUPPORTED(glGenerateMipmapEXT) && theTexture->get<TextureMipmapTag>()) {
            AC_DEBUG << "OffscreenBuffer::deactivate: generating mipmap levels";
            glBindTexture(GL_TEXTURE_2D, myTextureId);
            glGenerateMipmapEXT(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }
    else
#endif
    {
        // copy backbuffer to texture
        glBindTexture(GL_TEXTURE_2D, myTextureId);
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0 /*MIPMAP level*/, 0, 0,
                0, 0, theTexture->get<TextureWidthTag>(), theTexture->get<TextureHeightTag>());
        CHECK_OGL_ERROR;

        // generate mipmap levels
        if (IS_SUPPORTED(glGenerateMipmapEXT) && theTexture->get<TextureMipmapTag>()) {
            AC_DEBUG << "OffscreenBuffer::deactivate: generating mipmap levels";
            glGenerateMipmapEXT(GL_TEXTURE_2D);
            CHECK_OGL_ERROR;
        }

        glBindTexture(GL_TEXTURE_2D, 0);
    }
    CHECK_OGL_ERROR;

    if (theCopyToImageFlag) {
        copyToImage(theTexture);
    }
}


void OffscreenBuffer::copyToImage(TexturePtr theTexture)
{
    ImagePtr myImage = theTexture->getImage();
    if (!myImage) {
        AC_ERROR << "Texture id='" << theTexture->get<IdTag>() << "' has no image associated";
        return;
    }
    AC_DEBUG << "OffscreenBuffer::copyToImage texture=" << theTexture->get<NameTag>() 
             << " image=" << myImage->get<NameTag>();

#ifdef GL_EXT_framebuffer_object
    if (_myUseFBO) {
        if (_myFrameBufferObject[1]) { // UH: not a bug, to determine if 
                                       // multisampling is enabled
            glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, _myFrameBufferObject[0]);
        } else {
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _myFrameBufferObject[0]);
        }
    }
#endif
 
    PixelEncodingInfo myPixelEncodingInfo = 
        getDefaultGLTextureParams(myImage->getRasterEncoding());
    myPixelEncodingInfo.internalformat = 
        asGLTextureInternalFormat(theTexture->getInternalEncoding());

    unsigned myWidth = myImage->get<ImageWidthTag>();
    unsigned myHeight = myImage->get<ImageHeightTag>();
    AC_TRACE << "pixelformat " << myImage->get<RasterPixelFormatTag>();
    AC_TRACE << "image size " << myWidth << "x" << myHeight;

    glReadPixels(0, 0, myWidth, myHeight,
                myPixelEncodingInfo.externalformat, myPixelEncodingInfo.pixeltype,
                myImage->getRasterPtr()->pixels().begin());        
    CHECK_OGL_ERROR;

    theTexture->triggerUpload();

#ifdef GL_EXT_framebuffer_object
    if (_myUseFBO) {
        if (_myFrameBufferObject[1]) { // UH: not a bug, to determine if 
                                       // multisampling is enabled
            glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, 0);
        } else {
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
        }
        CHECK_OGL_ERROR;
    }
#endif
}


void OffscreenBuffer::reset()
{
    _myFrameBufferObject[0] = _myFrameBufferObject[1] = 0;
    _myColorBuffer[0] = _myColorBuffer[1] = 0;
    _myDepthBuffer[0] = _myDepthBuffer[1] = 0;
}


void OffscreenBuffer::bindOffscreenFrameBuffer(TexturePtr theTexture, unsigned theSamples, 
                                               unsigned theCubemapFace)
{
#ifdef GL_EXT_framebuffer_object
    // rebind texture if target image has changed
    if (_myFrameBufferObject[0] 
        && theTexture->getNode().nodeVersion() != _myTextureNodeVersion) 
    {
        AC_DEBUG << "Tearing down FBO since Texture has changed " 
                 << theTexture->getNode().nodeVersion() << " != " << _myTextureNodeVersion;

        glDeleteFramebuffersEXT(2, &_myFrameBufferObject[0]);
        glDeleteRenderbuffersEXT(2, &_myColorBuffer[0]);
        glDeleteRenderbuffersEXT(2, &_myDepthBuffer[0]);

        reset();
    }

    unsigned myTextureId = theTexture->getTextureId();
    if (!_myFrameBufferObject[0]) {
        /*
         * create FBO
         */
        // XXX use power-of-two for image size since Y60 textures are POT only at the moment.
        // Once we support non-power-of-two textures this should be replaced with the actual
        // texture size (should come from Image/Texture object so that NPOT support is handled
        // properly)
        unsigned myWidth = theTexture->get<TextureWidthTag>(); 
        unsigned myHeight = theTexture->get<TextureHeightTag>(); 
//        if (theImage->get<ImageResizeTag>() == IMAGE_RESIZE_PAD) { 
//            myWidth = asl::nextPowerOfTwo(theImage->get<ImageWidthTag>());
//            myHeight = asl::nextPowerOfTwo(theImage->get<ImageHeightTag>());
//        }
        AC_DEBUG << "setup RTT framebuffer texture=" << theTexture->get<NameTag>() 
                 << " size=" << myWidth << "x" << myHeight;

        if (theSamples >= 1 
            && !(IS_SUPPORTED(glRenderbufferStorageMultisampleEXT) 
                 && IS_SUPPORTED(glBlitFramebufferEXT))) 
        {
            AC_WARNING << "Multisampling requested but not supported, turning it off";
            theSamples = 0;
        }

        if (theSamples >= 1) {
            /*
             * setup multisample framebuffer
             */
            int myMaxSamples;
            glGetIntegerv(GL_MAX_SAMPLES_EXT, &myMaxSamples);
            AC_DEBUG << "setup multisample framebuffer multisampling samples=" 
                     << theSamples << " max.samples=" << myMaxSamples;

            // color buffer
            glGenRenderbuffersEXT(1, &_myColorBuffer[1]);
            glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, _myColorBuffer[1]);
            TextureInternalFormat myImageFormat = theTexture->getInternalEncoding();
            glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT,
                                                theSamples, 
                                                asGLTextureInternalFormat(myImageFormat),
                                                myWidth, myHeight);
            checkOGLError(PLUS_FILE_LINE);

            // depth buffer
            glGenRenderbuffersEXT(1, &_myDepthBuffer[1]);
            glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, _myDepthBuffer[1]);
            glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT,
                    theSamples, GL_DEPTH_COMPONENT24,
                    myWidth, myHeight);
            checkOGLError(PLUS_FILE_LINE);

            // multisample framebuffer
            glGenFramebuffersEXT(1, &_myFrameBufferObject[1]);
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _myFrameBufferObject[1]);
            glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                    GL_RENDERBUFFER_EXT, _myColorBuffer[1]);
            glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                    GL_RENDERBUFFER_EXT, _myDepthBuffer[1]);
            checkOGLError(PLUS_FILE_LINE);
        }

        /*
         * setup render-to-texture framebuffer
         */
        _myTextureNodeVersion = theTexture->getNode().nodeVersion();
        _myColorBuffer[0] = myTextureId;

        AC_TRACE << "nodeVersion=" << _myTextureNodeVersion << " textureID=" 
                 << _myColorBuffer[0];

        // framebuffer
        glGenFramebuffersEXT(1, &_myFrameBufferObject[0]);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _myFrameBufferObject[0]);
        checkOGLError(PLUS_FILE_LINE);

        // color buffer
        switch (theTexture->getType()) {
            case TEXTURE_2D:
                AC_DEBUG << "attaching 2D texture";
                glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                                          GL_TEXTURE_2D, _myColorBuffer[0], 0);
                checkOGLError(PLUS_FILE_LINE);
                break;
            case TEXTURE_CUBEMAP:
                {
                    attachCubemapFace(theCubemapFace);
                    // for depth buffer (see below) the face dimensions are needed
                    // and not the dimension of the whole cubemap
                    /*
                    ImagePtr myImage = theTexture->getImage();
                    if (myImage) {
                        asl::Vector2i myTile = myImage->get<ImageTileTag>();
                        myWidth = theTexture->get<TextureWidthTag>() / myTile[0];
                        myHeight = theTexture->get<TextureHeightTag>() / myTile[1];
                    }
                    */
                }
                break;
            default:
                throw TextureException(std::string("Unknown texture type '")+
                        theTexture->get<NameTag>() + "'", PLUS_FILE_LINE);
        }
        checkOGLError(PLUS_FILE_LINE);

        if (theSamples == 0) {
            // depth buffer (only necessary when not multisampling)
            glGenRenderbuffersEXT(1, &_myDepthBuffer[0]);
            glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, _myDepthBuffer[0]);
            glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, 
                                     myWidth, myHeight);
            checkOGLError(PLUS_FILE_LINE);
            glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                    GL_RENDERBUFFER_EXT, _myDepthBuffer[0]);
            checkOGLError(PLUS_FILE_LINE);
        }

        checkFramebufferStatus();
    } else {
        /*
         * bind FBO
         */
        if (_myFrameBufferObject[1]) {
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _myFrameBufferObject[1]);
            glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
        } else {
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _myFrameBufferObject[0]);
        }

        if (_myColorBuffer[0] != myTextureId) {
            _myColorBuffer[0] = myTextureId;
            if (theTexture->getType() == TEXTURE_CUBEMAP) {
                attachCubemapFace(theCubemapFace);
            } else {
                glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                                          GL_TEXTURE_2D, _myColorBuffer[0], 0);
            }
        }
    }
#else
    throw OpenGLException("GL_EXT_framebuffer_object support not compiled", PLUS_FILE_LINE);
#endif
}

void OffscreenBuffer::attachCubemapFace(unsigned theCubemapFace) {
    AC_DEBUG << "attaching cubemap face " << theCubemapFace;
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                              asGLCubemapFace(theCubemapFace),
                              _myColorBuffer[0], 0);
    checkOGLError(PLUS_FILE_LINE);
}
}
