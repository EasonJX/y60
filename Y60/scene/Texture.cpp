//=============================================================================
// Copyright (C) 1993-2007, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================

#include "Texture.h"

#include <y60/IScene.h>
#include <dom/Field.h>


using namespace asl;
using namespace std;

#undef DB
#define DB(x) // x

namespace y60 {

    DEFINE_EXCEPTION(UnknownEncodingException, asl::Exception);

    Texture::Texture(dom::Node & theNode) :
        Facade(theNode),
        IdTag::Plug(theNode),
        NameTag::Plug(theNode),
        TextureImageIdTag::Plug(theNode),
        TexturePixelFormatTag::Plug(theNode),
        //TextureTypeTag::Plug(theNode),
        TextureMipmapTag::Plug(theNode),
        TextureAnisotropyTag::Plug(theNode),
        TextureMatrixTag::Plug(theNode),
        TextureColorScaleTag::Plug(theNode),
        TextureColorBiasTag::Plug(theNode),
        TextureWrapModeTag::Plug(theNode),
        TextureMinFilterTag::Plug(theNode),
        TextureMagFilterTag::Plug(theNode),
		TextureImageIndexTag::Plug(theNode),
        dom::FacadeAttributePlug<TextureTypeTag>(this),
        dom::FacadeAttributePlug<TextureIdTag>(this),
        dom::FacadeAttributePlug<TextureInternalFormatTag>(this),
        dom::FacadeAttributePlug<TextureWidthTag>(this),
        dom::FacadeAttributePlug<TextureHeightTag>(this),
        dom::FacadeAttributePlug<TextureDepthTag>(this),
#ifdef BAD_TX
        dom::FacadeAttributePlug<TextureImageTag>(this),
#endif
        dom::FacadeAttributePlug<TextureParamChangedTag>(this),
        _myResourceManager(0),
        _myRefCount(0),
        _myTextureId(0), 
        _myPixelBufferId(0),
        _myImageNodeVersion(0)
    {
        AC_DEBUG << "Texture::Texture " << (void*) this;
    }

    Texture::~Texture() {
        AC_DEBUG << "Texture::~Texture " << (void*) this;
        if (_myResourceManager) {
            _myResourceManager->unbindTexture(this);
            unbind();
        }
    }

    void
    Texture::registerDependenciesRegistrators() {
        Facade::registerDependenciesRegistrators();
        AC_DEBUG << "Texture::registerDependenciesRegistrators '" << get<NameTag>();

        //TextureImageTag::Plug::setReconnectFunction(&Texture::registerDependenciesForImageTag);
        TextureIdTag::Plug::setReconnectFunction(&Texture::registerDependenciesForTextureUpdate);
        TextureParamChangedTag::Plug::setReconnectFunction(&Texture::registerDependenciesForTextureParamChanged);
        TextureInternalFormatTag::Plug::setReconnectFunction(&Texture::registerDependenciesForInternalFormatUpdate);

        TextureWidthTag::Plug::setReconnectFunction(&Texture::registerDependenciesForTextureWidthUpdate);
        TextureHeightTag::Plug::setReconnectFunction(&Texture::registerDependenciesForTextureHeightUpdate);
        TextureTypeTag::Plug::setReconnectFunction(&Texture::registerDependenciesForTextureTypeUpdate);
    }

    void
    Texture::registerDependenciesForImageTag() {
#ifdef BAD_TX
        if (getNode()) {
            TextureImageTag::Plug::dependsOn<TextureImageIdTag>(*this);
            TextureImageTag::Plug::getValuePtr()->setCalculatorFunction(
                dynamic_cast_Ptr<Texture>(getSelf()), &Texture::calculateImageTag);
        }
#endif
    }

    void
    Texture::registerDependenciesForTextureUpdate() {
        if (getNode()) {
#ifdef BAD_TX
            TextureIdTag::Plug::dependsOn<TextureImageTag>(*this);  
#endif
            TextureIdTag::Plug::dependsOn<TextureColorBiasTag>(*this);  
            TextureIdTag::Plug::dependsOn<TextureColorScaleTag>(*this); 
            TextureIdTag::Plug::dependsOn<TextureMipmapTag>(*this);
            TextureIdTag::Plug::dependsOn<TextureInternalFormatTag>(*this);
            //TextureIdTag::Plug::getValuePtr()->setCalculatorFunction(
            //    dynamic_cast_Ptr<Texture>(getSelf()), &Texture::applyTexture);
        }
    }

