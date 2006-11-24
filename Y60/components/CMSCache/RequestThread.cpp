//==============================================================================
// Copyright (C) 2006, ART+COM AG Berlin, Germany
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//==============================================================================

#include "RequestThread.h"
#include <y60/Request.h>
#include <asl/file_functions.h>
#include <asl/os_functions.h>

using namespace std;
using namespace asl;
using namespace inet;

namespace y60 {

RequestThread::RequestThread(const std::string & theLocalPath, 
                      const std::string & theUsername, 
                      const std::string & thePassword,
                      const std::string & theUserAgent,
                      const std::vector<std::pair<std::string, std::string> > & theOutdatedAssets,
                      unsigned int theMaxRequestCount) :
        _myLocalPath(theLocalPath), 
        _myUsername(theUsername), 
        _myPassword(thePassword), 
        _myUserAgent(theUserAgent), 
        _myOutdatedAssets(theOutdatedAssets),
        _myMaxRequestCount(theMaxRequestCount),
        _remainingCount(theOutdatedAssets.size())
    {
        fillRequestQueue();
    };


int 
RequestThread::getRemainingCount() const {
    asl::ScopeLocker(_remainingCountLock, false);
    return _remainingCount;
}
void 
RequestThread::setRemainingCount(unsigned int theNewCount) {
    asl::ScopeLocker(_remainingCountLock, true);
    _remainingCount = theNewCount;
}

void 
RequestThread::addToCookieJar(const std::string & theCookieString) {
    // JSESSIONID=c04c810430da7ff877e211624fbcb7e591c88151b82e.e34LaNmQaNeMci0Mb38LbhmPbxj0n6jAmljGr5XDqQLvpAe; path=/content
    string::size_type myEquals = theCookieString.find("=");
    if (myEquals == string::npos) {
        AC_WARNING << "illegal cookie header:" << theCookieString;
    }
    string myCookieName = theCookieString.substr(0, myEquals);
    string myCookieValue = theCookieString.substr(myEquals+1);

    string::size_type myDelimiter = myCookieValue.find(";");
    if (myDelimiter != string::npos) {
        myCookieValue = myCookieValue.substr(0, myDelimiter);
    }

    // now add name/value to map
    _myCookieJar[myCookieName] = myCookieValue;
}

bool 
RequestThread::handleRequests() {
    int myRunningCount = _myRequestManager.handleRequests(true);

    // TODO: Error statistics and handling
    AssetRequestMap::iterator myIter = _myAssetRequests.begin();
    while (myIter != _myAssetRequests.end()) {
        if (myIter->second->isDone()) {
            int myResponseCode = myIter->second->getResponseCode();
            if ( myResponseCode == 200) {
                std::string myFilename = _myLocalPath + "/" + myIter->first;
                time_t myTime = Request::getTimeFromHTTPDate(
                         myIter->second->getResponseHeader("Last-Modified"));
                setLastModified(myFilename, myTime);
            }  else if ( myResponseCode == 304) { 
                // not modified, everything ok
            } else {
                string myReason = myIter->second->getResponseHeader("X-ORA-CONTENT-Info");
                    if (!myReason.empty()) {
                        AC_PRINT << "OCS server reason: '" << myReason << "' .";
                    }
                // TODO: retry handling
            }
            _myAssetRequests.erase( myIter++ );
        } else {
            AC_TRACE << "  still running:" << myIter->second->getResponseCode() << ":" << myIter->first;
            ++myIter;
        }
    }

    fillRequestQueue();
    //AC_WARNING << "queued:" << _myOutdatedAssets.size() << ", transferring" << _myAssetRequests.size();
    unsigned int remainingCount = _myOutdatedAssets.size() + _myAssetRequests.size();
    setRemainingCount(remainingCount);
    return (remainingCount == 0) && (myRunningCount == 0);
}

void
RequestThread::fillRequestQueue() {
    for (unsigned i = _myAssetRequests.size(); i < _myMaxRequestCount; ++i) {
        if ( ! _myOutdatedAssets.empty()) {
            addAssetRequest( _myOutdatedAssets.back().first, _myOutdatedAssets.back().second );
            _myOutdatedAssets.pop_back();
        } else {
            break;
        }
    }
}


void
RequestThread::addAssetRequest(const std::string & thePath, const std::string & theURI) {
    // AC_PRINT << "Fetching " << _myLocalPath + "/" + thePath;
    AssetRequestPtr myRequest;
    if (_myUserAgent.empty()) {
        myRequest = AssetRequestPtr(new AssetRequest( this, thePath, theURI, _myLocalPath, _myCookieJar));
    } else {
        myRequest = AssetRequestPtr(new AssetRequest( this, thePath, theURI, _myLocalPath, _myCookieJar, _myUserAgent));
    }
    if ( ! _myUsername.empty() ) {
        myRequest->setCredentials(_myUsername, _myPassword, DIGEST);
    }
    // myRequest->setVerbose(true);
    _myAssetRequests.insert(make_pair(thePath, myRequest));
    _myRequestManager.performRequest(myRequest);
}



void 
RequestThread::run () {
    while (!shouldTerminate()) {
        if (handleRequests()) {
            break;
        }
    }
}

}

