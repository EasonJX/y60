//============================================================================
//
// Copyright (C) 2007, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//============================================================================

#include "Blobs.h"
#include <asl/numeric_functions.h>

using namespace asl;
using namespace std;
using namespace dom;

namespace y60 {

    Box2f
    asBox2f( const Box2i & theBox ) {
        return Box2f( Vector2f( float(theBox[Box2i::MIN][0]), float(theBox[Box2i::MIN][1])),
                      Vector2f( float(theBox[Box2i::MAX][0]), float(theBox[Box2i::MAX][1])));
    }


    Blobs::Blobs(const std::string & theName) :
        Algorithm(theName),
        _myResultNode("result"),
        _myThreshold(5),
        _myIDCounter( 0 ),
        _myDistanceThreshold( 0 ),
        _myCarCounter( 0 )
    {   
    }
  

    void 
    Blobs::configure(const dom::Node & theNode) {
        
        for( unsigned int i=0; i<theNode.childNodesLength(); i++) {
            const std::string myName = theNode.childNode("property",i)->getAttribute("name")->nodeValue();
            const std::string myValue = theNode.childNode("property",i)->getAttribute("value")->nodeValue();
            dom::NodePtr myNode = _myScene->getSceneDom()->getElementById(myValue);
            AC_PRINT << "configure " << myName;
            if( myNode ) {
                if( myName == "sourceimage") {
                    _mySourceImage = myNode->getFacade<y60::Image>();
                } else if( myName == "targetimage") {
                    _myTargetImage = myNode->getFacade<y60::Image>();
                } 
            } else {
                if( myName == "threshold") {
                    asl::fromString(myValue, _myThreshold);
                } else if( myName == "overlay") {
                    std::vector<NodePtr> myResults;
                    dom::NodePtr myNode = _myScene->getSceneDom()->firstChild()->childNode(CANVAS_LIST_NAME);
                    myNode->getNodesByAttribute(OVERLAY_NODE_NAME, NAME_ATTRIB, myValue, true, myResults);
                    _myOverlay = myResults[0];
                } else if( myName == "distance") {
                    asl::fromString(myValue, _myDistanceThreshold);
                }      
            }
        }   
    }

	void 
    Blobs::onFrame(double t) {
        y60::ImagePtr myGrayImage = _mySourceImage;

        const GRAYRaster * mySourceFrame = dom::dynamic_cast_Value<GRAYRaster>(&*_mySourceImage->getRasterValue());
        const GRAYRaster * myTargetFrame = dom::dynamic_cast_Value<GRAYRaster>(&*_myTargetImage->getRasterValue());
        
        dom::ResizeableRasterPtr myResizeableRasterPtr = _mySourceImage->getRasterPtr();

        // // left-to-right horizontal pass
        GRAYRaster::iterator itSrc = const_cast<GRAYRaster::iterator>(mySourceFrame->begin());
        GRAYRaster::iterator itTarget = const_cast<GRAYRaster::iterator>(myTargetFrame->begin());
        
        for (itSrc; itSrc != mySourceFrame->end(); ++itSrc, ++itTarget) {
            (*itTarget) = (*itSrc);
        }
        
        BlobListPtr myBlobs = connectedComponents( _myTargetImage->getRasterPtr(), static_cast<int>(_myThreshold));
        
        groupBlobs(myBlobs);

        correlatePositions( myBlobs );
        
	}

    void
    Blobs::groupBlobs( BlobListPtr & theROIs ) {

    }

