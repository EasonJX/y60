//=============================================================================
// Copyright (C) 1993-2005, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================
//
//   $RCSfile: Texture.cpp,v $
//   $Author: pavel $
//   $Revision: 1.25 $
//   $Date: 2005/04/24 00:41:21 $
//
//
//=============================================================================
#include "Texture.h"
#include "TextureManager.h"

#include <y60/NodeNames.h>
#include <y60/NodeValueNames.h>
#include <asl/string_functions.h>
#include <asl/Logger.h>

using namespace asl;

namespace y60 {

    Texture::Texture(dom::Node & theNode) :
        Facade(theNode),
        TextureImageIdTag::Plug(theNode),
        TextureApplyModeTag::Plug(theNode),
        TextureWrapModeTag::Plug(theNode),
        TextureSpriteTag::Plug(theNode),
		TextureMatrixTag::Plug(theNode),
        TextureMinFilterTag::Plug(theNode),
        TextureMagFilterTag::Plug(theNode),
        dom::FacadeAttributePlug<TextureImageTag>(this)
    {
    }

    Texture::~Texture() {
        AC_TRACE << "Texture destructor";
        ImagePtr myImage = getImage();
        if (myImage) {
            myImage->deregisterTexture();
        }
    }

    void
    Texture::registerDependenciesRegistrators() {
        Facade::registerDependenciesRegistrators();
        TextureImageTag::Plug::setReconnectFunction(&Texture::registerDependenciesForImageTag);
    }
    void
    Texture::registerDependenciesForImageTag() {        
        TextureImageTag::Plug::dependsOn<TextureImageIdTag>(*this);  
        TexturePtr mySelf = dynamic_cast_Ptr<Texture>(getSelf());
        TextureImageTag::Plug::getValuePtr()->setCalculatorFunction(mySelf, &Texture::update);
        /*if (getNode() && getNode().hasFacade() && _myImage) {
            dom::FacadeAttributePlug<TextureIdTag>::dependsOn<ImageColorBiasTag>(*getImage());  
            Image::TextureIdTag::Plug::dependsOn<ImageColorScaleTag>(*getImage());  
            TexturePtr mySelf = dynamic_cast_Ptr<Texture>(getSelf());
            Image::TextureIdTag::Plug::getValuePtr()->setCalculatorFunction(mySelf, &Texture::invalidateTexture);

        }*/
    }

    void 
    Texture::setTextureManager(const TextureManagerPtr theTextureManager) {
        _myTextureManager = theTextureManager;
    }

    void
    Texture::update() {
        AC_TRACE << "Texture::update";

        //if (!_myImage || getImage().getNativePtr() != myImageInDom.getNativePtr()) {

        ImagePtr myOldImage = getImage();
        if (myOldImage) {
            myOldImage->deregisterTexture();
        }

        ImagePtr myNewImage = _myTextureManager->findImage(get<TextureImageIdTag>());            
        myNewImage->registerTexture();
        set<TextureImageTag>(ImageWeakPtr(myNewImage));
        //} 
    }

    TextureApplyMode
    Texture::getApplyMode() const {
        return TextureApplyMode(getEnumFromString( get<TextureApplyModeTag>(), TextureApplyModeStrings));
	}
	
    TextureWrapMode
    Texture::getWrapMode() const {
        return TextureWrapMode(getEnumFromString( get<TextureWrapModeTag>(), TextureWrapModeStrings));
    }

    TextureSampleFilter
    Texture::getMinFilter() const {
        return TextureSampleFilter(getEnumFromString( get<TextureMinFilterTag>(), TextureSampleFilterStrings));
    }
    
    TextureSampleFilter
    Texture::getMagFilter() const {
        return TextureSampleFilter(getEnumFromString( get<TextureMagFilterTag>(), TextureSampleFilterStrings));
    }
    
    ImagePtr
    Texture::getImage() const {
        AC_TRACE << "Texture::getImage";
		return get<TextureImageTag>().lock();
/*        if (_myImage) {
            return _myImage.lock();
        } else {            
            throw TextureException(std::string("Invalid image in texture: ") + asl::as_string(getNode()), PLUS_FILE_LINE);
        }*/
    }

    /*Image & 
    Texture::getImageRef() { 
        ImagePtr myImageNode = getImage();
        if (!myImageNode) {
            throw asl::Exception(string("Texure ") + get<NameTag>() + ": Could not find image with id: " + get<TextureImageTag>(), PLUS_FILE_LINE);
        } else {
            return *(myImageNode->getFacade<Image>());
        }         
    }*/

	unsigned
    Texture::getId() const {
        ImagePtr myImage = getImage();
        if (myImage) {
            return myImage->get<TextureIdTag>();
        } else {
            throw TextureException(std::string("Invalid image in texture: ") + asl::as_string(getNode()), PLUS_FILE_LINE);
        }
    }
}
