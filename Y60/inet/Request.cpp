//=============================================================================
// Copyright (C) 1993-2006, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================

#include "Request.h"

#include <asl/MappedBlock.h>

#include <iostream>
#include <asl/string_functions.h>
#include <asl/os_functions.h>


using namespace std;
using namespace asl;

#define CURL_VERBOSE 0
#define DB(x) // x

#ifdef verify
// OSX macro pollution
#undef verify
#endif

IMPLEMENT_ENUM(inet::AuthentType, inet::AuthentTypeStrings);

namespace inet {

    Request::Request(const string & theURL, const string & theUserAgent) :
        _myErrorBuffer(CURL_ERROR_SIZE, '\0'), _myURL(theURL), _myUserAgent(theUserAgent),
        _myHttpHeaderList(0)

        {
            CURLcode myStatus;
            _myCurlHandle = curl_easy_init();
            DB(AC_TRACE << "got new handle: " << _myCurlHandle << " for " << theURL << endl);

            myStatus = curl_easy_setopt(_myCurlHandle, CURLOPT_URL, _myURL.c_str());
            checkCurlStatus(myStatus, PLUS_FILE_LINE);

            curl_easy_setopt(_myCurlHandle, CURLOPT_ERRORBUFFER, &(*_myErrorBuffer.begin()));
            checkCurlStatus(myStatus, PLUS_FILE_LINE);

            setVerbose(CURL_VERBOSE);
            verifyPeer(true); // default: check ssl cert

            // curl handles can save a single user-data char *
            // we abuse this to save a this backpointer
            myStatus = curl_easy_setopt(_myCurlHandle, CURLOPT_PRIVATE, this);
            checkCurlStatus(myStatus, PLUS_FILE_LINE);

            DB(AC_TRACE << "registering callbacks" << endl);
            // register onData callback
            myStatus = curl_easy_setopt(_myCurlHandle, CURLOPT_WRITEFUNCTION, &Request::curlWriteCallback);
            checkCurlStatus(myStatus, PLUS_FILE_LINE);
            myStatus = curl_easy_setopt(_myCurlHandle, CURLOPT_WRITEDATA, this);
            checkCurlStatus(myStatus, PLUS_FILE_LINE);

            // register onProgress callback
            myStatus = curl_easy_setopt(_myCurlHandle, CURLOPT_PROGRESSFUNCTION, &Request::curlProgressCallback);
            checkCurlStatus(myStatus, PLUS_FILE_LINE);
            myStatus = curl_easy_setopt(_myCurlHandle, CURLOPT_PROGRESSDATA, this);
            checkCurlStatus(myStatus, PLUS_FILE_LINE);
            myStatus = curl_easy_setopt(_myCurlHandle, CURLOPT_NOPROGRESS, false);
            checkCurlStatus(myStatus, PLUS_FILE_LINE);

            // register onHeader callback
            myStatus = curl_easy_setopt(_myCurlHandle, CURLOPT_HEADERFUNCTION, &Request::curlHeaderCallback);
            checkCurlStatus(myStatus, PLUS_FILE_LINE);
            myStatus = curl_easy_setopt(_myCurlHandle, CURLOPT_WRITEHEADER, this);
            checkCurlStatus(myStatus, PLUS_FILE_LINE);

            // set User-Agent <g>
            myStatus = curl_easy_setopt(_myCurlHandle, CURLOPT_USERAGENT, _myUserAgent.c_str());
            checkCurlStatus(myStatus, PLUS_FILE_LINE);
        }

    Request::~Request() {
        DB(AC_TRACE << "cleaning up " << _myURL << endl);
        curl_slist_free_all (_myHttpHeaderList);
        curl_easy_cleanup(_myCurlHandle);
    }

    void
    Request::checkCurlStatus(CURLcode theStatusCode, const string & theWhere) const {
        if (theStatusCode != CURLE_OK) {
            throw INetException(string(&(*_myErrorBuffer.begin())), theWhere);
        }
    }

    CURL*
    Request::getHandle() const {
        return _myCurlHandle;
    }

    const string &
    Request::getURL() const {
        return _myURL;
    }

    void
    Request::setResume(long theResumeOffset) {
        CURLcode myStatus = curl_easy_setopt(_myCurlHandle, CURLOPT_RESUME_FROM, theResumeOffset);
        checkCurlStatus(myStatus, PLUS_FILE_LINE);
    }

