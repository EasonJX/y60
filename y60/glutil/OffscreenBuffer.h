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
*/

#include <asl/base/Ptr.h>
#include <asl/dom/ThreadingModel.h>

#include "y60_glutil_settings.h"

#include <GL/glew.h>

#ifndef _ac_y60_OffscreenBuffer_h_
#define _ac_y60_OffscreenBuffer_h_

DEFINE_EXCEPTION( OffscreenRendererException, asl::Exception );

namespace y60 {
    class Texture;
    typedef asl::Ptr<Texture, dom::ThreadingModel> TexturePtr;
    class OffscreenBuffer;
    typedef asl::Ptr<OffscreenBuffer, dom::ThreadingModel> OffscreenBufferPtr;

    class Y60_GLUTIL_DECL OffscreenBuffer {
        public:
            /**
            * Allows Offscreen rendering into a texture.
            * Renders to an FBO by default but can fall-back to
            * backbuffer rendering.
            */
            OffscreenBuffer();
            ~OffscreenBuffer();

            /**
            * activate the textures as render targets and initializes FBOs
            * if necessary
            */
            void activate(const std::vector<TexturePtr> & theTextures,
                          unsigned int theSamples = 0, unsigned int theCubemapFace = 0);

            /**
            * deactivates the textures as render targets
            * @param theCopyToImageFlag copy result to texture rasters.
            */
            void deactivate(const std::vector<TexturePtr> & theTextures,
                            bool theCopyToImageFlag = false);

            /**
            * set to true if you want to render on a offscreen EXT_framebuffer_object
            * if false we render on the framebuffers back buffer
            */
            void setUseFBO(bool theFlag);
            inline bool getUseFBO() const {
                return _myUseFBO;
            }

            void copyToImage(TexturePtr theTexture, unsigned int theColorBufferIndex = 0);

        private:
            void reset();
            void copyToImage(const std::vector<TexturePtr> & theTextures);
            
            void bindFBO(const std::vector<TexturePtr> & theTextures,
                                          unsigned int theSamples, unsigned int theCubemapFace);
            void setupFBO(const std::vector<TexturePtr> & theTextures,
                                          unsigned int theSamples, unsigned int theCubemapFace);
            bool FBOrebindRequired(const std::vector<TexturePtr> & theTextures) const;
            void blitToTexture(const std::vector<TexturePtr> & theTextures);

            bool     _myUseFBO;
            bool     _myHasFBOMultisample;

            std::vector<asl::Unsigned64> _myTextureNodeVersions;
            unsigned int  _myTextureWidth;
            unsigned int  _myTextureHeight;
            
            unsigned int _myBlitFilter;

            GLuint _myFBO;
            std::vector<GLuint> _myColorBuffers;
            GLuint _myDepthBuffer;

            //multisampling buffer
            GLuint _myMultisampleFBO;
            std::vector<GLuint> _myMultisampleColorBuffers;
            GLuint _myMultisampleDepthBuffer;

    };
}

#endif