    void
    Texture::registerDependenciesForTextureWidthUpdate() {        
        AC_TRACE << "Texture::registerDependenciesForTextureWidthUpdate";
        if (getNode()) {
#ifdef BAD_TX
            TextureWidthTag::Plug::dependsOn<TextureImageTag>(*this);  
#endif
            /*TextureWidthTag::Plug::getValuePtr()->setCalculatorFunction(
                dynamic_cast_Ptr<Texture>(getSelf()), &Texture::calculateWidth);*/
        }
    }

    void
    Texture::registerDependenciesForTextureHeightUpdate() {        
        AC_TRACE << "Texture::registerDependenciesForTextureHeightUpdate";
        if (getNode()) {
#ifdef BAD_TX
            TextureHeightTag::Plug::dependsOn<TextureImageTag>(*this);  
#endif
            /*TextureHeightTag::Plug::getValuePtr()->setCalculatorFunction(
                dynamic_cast_Ptr<Texture>(getSelf()), &Texture::calculateHeight);*/
        }
    }

    void
    Texture::registerDependenciesForTextureTypeUpdate() {
        AC_TRACE << "Texture::registerDependenciesForTextureTypeUpdate";
        if (getNode()) {
#ifdef BAD_TX
            TextureTypeTag::Plug::dependsOn<TextureImageTag>(*this);
#endif
            TextureTypeTag::Plug::getValuePtr()->setCalculatorFunction(
                dynamic_cast_Ptr<Texture>(getSelf()), &Texture::calculateTextureType);
        }
    }

    void
    Texture::registerDependenciesForTextureParamChanged() {
        AC_TRACE << "Texture::registerDependenciesForTextureParamChanged";
        if (getNode()) {
            TextureParamChangedTag::Plug::dependsOn<TextureAnisotropyTag>(*this);
            TextureParamChangedTag::Plug::dependsOn<TextureWrapModeTag>(*this);
            TextureParamChangedTag::Plug::dependsOn<TextureMinFilterTag>(*this);
            TextureParamChangedTag::Plug::dependsOn<TextureMagFilterTag>(*this);
            /*TextureParamChangedTag::Plug::getValuePtr()->setCalculatorFunction(
                dynamic_cast_Ptr<Texture>(getSelf()), &Texture::applyTextureParams);*/
        }
    }

    void
    Texture::registerDependenciesForInternalFormatUpdate() {
        AC_TRACE << "Texture::registerDependenciesForInternalFormatUpdate";
        if (getNode()) {
#ifdef BAD_TX
            TextureInternalFormatTag::Plug::dependsOn<TextureImageTag>(*this);
#endif
            TextureInternalFormatTag::Plug::dependsOn(getImageFacade().getRasterValue());
            TextureInternalFormatTag::Plug::dependsOn<ImageSourceTag>(getImageFacade());
            TextureInternalFormatTag::Plug::dependsOn<TextureColorBiasTag>(*this);
            TextureInternalFormatTag::Plug::dependsOn<TextureColorScaleTag>(*this);
            TextureInternalFormatTag::Plug::dependsOn<TexturePixelFormatTag>(*this);
            TextureInternalFormatTag::Plug::getValuePtr()->setCalculatorFunction(
                dynamic_cast_Ptr<Texture>(getSelf()), &Texture::calculateInternalFormat);
        }
    }

    // This function may be correct but it looks fragile and might easily break when the dom structure
    // wil be changed.
    void
    Texture::ensureResourceManager() {
        AC_TRACE << "Texture::ensureResourceManager '" << get<NameTag>() << "' _myResourceManager=" << (void*)_myResourceManager;
        if (_myResourceManager == 0) {
            if (getNode()) {
                IScenePtr myScene = getNode().parentNode()->parentNode()->getFacade<IScene>();                
                _myResourceManager = myScene->getResourceManager();
            }
        }
        AC_TRACE << "Texture::ensureResourceManager (onexit)'" << get<NameTag>() << "' _myResourceManager=" << (void*)_myResourceManager;
    }

    void
    Texture::calculateWidth() {
        ImagePtr myImage = getImage();
        if (myImage) {
            AC_TRACE << "Texture::calculateWidth '" << get<NameTag>() << "' width=" << myImage->Image::get<ImageWidthTag>();
            set<TextureWidthTag>(myImage->Image::get<ImageWidthTag>());
        }
    }

    void
    Texture::calculateHeight() {
        ImagePtr myImage = getImage();
        if (myImage) {
            AC_TRACE << "Texture::calculateHeight '" << get<NameTag>() << "' height=" << myImage->Image::get<ImageHeightTag>();
            set<TextureHeightTag>(myImage->get<ImageHeightTag>());
        }
    }