    void
    Request::setProxy(const std::string & theProxyServer, bool theTunnelFlag) {
        CURLcode myStatus = curl_easy_setopt(_myCurlHandle, CURLOPT_PROXY, theProxyServer.c_str());
        checkCurlStatus(myStatus, PLUS_FILE_LINE);
        myStatus = curl_easy_setopt(_myCurlHandle, CURLOPT_HTTPPROXYTUNNEL, theTunnelFlag);
        checkCurlStatus(myStatus, PLUS_FILE_LINE);
    }

    long
    Request::getResponseCode() const {
        long myResponseCode = 0;
        CURLcode myStatus = curl_easy_getinfo(getHandle(), CURLINFO_HTTP_CODE, &myResponseCode);
        checkCurlStatus(myStatus, PLUS_FILE_LINE);
        return myResponseCode;
    }

    std::string
    Request::getResponseString() const {
        return std::string(_myResponseBlock.strbegin(), _myResponseBlock.strend());
    };

    const asl::Block &
    Request::getResponseBlock() const {
        return _myResponseBlock;
    };

    void
    Request::setVerbose(bool theVerboseFlag) {
        _myVerboseFlag = theVerboseFlag;
        CURLcode myStatus = curl_easy_setopt(_myCurlHandle, CURLOPT_VERBOSE, _myVerboseFlag);
        checkCurlStatus(myStatus, PLUS_FILE_LINE);
    }

    void
    Request::setLowSpeedLimit(unsigned theBytesPerSec) {
        _myLowSpeedLimit = theBytesPerSec;
        CURLcode myStatus = curl_easy_setopt(_myCurlHandle, CURLOPT_LOW_SPEED_LIMIT, theBytesPerSec);
        checkCurlStatus(myStatus, PLUS_FILE_LINE);
    }

    void
    Request::setLowSpeedTimeout(unsigned theSeconds) {
        _myLowSpeedTimeout = theSeconds;
        CURLcode myStatus = curl_easy_setopt(_myCurlHandle, CURLOPT_LOW_SPEED_TIME, theSeconds);
        checkCurlStatus(myStatus, PLUS_FILE_LINE);
    }

    void
    Request::setTimeoutParams(unsigned theBytesPerSec, unsigned theSeconds) {
        setLowSpeedLimit(theBytesPerSec);
        setLowSpeedTimeout(theSeconds);
    }

    const multimap<std::string,std::string> &
    Request::getResponseHeaders() const {
        return _myResponseHeaders;
    }

    unsigned
    Request::getLowSpeedLimit(void) const {
        return _myLowSpeedLimit;
    }

    unsigned
    Request::getLowSpeedTimeout(void) const {
        return _myLowSpeedTimeout;
    }

    std::string
    Request::getResponseHeader(const string & theHeader) const {
        multimap<string,string>::const_iterator it = _myResponseHeaders.find(theHeader);
        if (it != _myResponseHeaders.end()) {
            return it->second;
        }
        return "";
    }

    std::vector<std::string>
    Request::getAllResponseHeaders(const string & theHeader) const {
        multimap<string,string>::const_iterator it = _myResponseHeaders.begin();
        std::vector<std::string> myResults;
        while (it != _myResponseHeaders.end()) {
            if (it->first == theHeader) {
                myResults.push_back( it->second );
            }
            ++it;
        }
        return myResults;
    }

    time_t
    Request::getResponseHeaderAsDate(const string & theHeader) const {
        return getTimeFromHTTPDate( getResponseHeader(theHeader).c_str());
    }

    time_t
    Request::getTimeFromHTTPDate(const std::string & theHTTPDate ) {
        Time myTime;
        myTime.parse(theHTTPDate);
        return static_cast<time_t>(myTime);
    }

    string
    Request::getErrorString() const {
        return string(&(*_myErrorBuffer.begin()));
    }

    void
    Request::setCredentials(const std::string & theUsername, const std::string & thePassword, AuthentType theAuthentType) {
        _myAuthentData = theUsername + ":" + thePassword;

        // set the credentials for basic/digest authentication [jb]
        CURLcode myStatus = curl_easy_setopt(_myCurlHandle, CURLOPT_USERPWD, _myAuthentData.c_str());
        checkCurlStatus(myStatus, PLUS_FILE_LINE);
        unsigned myOption;
        switch (theAuthentType) {
            case BASIC:
                myOption = CURLAUTH_BASIC;
                break;
            case DIGEST:
                myOption = CURLAUTH_DIGEST;
                break;
            case ANY:
                // ANY: tries basic authentication before digest authentication [jb]
                myOption = CURLAUTH_ANY;
                break;
            default:
                throw INetException("Unkown authentication method '" + as_string(theAuthentType) + "'.", PLUS_FILE_LINE);

        }
        myStatus = curl_easy_setopt(_myCurlHandle, CURLOPT_HTTPAUTH, myOption);
        checkCurlStatus(myStatus, PLUS_FILE_LINE);
    }

