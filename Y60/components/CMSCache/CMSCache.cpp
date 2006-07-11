//==============================================================================
// Copyright (C) 2006, ART+COM AG Berlin, Germany
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//==============================================================================

#include "CMSCache.h"

#include <asl/Logger.h>
#include <asl/file_functions.h>
#include <y60/Request.h>

using namespace std;
using namespace asl;
using namespace inet;

#define VERBOSE_PRINT \
    _myVerboseFlag && AC_PRINT 
namespace y60 {

CMSCache::CMSCache(const string & theLocalPath,
                       dom::NodePtr thePresentationDocument,
                       const std::string & theUsername,
                       const std::string & thePassword) :
     _myPresentationDocument(thePresentationDocument),
     _myLocalPath( theLocalPath ),
     _myUsername(theUsername),
     _myPassword(thePassword),
     _myStatusDocument( new dom::Document()),
     _myVerboseFlag( false )
{
    dom::NodePtr myReport = dom::NodePtr( new dom::Element("report") );
    _myStatusDocument->appendChild( myReport );
    _myAssetReportNode = dom::NodePtr( new dom::Element("assets") );
    myReport->appendChild( _myAssetReportNode );
    _myStalledFilesNode = dom::NodePtr( new dom::Element("stalledfiles") );
    myReport->appendChild( _myStalledFilesNode );
}

CMSCache::~CMSCache() {
    //AC_DEBUG << "closing Zip " << _myZipFilename;
}

void
CMSCache::login() {
    std::string someAsset = _myAssets.begin()->second->getAttributeString("uri");
    
    //OCS doesn't like foreign user agents, that's why we claim to be wget! [jb,ds]
    RequestPtr myLoginRequest(new Request( someAsset, "Wget/1.10.2"));
    myLoginRequest->head();
    if (_myUsername.size() && _myPassword.size()) {
        myLoginRequest->setCredentials(_myUsername, _myPassword);
    }
    _myRequestManager.performRequest(myLoginRequest);
    
    int myRunningCount = 0;
    do {
        myRunningCount = _myRequestManager.handleRequests(); 
        asl::msleep(10);
    } while (myRunningCount);

    if (myLoginRequest->getResponseCode() != 200 ||
        myLoginRequest->getResponseHeader("Set-Cookie").size() == 0) {
            throw CMSCacheException("Login failed for user '" + _myUsername + 
                    "' at URL '" + someAsset + "'.", PLUS_FILE_LINE);
    }
    
    _mySessionCookie = myLoginRequest->getResponseHeader("Set-Cookie");
}

void
CMSCache::collectExternalAssetList() {
    if ( ! _myPresentationDocument || _myPresentationDocument->childNodesLength() == 0 ) {
        throw CMSCacheException("No presentation file. Bailing out.", PLUS_FILE_LINE);
    }
    
    dom::NodePtr myRoot;
    if (_myPresentationDocument->childNode(0)->nodeType() == dom::Node::PROCESSING_INSTRUCTION_NODE) {
        myRoot = _myPresentationDocument->childNode(1)->childNode("themepool", 0);
    } else {
        myRoot = _myPresentationDocument->childNode(0)->childNode("themepool", 0);
    }
    
    if ( ! myRoot ) {
        throw CMSCacheException("Failed to find themepool", PLUS_FILE_LINE);
    }
    
    collectAssets(myRoot);

    VERBOSE_PRINT << "Found " << _myAssets.size() << " assets.";
}

void
CMSCache::collectAssets(dom::NodePtr theParent) {
    for (unsigned i = 0; i < theParent->childNodesLength(); ++i) {
        dom::NodePtr myChild = theParent->childNode( i );
        if (myChild->nodeName() == "externalcontent") {
            const std::string & myPath = myChild->getAttributeString("path");
            if (_myAssets.find( myPath ) == _myAssets.end()) {
                if ( myChild->getAttributeString("uri").empty() ) {
                    AC_WARNING << "Asset has no URI: " << * myChild;
                } else {
                    dom::NodePtr myClone = myChild->cloneNode( dom::Node::DEEP );
                    _myAssetReportNode->appendChild( myClone );
                    myClone->appendAttribute("status", "uptodate");
                    _myAssets.insert( std::make_pair(myPath, myClone ));
                }
            }
        }
        collectAssets( myChild );
    }    
}

void
CMSCache::addAssetRequest(dom::NodePtr theAsset) {
    VERBOSE_PRINT << "Fetching " << _myLocalPath + "/" + theAsset->getAttributeString("path");
    AssetRequestPtr myRequest(new AssetRequest( theAsset, _myLocalPath, _mySessionCookie));
    _myAssetRequests.insert(std::make_pair( & ( * theAsset), myRequest));
    theAsset->getAttribute("status")->nodeValue("downloading");
    theAsset->appendAttribute("progress", "0.0");
    _myRequestManager.performRequest(myRequest);
}

void
CMSCache::synchronize() {

    collectExternalAssetList();
    updateDirectoryHierarchy();
    removeStalledAssets();
    
    if ( ! _myAssets.empty()) {
        login();
        collectOutdatedAssets();
        fillRequestQueue();
    }
}

void
CMSCache::collectOutdatedAssets() {
    std::map<std::string, dom::NodePtr>::iterator myIter = _myAssets.begin();
    for (; myIter != _myAssets.end(); myIter++) {
        if ( isOutdated( myIter->second )) {
            VERBOSE_PRINT << "Asset " << myIter->second->getAttributeString("path")
                     << " is outtdated.";
            myIter->second->getAttribute("status")->nodeValue("outdated");
            _myOutdatedAssets.push_back( myIter->second );
        }
    }
}

bool
CMSCache::isOutdated( dom::NodePtr theAsset ) {
    std::string myFile = _myLocalPath + "/" + theAsset->getAttributeString("path");
    if ( fileExists( myFile )) {
        time_t myLocalTimestamp = getLastModified( myFile );
        time_t myServerTimestamp = Request::getTimeFromHTTPDate(
                    theAsset->getAttributeString("lastmodified"));
        if (myServerTimestamp <= myLocalTimestamp) {
            return false;
        } 
    }
    return true;
}

void
CMSCache::fillRequestQueue() {
    for (unsigned i = _myAssetRequests.size(); i < MAX_REQUESTS; ++i) {
        if ( ! _myOutdatedAssets.empty()) {
            dom::NodePtr myAsset = _myOutdatedAssets.back();
            _myOutdatedAssets.pop_back();
            addAssetRequest( myAsset );
        } else {
            break;
        }
    }
}

bool
CMSCache::isSynchronized() {
    int myRunningCount = _myRequestManager.handleRequests(); 

    // TODO: Error statistics and handling
    AssetRequestMap::iterator myIter = _myAssetRequests.begin();
    while (myIter != _myAssetRequests.end()) {
        if (myIter->second->isDone()) {
            int myResponseCode = myIter->second->getResponseCode();
            if ( myResponseCode == 200) {
                std::string myFilename = _myLocalPath + "/" +
                        myIter->first->getAttributeString("path");
                time_t myTime = Request::getTimeFromHTTPDate(
                        myIter->first->getAttributeString("lastmodified"));
                setLastModified(myFilename, myTime);
            } else {
                // TODO: retry handling
            }
            _myAssetRequests.erase( myIter++ );
        } else {
            ++myIter;
        }
    }

    fillRequestQueue();
    return _myOutdatedAssets.empty() && _myAssetRequests.empty() && (myRunningCount == 0);
}


void
CMSCache::updateDirectoryHierarchy() {
    if ( ! fileExists( _myLocalPath )) {
        createPath( _myLocalPath );
    }
    std::map<std::string, dom::NodePtr>::iterator myIter = _myAssets.begin();
    while (myIter != _myAssets.end()) {
        std::string myPath = _myLocalPath + "/" +
                getDirectoryPart(myIter->second->getAttributeString("path"));
        if ( ! fileExists( myPath )) {
            createPath( myPath );
        }
        myIter++;
    }
}

void
CMSCache::removeStalledAssets() {
    scanStalledEntries( _myLocalPath );
}

void
CMSCache::scanStalledEntries(const std::string & thePath) {
    std::vector<std::string> myEntries = getDirectoryEntries(thePath);
    std::vector<std::string>::iterator myIter = myEntries.begin();
    for (; myIter != myEntries.end(); ++myIter) {
        std::string myEntry = thePath + "/" + (*myIter);
        if (isDirectory(myEntry)) {
            scanStalledEntries(myEntry);
        } else {
            std::string myFilename = myEntry.substr(_myLocalPath.size() + 1,
                        myEntry.size() - _myLocalPath.size());
            if (_myAssets.find(myFilename) == _myAssets.end()) {
                VERBOSE_PRINT << "Removing '" << myFilename << "'.";
                dom::NodePtr myFileNode( new dom::Element("file"));
                myFileNode->appendAttribute("path", myFilename );
                myFileNode->appendAttribute("status", "removed");
                _myStalledFilesNode->appendChild( myFileNode );
                deleteFile(_myLocalPath + "/" + myFilename);
            }
        }
    }
}

dom::NodePtr
CMSCache::getStatusReport() {
    return _myStatusDocument;
}

}
