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
//   $RCSfile: Scene.h,v $
//   $Author: danielk $
//   $Revision: 1.94 $
//   $Date: 2005/03/31 16:57:53 $
//
//  Description: A simple scene class.
//
//=============================================================================

#ifndef _ac_scene_Scene_h_
#define _ac_scene_Scene_h_

#include <y60/IScene.h>

#include "IShader.h"
#include "MaterialBase.h"
#include "AnimationManager.h"
#include "Shape.h"
#include "Camera.h"
#include "Light.h"
#include "TextureManager.h"
#include "Y60Decoder.h"

#include "SceneBuilder.h"
#include <asl/PackageManager.h>
#include <asl/Matrix4.h>
#include <asl/Box.h>
#include <dom/Nodes.h>
#include "ResourceManager.h"

#include <string>
#include <map>


namespace asl {
    template <class T> class LineSegment;
}

namespace y60 {

    class Primitive;

    DEFINE_EXCEPTION(SceneException, asl::Exception);
    DEFINE_EXCEPTION(SomError, asl::Exception);

    struct BodyInfo {
        BodyInfo() {};
        BodyInfo( dom::NodePtr theBody, y60::ShapePtr theShape, const asl::Matrix4f & theTransformation,
                const asl::Matrix4f & theInverseTransformation ) :
                _myBody( theBody ), _myShape( theShape ), _myTransformation( theTransformation ),
                _myInverseTransformation( theInverseTransformation )
        {}
        dom::NodePtr _myBody;
        y60::ShapePtr _myShape;
        asl::Matrix4f _myTransformation;
        asl::Matrix4f _myInverseTransformation;
    };

    struct IntersectionInfo : public BodyInfo {
        IntersectionInfo() : BodyInfo() {};
        IntersectionInfo( dom::NodePtr theBody, y60::ShapePtr theShape,
                const asl::Matrix4f & theTransformation,
                const asl::Matrix4f & theInverseTransformation,
                asl::Ptr<Primitive::IntersectionList> thePrimitiveIntersections ) :
                BodyInfo( theBody, theShape, theTransformation, theInverseTransformation ),
                _myPrimitiveIntersections( thePrimitiveIntersections )
        {}
        asl::Ptr<Primitive::IntersectionList> _myPrimitiveIntersections;
    };

    struct CollisionInfo : public BodyInfo{
        asl::Ptr<Primitive::SphereContactsList> _myPrimitiveSphereContacts;
    };

    typedef std::vector<IntersectionInfo> IntersectionInfoVector;
    typedef std::vector<CollisionInfo> CollisionInfoVector;

    /**
     * @ingroup Y60scene
     * 3D Scene object.
     *
     */
    class Scene : public dom::Facade, public IScene {
        public:
            struct Statistics {
                Statistics();
                unsigned long primitiveCount;
                unsigned long vertexCount;
                unsigned long materialCount;
                unsigned long lightCount;
            };

            //typedef std::map<std::string, MaterialBasePtr> MaterialIdMap;

            DEFINE_NESTED_EXCEPTION(Scene,Exception,asl::Exception);
            DEFINE_NESTED_EXCEPTION(Scene,IOError,Exception);
            DEFINE_NESTED_EXCEPTION(Scene,OpenFailed,IOError);
            DEFINE_NESTED_EXCEPTION(Scene,ReadFailed,IOError);
            DEFINE_NESTED_EXCEPTION(Scene,WriteFailed,IOError);
            DEFINE_NESTED_EXCEPTION(Scene,InvalidIdReference,asl::Exception);

            /**
             * Constructs a Scene object
             * @param theShaderLibrary Library of shaders, constructed from an XML file
             */
            Scene(dom::Node & theNode = dom::Node::Prototype);
            IMPLEMENT_FACADE(Scene);
            virtual ~Scene();