    void 
    Blobs::correlatePositions( BlobListPtr & theROIs )
    {
        Matrix4f myTransform = getTransformationMatrix();

        const BlobList & myROIs = * theROIs;

        // populate a map with all distances between existing cursors and new positions 
        typedef std::multimap<float, std::pair<int,int> > DistanceMap;
        DistanceMap myDistanceMap;
        float myDistanceThreshold = _myDistanceThreshold;
        AC_TRACE << "cursors: " <<_myCursors.size()<< " current positions: " << myROIs.size() << "; distance threshold: " << myDistanceThreshold;
        CursorMap::iterator myCursorIt  = _myCursors.begin();
        for (; myCursorIt != _myCursors.end(); ++myCursorIt ) {
            myCursorIt->second.correlatedPosition = -1;
            for (unsigned i = 0; i < myROIs.size(); ++i) {
                
                float myDistance = magnitude( myCursorIt->second.position + myCursorIt->second.motion
                                              -  (*theROIs)[i]->center());
                //AC_PRINT << myDistance << " id " << myCursorIt->first;
                if (myDistance < myDistanceThreshold) {
                    myDistanceMap.insert(std::make_pair(myDistance, std::make_pair(i, myCursorIt->first)));
                }
            }
        }

        // will contain the correlated cursor id at index n for position n or -1 if uncorrelated
        std::vector<int> myCorrelatedPositions(myROIs.size(), -1);

        AC_TRACE << "distance map is " << myDistanceMap.size() << " elements long.";

        //AC_PRINT << "myCars" << _myCarCounter;
        _myResultNode = dom::Element("result");
        _myResultNode.appendChild(Element("cars")); 
        _myResultNode.childNode("cars")->appendChild(Text("")); 
        dom::Node & centerNode = *(_myResultNode.childNode("cars"));
        centerNode.childNode("#text")->nodeValue(asl::as_string(_myCarCounter));
        
        
        // iterate through the distance map and correlate cursors in increasing distance order
        for (DistanceMap::iterator dit = myDistanceMap.begin(); 
             dit!=myDistanceMap.end(); ++dit)
            {
                // check if we already have correlated one of our nodes
                int myPositionIndex = dit->second.first;
                int myCursorId = dit->second.second;
                if (_myCursors[myCursorId].correlatedPosition == -1) {
                    if (myCorrelatedPositions[myPositionIndex] == -1)  {
                        // correlate
                        myCorrelatedPositions[myPositionIndex] = myCursorId;
                        Cursor & myCursor = _myCursors[myCursorId];
                        myCursor.correlatedPosition = myPositionIndex;

                        AC_TRACE << "correlated cursor " << myCursorId << " with " << myPositionIndex;

                        Element myPosition("position");
                        myPosition.appendAttribute("last", asl::as_string(myCursor.position));
                        myPosition.appendAttribute("id", myCursorId);

                        // update cursor with new position
                        asl::Vector2f myCenter = (*theROIs)[myPositionIndex]->center();
                        myPosition.appendAttribute("current", asl::as_string(myCenter));
                        
                        myCursor.motion = myCenter - myCursor.position;
                        myCursor.position = myCenter;
                        myCursor.previousRoi = myCursor.roi;
                        myCursor.roi = asBox2f( (*theROIs)[myPositionIndex]->bbox() );

                        _myResultNode.appendChild(myPosition); 
                    }
                    // place an "else" block here for code if we want to allow to correlate multiple cursors to the same position,
                    // but he have to take care that they will be separated at some later point again
                }
            }

        // Now let us iterate through all new positions and create "cursor add" events for every uncorrelated position
        unsigned int myCars = 0;
        for (unsigned i = 0; i < myROIs.size(); ++i) {
            if (myCorrelatedPositions[i] == -1)  {
                // new cursor
                int myNewID( _myIDCounter++ );
                AC_TRACE << "new cursor " <<myNewID<< " at " << (*theROIs)[i]->center();
                myCorrelatedPositions[i] = myNewID;
                _myCursors.insert( make_pair( myNewID, Cursor( (*theROIs)[i]->center(),
                                asBox2f( (*theROIs)[i]->bbox()) )));
                _myCursors[myNewID].correlatedPosition = i;
                myCars++;
                _myCarCounter++;
            }
        }


        // Now let us iterate through all cursors and create "cursor remove" events for every uncorrelated cursors
        for (CursorMap::iterator myIt = _myCursors.begin(); myIt != _myCursors.end(); ) {
            // advance iterator to allow safe map erase while iterating
            CursorMap::iterator nextIt = myIt;
            ++nextIt;
            if (myIt->second.correlatedPosition == -1) {
                // cursor removed
                AC_TRACE << "removing cursor "<<myIt->first<<" at " << myIt->second.correlatedPosition;
                _myCursors.erase(myIt);
                _myCarCounter--;
            }
            myIt = nextIt;
        }
    }