    void
    Request::setFollowLocation(bool theFollowFlag) {
        // follow the "Location"-header field on response codes 3xx (redirection) [jb]
        CURLcode myStatus = curl_easy_setopt(_myCurlHandle, CURLOPT_FOLLOWLOCATION, theFollowFlag);
        checkCurlStatus(myStatus, PLUS_FILE_LINE);
    }

    void
    Request::addHttpHeader(const std::string & theKey, const std::string & theValue) {
        _myHttpHeaderBuffers.push_back(theKey + ": " + theValue);
        _myHttpHeaderList = curl_slist_append(_myHttpHeaderList, _myHttpHeaderBuffers.back().c_str());
        CURLcode myStatus = curl_easy_setopt(_myCurlHandle, CURLOPT_HTTPHEADER, _myHttpHeaderList);
        checkCurlStatus(myStatus, PLUS_FILE_LINE);
    }

    void
    Request::addHttpHeaderAsDate(const std::string & theKey, const time_t & theValue) {
        // change to RFC1162 date formatted string
        struct tm * myTimeStruct = gmtime(&theValue);
        char myBuffer[128];
        strftime(myBuffer, 128, "%a, %d %b %Y %H:%M:%S GMT", myTimeStruct);

        addHttpHeader(theKey, myBuffer);
    }

    string
    Request::urlEncode(const std::string & theUrl) {
        return string(curl_escape(theUrl.c_str(), 0));
    }

    string
    Request::urlDecode(const std::string & theUrl) {
        return string(curl_unescape(theUrl.c_str(), 0));
    }

    // request-method type methods
    size_t
    Request::post(const std::string & thePostData) {
        _myPostBuffer = thePostData;
        DB(AC_TRACE << "Posting Data '" << _myPostBuffer << "' size: " << _myPostBuffer.size() << endl);
        CURLcode myStatus = curl_easy_setopt(_myCurlHandle, CURLOPT_POST, true);
        checkCurlStatus(myStatus, PLUS_FILE_LINE);
        myStatus = curl_easy_setopt(_myCurlHandle, CURLOPT_POSTFIELDS, _myPostBuffer.c_str());
        checkCurlStatus(myStatus, PLUS_FILE_LINE);
        myStatus = curl_easy_setopt(_myCurlHandle, CURLOPT_POSTFIELDSIZE, _myPostBuffer.size());
        checkCurlStatus(myStatus, PLUS_FILE_LINE);
        return _myPostBuffer.size();
    }

    size_t
    Request::postBlock(const asl::Ptr<ReadableBlock> & theBlock) {
        _myPostBlock = theBlock;
        DB(AC_TRACE << "Posting Block with size=" << _myPostBlock->size() << endl);
        CURLcode myStatus = curl_easy_setopt(_myCurlHandle, CURLOPT_POST, true);
        checkCurlStatus(myStatus, PLUS_FILE_LINE);
        myStatus = curl_easy_setopt(_myCurlHandle, CURLOPT_POSTFIELDS, _myPostBlock->begin());
        checkCurlStatus(myStatus, PLUS_FILE_LINE);
        myStatus = curl_easy_setopt(_myCurlHandle, CURLOPT_POSTFIELDSIZE, _myPostBlock->size());
        checkCurlStatus(myStatus, PLUS_FILE_LINE);
        return _myPostBlock->size();
    }

    size_t
    Request::postFile(const std::string & theFilename) {
        DB(AC_TRACE << "Posting File '" << theFilename << endl);
        asl::Ptr<ReadableBlock> myFileBlock(new ConstMappedBlock(asl::expandEnvironment(theFilename)));
        return postBlock(myFileBlock);
    }

    void
    Request::get() {
        CURLcode myStatus = curl_easy_setopt(_myCurlHandle, CURLOPT_HTTPGET, true);
        checkCurlStatus(myStatus, PLUS_FILE_LINE);
    }

   void
    Request::head() {
        CURLcode myStatus = curl_easy_setopt(_myCurlHandle, CURLOPT_NOBODY, true);
        checkCurlStatus(myStatus, PLUS_FILE_LINE);
    }