            /**
             * Loads the file given in theFilename into the scene. It tries all available decoders
             * to decode the file. By default the X60/B60 decoder is installed. To decode other
             * file formats, register their decoders with the DecoderManager
             * @param theFilename File to decode into the scene
             * @param thePackageManager PackageManager for the file theFilename
             * @param theNotifier progress callback interface
             * @param useSchema use a schema to decode?
             * @param loadLazy load catalog and root, load children only on demand
             * @see DecoderManager
             */
            static asl::Ptr<Scene,dom::ThreadingModel> load(const std::string & theFilename, asl::PackageManagerPtr thePackageManager,
                      const IProgressNotifierPtr & theNotifier,
                      bool useSchema, bool loadLazy);
            /**
             * Imports the {B|X}60-File at theFilename of thePackageManager into the current Scene. It's
             * world node is inserted under theRoot.
             * @param theFilename file to insert
             * @param thePackageManager package containing the file
             * @param theRoot node to insert to files world below.
             */
            void import(const std::string & theFilename, asl::PackageManagerPtr thePackageManager,
                dom::NodePtr theRoot = dom::NodePtr(0));

            /**
             * Loads the file given in theFilename from the Stream theSource into the scene. It tries
             * all available decoders to decode the file. By default the X60/B60 decoder is installed.
             * To decode other file formats, register their decoders with the DecoderManager
             * @param theSource ReadableStream to read theFilename from
             * @param theFilename File to decode into the scene
             * @param useSchema use a schema to decode?
             * @param loadLazy load catalog and root, load children only on demand
             * @see DecoderManager
             */
            static asl::Ptr<Scene,dom::ThreadingModel> load(asl::Ptr<asl::ReadableStreamHandle> theSource, const std::string & theFilename,
                      const IProgressNotifierPtr & theNotifier,
                      bool useSchema, bool loadLazy);

            /**
             * Creates the stub nodes for the current (empty) scene.
             * @param thePackageManager Sets the PackageManger of the scene. The Packagemanger is used to load textures and stuff
             */
            static asl::Ptr<Scene,dom::ThreadingModel> createStubs(asl::PackageManagerPtr thePackageManager);
            /**
             * Sets up an empty scene document
             * @param theDomDocument Document to setup
             * @param theSchemaFilename schema filename to create the document from
             */
            static void setupEmptyDocument(dom::Document & theDomDocument, const std::string & theSchemaFilename);
            void clear();

            asl::Ptr<TextureManager> getTextureManager() const {
                return _myTextureManager;
            }

            /**
             * Update bit-flags. Used in calls to update.
             * @see update
             */
            enum {
                MATERIALS         = 1,
                ANIMATIONS        = 2,
                ANIMATIONS_LOAD   = 4,
                SHAPES            = 8,
                WORLD             = 16,
                ALL               = 31
            };

            /**
             * Checks the version of all subnodes and updates them if necessary by caling
             * update with the corresponding parameters. It uses the nodeVersion of the
             * nodes to check if the were modified.
             */
            void updateAllModified();
            /**
             * updates the components of the scene that are set in myUpdateFlags
             * @param myUpdateFlags components of the scene to update.
             */
            void update(unsigned short myUpdateFlags = MATERIALS+SHAPES);

            // Reduces the scene to one body with one shape
            void optimize(dom::NodePtr theRootNode = dom::NodePtr(0));
            void collectGarbage();

            /**
             * Saves the scene to the file given in theFilename
             * @param theFilename name of the file to save the scene to
             * @param theBinaryFlag if true, the scene is saved in binary b60 format, else
             *        it is saved in xml x60 format.
             */
            void save(const std::string & theFilename, bool theBinaryFlag = false);
            void saveWithCatalog(const std::string & theFilename, const std::string & theCatalogFilenname, bool theBinaryFlag = true);

            // no need to call these threee functions directly, but it does no harm
            void saveBinaryWithCatalog(const std::string & theFilename, const std::string & theCatalogFilenname);
            void saveAsText(const std::string & theFilename);
            void saveBinary(const std::string & theFilename);
 
            void saveSchema(const std::string & theFilename,
                            int theSchemaIndex, bool theBinaryFlag = false);