 //    void 
//     Blobs::correlatePositions( BlobListPtr & theBlobs)
//     {

//         Matrix4f myTransform = getTransformationMatrix();

//         const BlobList & myBlobs = *theBlobs;

//         std::vector<int> myCorrelatedIds;
//         unsigned int myCars = 0;
//         for (unsigned i = 0; i < myBlobs.size(); ++i) {
//             float myMinDistance = 10000.0;
//             CursorMap::iterator myMinDistIt  = _myCursors.end();
//             CursorMap::iterator myCursorIt  = _myCursors.begin();
//             for (; myCursorIt != _myCursors.end(); ++myCursorIt ) {
//                 if ( find( myCorrelatedIds.begin(), myCorrelatedIds.end(), myCursorIt->first ) ==
//                      myCorrelatedIds.end())
//                     {   
//                         asl::Vector2f myCenter =  (*theBlobs)[i]->center(); 
//                         float myDistance = magnitude( myCursorIt->second.position - myCenter);
//                         if (myDistance < myMinDistance) {
//                             myMinDistance = myDistance;
//                             myMinDistIt = myCursorIt;
//                         }
//                     }
//             }
//             float myDistanceThreshold = 18.0f;
//             //AC_PRINT << "distance threshold: " << myDistanceThreshold;
//             if (myMinDistIt != _myCursors.end() && myMinDistance < myDistanceThreshold) {
//                 // cursor moved
//                 AC_PRINT << "blob moved";
//             } else {
//                 // new cursor
//                 //AC_PRINT << "new blob";
//                 _myCarCounter++;
//                 myCars++;

//                 int myNewID( _myIDCounter++ );    
//                 _myCursors.insert( make_pair( myNewID, Cursor( (*theBlobs)[i]->center(),
//                                                                asBox2f( myBlobs[i]->bbox()) )));
//                 myCorrelatedIds.push_back( myNewID );
//             }   
//         }

//         _myResultNode = dom::Element("result");
//         _myResultNode.appendChild(Element("cars")); 
//         _myResultNode.childNode("cars")->appendChild(Text("")); 
//         dom::Node & centerNode = *(_myResultNode.childNode("cars"));
//         centerNode.childNode("#text")->nodeValue(asl::as_string(myCars));

//         CursorMap::iterator myIt = _myCursors.begin();
//         std::vector<int> myOutdatedCursorIds;
//         for (; myIt != _myCursors.end(); ++myIt) {
//             if ( find( myCorrelatedIds.begin(), myCorrelatedIds.end(), myIt->first ) ==
//                  myCorrelatedIds.end())
//                 {
//                     // cursor removed
//                     myOutdatedCursorIds.push_back( myIt->first );
//                     _myCarCounter--;

//                 }
//         }

//         for (unsigned i = 0; i < myOutdatedCursorIds.size(); ++i) {
//             _myCursors.erase( myOutdatedCursorIds[i] );
//         }
                 
//         //AC_PRINT << "cursors length " << _myCursors.size() << " car count " << _myCarCounter;
        
//     }

    asl::Matrix4f
    Blobs::getTransformationMatrix() {
        Matrix4f myTransform;
        if (_myOverlay ) {
            const y60::Overlay & myOverlay = *(_myOverlay->getFacade<y60::Overlay>());
            const Vector2f & myPosition = myOverlay.get<Position2DTag>();
            const Vector2f & myScale = myOverlay.get<Scale2DTag>();
            const float & myOrientation = myOverlay.get<Rotation2DTag>();

            myTransform.makeIdentity();

            myTransform.translate( Vector3f( 0.5, 0.5, 0));

            myTransform.scale( Vector3f( myScale[0], myScale[1], 0));
            myTransform.translate( Vector3f( myPosition[0], myPosition[1], 0));
            myTransform.rotateZ( myOrientation );
        } else {
            myTransform.makeIdentity();
        }
        return myTransform;
    }

    


}