    void
    Request::setSSLCertificate(const std::string & theCertificateFilename) {
        _mySSLCertificateFilename = theCertificateFilename;
        CURLcode myStatus = curl_easy_setopt(_myCurlHandle, CURLOPT_CAINFO, _mySSLCertificateFilename.c_str());
        checkCurlStatus(myStatus, PLUS_FILE_LINE);
    }

    void
    Request::verifyPeer(bool theFlag) {
        _myVerifyPeerFlag = theFlag;
        CURLcode myStatus = curl_easy_setopt(_myCurlHandle, CURLOPT_SSL_VERIFYPEER, theFlag);
        checkCurlStatus(myStatus, PLUS_FILE_LINE);
    }

    void 
    Request::setCookie(const std::string & theCookie) {
        if ( !_myCookieBuffer.empty()) {
            _myCookieBuffer += ";";
        }

        string::size_type myDelimit = theCookie.find(";");
        if (myDelimit == string::npos) {
            _myCookieBuffer += theCookie;
        } else {
            _myCookieBuffer += theCookie.substr(0, myDelimit);
        }

        CURLcode myStatus = curl_easy_setopt(_myCurlHandle, CURLOPT_COOKIE, _myCookieBuffer.c_str());
        checkCurlStatus(myStatus, PLUS_FILE_LINE);
    };

    void 
    Request::setCookies(const CookieJar & theCookies) {
        _myCookieBuffer = "";
        for (CookieJar::const_iterator i=theCookies.begin(); i != theCookies.end(); ++i) {
            setCookie(i->first + "=" + i->second);
        }
    }

    // virtual callback hooks
    size_t
    Request::onData(const char * theData, size_t theReceivedByteCount) {
        _myResponseBlock.append(theData, theReceivedByteCount);
        return theReceivedByteCount;
    }

    bool
    Request::onResponseHeader(const string & theHeader) {
        // divide into key/value pairs
        string::size_type myColon = theHeader.find(":");
        if (myColon != string::npos) {
            string myKey = theHeader.substr(0,myColon);
            string myValue = theHeader.substr(myColon+2);
            DB(AC_TRACE << "adding '" << myKey << "' / '" << myValue << "'" << endl);
            _myResponseHeaders.insert(make_pair(myKey, myValue));
        } else {
            DB(AC_TRACE << "adding '" << theHeader << "'" << endl);
            _myResponseHeaders.insert(make_pair(theHeader, ""));
        }
        return true;
    }

    void
    Request::onStart() {
    };

    void
    Request::onError(CURLcode theCode) {
        AC_DEBUG << "CURL error code: " << theCode;
    };

    bool
    Request::onProgress(double theDownloadTotal, double theCurrentDownload,
                double theUploadTotal, double theCurrentUpdate)
    {
        return true;
    };  // return false to abort transfer

    void
    Request::onDone() {
    };

    // //////////////////////////////////////////////////////////
    //
    // static callback dispatchers for curl
    //
    // //////////////////////////////////////////////////////////

    size_t
    Request::curlWriteCallback( void *theData, size_t theBlockCount, size_t theBlockSize, void *theRequestObject) {
        Request * myRequest = static_cast<Request*>(theRequestObject);
        return myRequest->onData(static_cast<const char *>(theData), theBlockSize * theBlockCount);
    }

    int
    Request::curlProgressCallback( void *theRequestObject, double theDownloadTotal,
                double theCurrentDownload, double theUploadTotal, double theCurrentUpload) {
        Request * myRequest = static_cast<Request*>(theRequestObject);
        bool myContinueFlag = myRequest->onProgress(theDownloadTotal, theCurrentDownload,
                    theUploadTotal, theCurrentUpload);

        // curl expects zero=continue, non-zero=abort
        return myContinueFlag ? 0 : 1;
    }

    size_t
    Request::curlHeaderCallback( void *theData, size_t theBlockCount, size_t theBlockSize, void *theRequestObject) {
        Request * myRequest = static_cast<Request*>(theRequestObject);
        string myHeader(static_cast<char*>(theData), theBlockSize*theBlockCount);
        // remove trailing newline
        string::size_type myEndOfLine = myHeader.find_first_of("\r\n");
        if (myEndOfLine != string::npos) {
            myHeader.erase(myEndOfLine);
        }
        if (myHeader.size()) {
            if (myRequest->onResponseHeader(myHeader) == false) {
                return (size_t) -1; // abort transfer
            }
        }
        return theBlockCount*theBlockSize;
    }
}