            LightVector & getLights() {
                return _myLights;
            }
            std::vector<dom::NodePtr> & getAnalyticGeometry() {
                return _myAnalyticGeometry;
            }

            const Statistics getStatistics() const;

            void collectCameras(dom::NodeList & theCameras) const;

            dom::DocumentPtr getSceneDom() {
                return _mySceneDom;
            }
            const dom::DocumentPtr getSceneDom() const {
                return _mySceneDom;
            }

            dom::NodePtr getTexturesRoot();
            const dom::NodePtr getTexturesRoot() const;

            dom::NodePtr getImagesRoot();
            const dom::NodePtr getImagesRoot() const;

            dom::NodePtr getCanvasRoot();
            const dom::NodePtr getCanvasRoot() const;

            dom::NodePtr getRecordsRoot();
            const dom::NodePtr getRecordsRoot() const;

            dom::NodePtr getMaterialsRoot();
            const dom::NodePtr getMaterialsRoot() const;

            float getWorldSize(const dom::NodePtr & theActiveCamera) const;

            dom::NodePtr getWorldRoot();
            const dom::NodePtr getWorldRoot() const;

            dom::NodePtr getShapesRoot();
            const dom::NodePtr getShapesRoot() const;

            SceneBuilderPtr getSceneBuilder() {
                return _mySceneBuilder;
            }
            void setSceneBuilder(SceneBuilderPtr theSceneBuilder) {
                _mySceneBuilder = theSceneBuilder;
            }
            const AnimationManager & getAnimationManager() const {
                return _myAnimationManager;
            }
            AnimationManager & getAnimationManager() {
                return _myAnimationManager;
            }
            ShapePtr getShape(const std::string & theId) {
                dom::NodePtr myShapeNode = _mySceneDom->getElementById(theId);
                if (myShapeNode) {
                    return myShapeNode->getFacade<Shape>();
                }
                return ShapePtr(0);
            }
            const ShapePtr getShape(const std::string & theId) const {
                dom::NodePtr myShapeNode = _mySceneDom->getElementById(theId);
                if (myShapeNode) {
                    return myShapeNode->getFacade<Shape>();
                }
                return ShapePtr(0);
            }

            bool intersectWorld(const asl::LineSegment<float> & theStick,
                                IntersectionInfoVector & theIntersectionsbool,
                                bool theIntersectInvisibleBodysFlag = true);
            bool intersectWorld(const asl::Ray<float> & theRay,
                                IntersectionInfoVector & theIntersections,
                                bool theIntersectInvisibleBodysFlag = true);
            bool intersectWorld(const asl::Line<float> & theLine,
                                IntersectionInfoVector & theIntersections,
                                bool theIntersectInvisibleBodysFlag = true);

            bool collideWithWorld(const asl::Sphere<float> & theSphere,
                                  const asl::Vector3<float> & theMotion,
                                  CollisionInfoVector & theCollisions,
                                  bool theIntersectInvisibleBodysFlag = true);
            bool collideWithWorld(const asl::Sphere<float> & theSphere,
                                  const asl::Vector3<float> & theMotion,
                                  CollisionInfo & theCollision,
                                  bool theIntersectInvisibleBodysFlag = true);

            static bool intersectBodyCenters(dom::NodePtr theRootNode,
                                 const asl::Box3<float> & theBox,
                                 IntersectionInfoVector & theIntersections,
                                 bool theIntersectInvisibleBodysFlag = true);
            static bool intersectBodies(dom::NodePtr theRootNode,
                                 const asl::Box3<float> & theBox,
                                 IntersectionInfoVector & theIntersections,
                                 bool theIntersectInvisibleBodysFlag = true);
            static bool intersectBodies(dom::NodePtr theRootNode,
                                 const asl::LineSegment<float> & theStick,
                                 IntersectionInfoVector & theIntersections,
                                 bool theIntersectInvisibleBodysFlag = true);
            static bool intersectBodies(dom::NodePtr theRootNode,
                                 const asl::Ray<float> & theRay,
                                 IntersectionInfoVector & theIntersections,
                                 bool theIntersectInvisibleBodysFlag = true);
            static bool intersectBodies(dom::NodePtr theRootNode,
                                 const asl::Line<float> & theLine,
                                 IntersectionInfoVector & theIntersections,
                                 bool theIntersectInvisibleBodysFlag = true);