    void 
    Texture::calculateInternalFormat() {
        std::string myInternalFormat = get<TexturePixelFormatTag>();     
        if (myInternalFormat.size()) {
            // If there is an texture pixel format set from outside, we use it
        } else {
            ImagePtr myImage = getImage();
            if (!myImage) {
                AC_ERROR << "Unable to determine internal pixelformat since no image is"
                         << " attached, texture '" << get<NameTag>() << "' id=" << get<IdTag>();
                return;
            }

            PixelEncoding myRasterFormat = myImage->getRasterEncoding();
            bool myAlphaChannelRequired = (get<TextureColorScaleTag>()[3] < 1.0f 
                                           || get<TextureColorBiasTag>()[3] > 0.0f);

            if (myAlphaChannelRequired 
                && (myRasterFormat == y60::RGB || myRasterFormat == y60::BGR)) 
            {
                // If a change in colorscale introduces an alpha channel ensure
                // that internal format has alpha
                myInternalFormat = getStringFromEnum(TEXTURE_IFMT_RGBA8, 
                                                     TextureInternalFormatStrings);
            } else {
                TextureInternalFormat myInternalPixelFormat = 
                    getInternalPixelFormat(myRasterFormat);
                myInternalFormat = getStringFromEnum(myInternalPixelFormat, 
                                                     TextureInternalFormatStrings);
            }
        }
        AC_DEBUG << "Texture::calculateInternalFormat '" << get<NameTag>() 
                 << "' internalFormat=" << myInternalFormat;
        set<TextureInternalFormatTag>(myInternalFormat);
    }



    void
    Texture::calculateTextureType() {
        ImagePtr myImage = getImage();
        if (myImage) {
            TextureType myType = TEXTURE_2D;
            if (myImage->get<ImageDepthTag>() > 1) {
                myType = TEXTURE_3D;
            } else {
                asl::Vector2i myTiles = myImage->get<ImageTileTag>();
                unsigned myNumTiles = myTiles[0] * myTiles[1];
                if (myNumTiles == 6) {
                    myType = TEXTURE_CUBEMAP;
                } else if (myNumTiles != 1) {
                    AC_ERROR << "Number of texture tiles is not six (cubemap) and not one"
                             << " (texture_2D), assuming texture_2D";
                }
            }
            AC_DEBUG << "Texture::calculateTextureType '" << get<NameTag>() 
                     << "' textureType=" << myType;
            set<TextureTypeTag>(myType);
        }
    }


    void
    Texture::calculateImageTag() {
        AC_DEBUG << "Texture::calculateImageTag '" << get<NameTag>() << "'";
        const std::string & myImageId = get<TextureImageIdTag>();
        dom::NodePtr myImageNode = getNode().getElementById(myImageId);
        if (!myImageNode) {
            AC_ERROR << "Texture '" << get<NameTag>() << "' id=" << get<IdTag>() << " references invalid image id=" << myImageId;
            return;
        }
        ImagePtr myImage = myImageNode->getFacade<Image>();
#ifdef BAD_TX
        set<TextureImageTag>(ImageWeakPtr(myImage));
#endif
    }

    void
    Texture::preload() {
        AC_DEBUG << "Texture::triggerUpload '" << get<NameTag>() << "' id=" << get<IdTag>();
        TextureIdTag::Plug::getValuePtr()->setDirty(); // force call to applyTexture()
        AC_INFO << "TextureManager::reloadTextures preload() disabled;";
        applyTexture();
    }
   
    unsigned 
    Texture::applyTexture() {

        ensureResourceManager();

        AC_TRACE << "Texture::applyTexture '" << get<NameTag>() << "' id=" << get<IdTag>() << " texId=" << _myTextureId;
        
        TexturePtr myTexture = dynamic_cast_Ptr<Texture>(getSelf());
        
        bool myForceSetupFlag = isDirty<TextureIdTag>();
        myForceSetupFlag |= get<TextureIdTag>() ? false:true;
        bool myImageContentChangedFlag = false;

        // setup flags
        ImagePtr myImage = getImage();
        if (myImage && myImage->getNode().nodeVersion() != _myImageNodeVersion) {
            bool myImageMatchesTextureFlag = _myResourceManager->imageMatchesGLTexture(myTexture);
            if (!myImageMatchesTextureFlag) {
                myForceSetupFlag = true;
            } else {
                // do upload
                myImageContentChangedFlag = true;
            }

            _myImageNodeVersion = myImage->getNode().nodeVersion();
        } 

        // perform actions
        if (myForceSetupFlag && _myTextureId != 0) {
            _myResourceManager->unbindTexture(this);
            _myTextureId = 0;
        }
        
        if (myForceSetupFlag) {
            _myTextureId = _myResourceManager->setupTexture(myTexture);
            set<TextureIdTag>(_myTextureId);
        } else {

            if (isDirty<TextureParamChangedTag>()) {
                _myResourceManager->updateTextureParams(myTexture);
                (void) get<TextureParamChangedTag>();
            } 

            if (myImageContentChangedFlag) {
                _myResourceManager->updateTextureData(myTexture);
                (void) get<TextureIdTag>();
            }
        }

        return _myTextureId;
    }

