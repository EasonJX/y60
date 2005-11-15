//=============================================================================
// Copyright (C) 2003, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================
//
//   $RCSfile: Image.h,v $
//   $Author: david $
//   $Revision: 1.3 $
//   $Date: 2005/04/06 16:47:30 $
//
//  Description: A simple scene class.
//
//=============================================================================

#ifndef _ac_y60_Image_h_
#define _ac_y60_Image_h_

#include "PixelEncoding.h"
#include "ImageTags.h"
#include <y60/CommonTags.h>
#include <y60/NodeValueNames.h>

#include <dom/AttributePlug.h>
#include <dom/Facade.h>

namespace asl {
    class PackageManager;
}

namespace y60 {

    /**
     * Exception
     */
    DEFINE_EXCEPTION(ImageException, asl::Exception);
    /**
     * Exception
     */
    DEFINE_EXCEPTION(RasterValueDoesNotExist, ImageException);

    class ITextureManager;

    /**
     * @ingroup y60image
     * Facade for an image node in the dom.
     *
     */
    class Image :
        public dom::Facade,
        public IdTag::Plug,
        public NameTag::Plug,
        public ImageWidthTag::Plug,
        public ImageHeightTag::Plug,
        public ImageDepthTag::Plug,
        public ImageSourceTag::Plug,
        public ImagePixelFormatTag::Plug,
        public ImageInternalFormatTag::Plug,
        public ImageTypeTag::Plug,
        public ImageMipmapTag::Plug,
        public ImageResizeTag::Plug,
        public ImageFilterTag::Plug,
        public ImageFilterParamsTag::Plug,
        public ImageMatrixTag::Plug,
        public ImageColorBiasTag::Plug,
        public ImageColorScaleTag::Plug,
        public ImageTileTag::Plug,
        public dom::FacadeAttributePlug<ImageBytesPerPixelTag>
    {
        public:
            Image(dom::Node & theNode);
            IMPLEMENT_FACADE(Image);

            virtual ~Image();

            void storeTextureVersion();

            void setGraphicsId(unsigned theId) {
                _myTexId = theId;
            }

            unsigned getGraphicsId() const {
                return _myTexId;
            }

            void setTextureManager(const ITextureManager & theTextureManager);
            void registerTexture();
            void deregisterTexture();

            void set(unsigned int theNewWidth,
                    unsigned int theNewHeight,
                    unsigned int theNewDepth,
                    PixelEncoding theEncoding);
            void set(unsigned int theNewWidth,
                    unsigned int theNewHeight,
                    unsigned int theNewDepth,
                    PixelEncoding theEncoding,
                    const asl::ReadableBlock & thePixels);
            void set(unsigned int theNewWidth,
                    unsigned int theNewHeight,
                    unsigned int theNewDepth,
                    PixelEncoding theEncoding,
                    dom::ValuePtr theRaster);
            void createRaster(PixelEncoding theEncoding);
            //void makeHalfSize();

            virtual void load(asl::PackageManager & thePackageManager);
            virtual void load(const std::string & theImagePath = ".");

            ImageType getType() const;

            PixelEncoding getEncoding() const;

            unsigned getMemUsed() const{
                if (getRasterPtr()) {
                    return getRasterPtr()->pixels().size();
                } else {
                    return 0;
                }
            }

            // Image data access
            //unsigned char * getWritableData();
            //const unsigned char * getData(unsigned int theLayer = 0,
            //                              unsigned int theLayerNum = 1) const;
            void deleteData();

            /**
             * Checks if a reload from the source is required.
             * @retval true if a reload to the ResourceManager is required.
             */
            virtual bool reloadRequired() const;

            /**
             * Check if a texture upload is required since relevant
             * Image node values have changed.
             * @return true if an upload is required, else false.
             */
            virtual bool textureUploadRequired() const;

            /**
             * Checks if the applied texture and current Image node
             * parameters are compatible so that the texture can be
             * reused.
             * @retval true if the texture can be safely reused.
             * @retval false if the texture cannot be reused.
             */
            bool canReuseTexture() const;
            bool isImageNewerThanTexture() const {
                unsigned long long myNewVersion = getValueVersion();
                return _myTextureImageVersion < myNewVersion;
            }
            /** Saves the image to disk using the PNG format. 
             * @todo replace @p theVerticalFlipFlag by generic filter factory
             * string
             */
            void saveToFile(const std::string & theImagePath, bool theVerticalFlipFlag = false);

            unsigned long long getValueVersion() const{
                dom::NodePtr myValueNode = getRasterValueNode();
                if (myValueNode) {
                    return myValueNode->nodeVersion();
                }
                return 0;
            }
            dom::NodePtr getRasterValueNode() const {
                dom::NodePtr myValueElement = getNode().firstChild();
                if (myValueElement) {
                    return myValueElement->firstChild();
                }
                return dom::NodePtr(0);
            }
            dom::ValuePtr getRasterValue() {
                dom::NodePtr myValueNode = getRasterValueNode();
                if (myValueNode) {
                    return myValueNode->nodeValueWrapperPtr();
                }
                return dom::ValuePtr(0);
            }
            const dom::ValuePtr getRasterValue() const {
                dom::NodePtr myValueNode = getRasterValueNode();
                if (myValueNode) {
                    return myValueNode->nodeValueWrapperPtr();
                }
                return dom::ValuePtr(0);
            }
            void setRasterValue(dom::ValuePtr theRasterValue) {
                dom::NodePtr myValueNode = getRasterValueNode();
                if (myValueNode) {
                    myValueNode->nodeValueWrapperPtr(theRasterValue);
                    return;
                }
                throw RasterValueDoesNotExist(JUST_FILE_LINE);
            }
            dom::ResizeableRasterPtr getRasterPtr() {
                return dynamic_cast_Ptr<dom::ResizeableRaster>(getRasterValue());
            }
            const dom::ResizeableRasterPtr getRasterPtr() const {
                return dynamic_cast_Ptr<dom::ResizeableRaster>(getRasterValue());
            }
    protected:
            //asl::Ptr<asl::ReadableBlock> & getDataBlock() {
            //    return _myData;
            //}

            //void replaceBlock(asl::Ptr<asl::ReadableBlock> theData, unsigned theHeaderSize = 0);

            // This is neccessary to detect, when the src-Attribute does not correspond to the
            // loaded data. Hopefully we can find a more elegant solution with xml-events, soon.
            std::string                  _myLoadedFilename;
            std::string                  _myAppliedFilter;
            ImageFilterParamsTag::TYPE   _myAppliedFilterParams;
            asl::Vector4f                _myAppliedColorScale;
            asl::Vector4f                _myAppliedColorBias;
            std::string                  _myAppliedPixelFormat;
            std::string                  _myAppliedInternalFormat;
            bool                         _myAppliedMipmap;

        private:
            Image();
            void loadFromFile(asl::PackageManager & thePackageManager);

            unsigned                      _myTexId;

            int                           _myRefCount;
            asl::WeakPtr<ITextureManager> _myTextureManager;
            unsigned long long            _myTextureImageVersion;

            // used to detect texture size changes
            std::string                   _myTexturePixelFormat; // XXX how's this different from _myAppliedPixelFormat?!?!
            unsigned                      _myTextureWidth;
            unsigned                      _myTextureHeight;
            unsigned                      _myTextureDepth;
    };

    typedef asl::Ptr<Image, dom::ThreadingModel> ImagePtr;
    typedef asl::WeakPtr<Image, dom::ThreadingModel> ImageWeakPtr;

    inline
    ImageFilter lookupFilter(TextureUsage theUsage) {
        switch (theUsage) {
            case BUMP:
                return HEIGHT_TO_NORMALMAP;
            default:
                return NO_FILTER;
        }
        return NO_FILTER; // avoid gcc4.0 warning
    }
}

#endif