            static bool collideWithBodies(dom::NodePtr theRootNode,
                                    const asl::Sphere<float> & theSphere,
                                    const asl::Vector3<float> & theMotion,
                                    CollisionInfoVector & theCollisions,
                                    bool theIntersectInvisibleBodysFlag = true);
            static bool collideWithBodies(dom::NodePtr theRootNode,
                                    const asl::Sphere<float> & theSphere,
                                    const asl::Vector3<float> & theMotion,
                                    CollisionInfo & theCollision,
                                    bool theIntersectInvisibleBodysFlag = true);

	        void reverseUpdateShape(ShapePtr theShape);
            void reverseUpdateShapes();

            void setup();
            void clearShapes();

            /**
             * Register a ResourceManager with the scenes Texturemanager. The ResourceManager
             * is used to render Textures in the context it is created for. If the Scene does
             * not get rendered anymore, call deregisterResourceManager.
             * @warn Only call this method once from one renderer
             * @param theResourceManager ResourceManager to register
             */
            void registerResourceManager(ResourceManager* theResourceManager);
            /**
             * Deregisters a previously registered ResourceManager.
             * @warn Only call this method once when disconnecting a renderer.
             */
            void deregisterResourceManager();

            const IResourceManager * getResourceManager() const;
            IResourceManager * getResourceManager();

        private:
            void setSceneDom(dom::DocumentPtr theDocument);
            void updateReferences(dom::NodePtr theRootNode,
                std::map<std::string, std::string> & theOldToNewIdMap);
            void createUniqueIds(dom::NodePtr theDocument, dom::NodePtr theNode,
                std::map<std::string, std::string> & theOldToNewIdMap, const std::string & theFilename);

            void buildShape(ShapePtr theShape);
            void updateShapes();
            void loadMaterial(dom::NodePtr theMaterialNode);
            void reloadMaterial(dom::NodePtr theMaterialNode, MaterialBasePtr theMaterial);
            void rebindMaterials();            
            void loadAnimations();
            void updateMaterials();

            void collectReferences(dom::NodePtr theNode, std::set<std::string> & theReferences);
            void removeDangelingNodes(dom::NodePtr theNode, dom::NodePtr theDocument);
            bool isDangelingNode(dom::NodePtr theNode, dom::NodePtr theDocument);
            void removeUnreferencedNodes(dom::NodePtr theNode, const std::set<std::string> & theReferences);

            void calculateShapeBoundingBox(ShapePtr myShape);

            void updateTransformHierachy(dom::NodePtr theNode,
                                         const asl::Matrix4f & theParentMatrix);

            Primitive & createPrimitive(int theMaterialNumber,
                                        const std::string & theShapeId, unsigned int theDomIndex);
            void collectCameras(dom::NodeList & theCameras, dom::NodePtr theNode) const;

            template <class VISITOR>
            static bool visitBodys(VISITOR & theVisitor, dom::NodePtr theNode, bool theIntersectInvisibleBodysFlag);

            static
            dom::NodePtr getVertexDataNode(dom::NodePtr theShapeNode, const std::string & theDataName);

            unsigned findMaxIndexSize(dom::NodePtr theElementsNode);

            IShaderLibraryPtr getShaderLibrary() const;
            void setupShaderLibrary();

            SceneBuilderPtr          _mySceneBuilder;
            TextureManagerPtr        _myTextureManager;
            AnimationManager         _myAnimationManager;

            LightVector               _myLights;
            std::vector<dom::NodePtr> _myAnalyticGeometry;

            dom::DocumentPtr         _mySceneDom;
            Statistics               _myStatistics;

            unsigned long long       _myPreviousDomVersion;
    };

    typedef asl::Ptr<Scene,dom::ThreadingModel> ScenePtr;
}

#endif