    void
    Texture::applyTextureParams() {
        AC_DEBUG << "Texture::applyTextureParams";
        _myResourceManager->updateTextureParams(dynamic_cast_Ptr<Texture>(getSelf()));
    }

    TextureType
    Texture::getType() const {
        return get<TextureTypeTag>();
    }

    Image &
    Texture::getImageFacade() {
        ImagePtr myImage = getImage();
        if (!myImage) {
            throw asl::Exception(string("Texture ") + get<NameTag>() + ": Could not find image with id=" + get<TextureImageIdTag>(), PLUS_FILE_LINE);
        }
        return *myImage;
    }

    const Image &
    Texture::getImageFacade() const {
        ImagePtr myImage = getImage();
        if (!myImage) {
            throw asl::Exception(string("Texture ") + get<NameTag>() + ": Could not find image with id=" + get<TextureImageIdTag>(), PLUS_FILE_LINE);
        }
        return *myImage;
    }

    ImagePtr
    Texture::getImage() const {
#ifdef BAD_TX
        return get<TextureImageTag>().lock();
#else
      const std::string & myImageId = get<TextureImageIdTag>();
        dom::NodePtr myImageNode = getNode().getElementById(myImageId);
        if (!myImageNode) {
            // AC_ERROR << "Texture '" << get<NameTag>() << "' id=" << get<IdTag>() << " references invalid image id=" << myImageId;
            // afaik, this is not an error, because the image-texture separation implies that one can have a texture which doesn`t
            // have a representing image. hence one can create a texture from an image and throw away the image afterwards. [sh]
            return ImagePtr(0);
        }
        ImagePtr myImage = myImageNode->getFacade<Image>();
        return myImage;
#endif
    }

    TextureInternalFormat
    Texture::getInternalEncoding() const {
        return TextureInternalFormat(getEnumFromString(get<TextureInternalFormatTag>(), TextureInternalFormatStrings));
    }

    void 
    Texture::removeTextureFromResourceManager() {
        //AC_DEBUG << "removeTextureFromResourceManager '" << get<NameTag>() << "' id=" << get<IdTag>() << " texId=" << _myTextureId;
        if (_myTextureId == 0) {
            return;
        }
        ensureResourceManager();
        _myResourceManager->unbindTexture(this); // calls Texture::unbind
        unbind();
    }

    TextureWrapMode
    Texture::getWrapMode() const {
        return get<TextureWrapModeTag>();
    }

    TextureSampleFilter
    Texture::getMinFilter() const {
        return get<TextureMinFilterTag>();
    }
    
    TextureSampleFilter
    Texture::getMagFilter() const {
        return get<TextureMagFilterTag>();
    }

    void 
    Texture::unbind() {
        if (_myTextureId == 0) {
            return;
        }
        AC_DEBUG << "Texture::unbind '" << get<NameTag>() << "' id=" << get<IdTag>() << " texId=" << _myTextureId;
        _myTextureId = 0;
        set<TextureIdTag>(_myTextureId);
        //TextureIdTag::Plug::getValuePtr()->setDirty();
    }

    void
    Texture::refTexture() {
        if (_myRefCount++ == 0) {
            ensureResourceManager();
            _myResourceManager->setTexturePriority(dynamic_cast_Ptr<Texture>(getSelf()), TEXTURE_PRIORITY_IN_USE);
        }
    }

    void
    Texture::unrefTexture() {
        if (--_myRefCount == 0) {
            ensureResourceManager();
            _myResourceManager->setTexturePriority(dynamic_cast_Ptr<Texture>(getSelf()), TEXTURE_PRIORITY_IDLE);
        }
    }

    bool
    Texture::usePixelBuffer() const {
        // not much use for regular images
        return false;
    }
}
