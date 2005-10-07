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
//   $RCSfile: Request.h,v $
//   $Author: pavel $
//   $Revision: 1.16 $
//   $Date: 2005/03/07 20:32:55 $
//
//
//=============================================================================
#ifndef _Y60_INET_REQUEST_INCLUDED_
#define _Y60_INET_REQUEST_INCLUDED_

#ifdef WIN32
    #include <winsock2.h>
#endif

#include <asl/Exception.h>
#include <asl/Block.h>
#include <asl/Ptr.h>

#include <string>
#include <curl/curl.h>
#include <vector>
#include <map>

namespace inet {

    DEFINE_EXCEPTION(INetException, asl::Exception);

    class RequestManager;

    class Request {
        friend class RequestManager;

        public:
            Request(const std::string & theURL, const std::string & theUserAgent = "Art+Com Y60");
            virtual ~Request();
            CURL * getHandle() const;
            long getResponseCode() const;
            const std::string & getResponseString() const;
            std::string getErrorString() const;
            const std::string & getURL() const;
            void setLowSpeedLimit(unsigned theBytesPerSec);
            void setLowSpeedTimeout(unsigned theSeconds);
            void setTimeoutParams(unsigned theBytesPerSec, unsigned theSeconds);
            void setVerbose(bool theVerboseFlag);
            void setSSLCertificate(const std::string & theCertificateFilename);
            void setCookie(const std::string & theCookie);
            void setResume(long theResumeOffset);

            // request-method type methods
            size_t post(const std::string & thePostData);
            size_t postFile(const std::string & theFilename);
            size_t postBlock(const asl::Ptr<asl::ReadableBlock> & theBlock);
            void get();

            const std::multimap<std::string,std::string> & getResponseHeaders() const;
            std::string getResponseHeader(const std::string & theHeader) const;
            time_t getResponseHeaderAsDate(const std::string & theHeader) const;
            unsigned getLowSpeedLimit(void) const;
            unsigned getLowSpeedTimeout(void) const;

            void addHttpHeader(const std::string & theKey, const std::string & theValue);
            void addHttpHeaderAsDate(const std::string & theKey, time_t & theValue);

            //TODO: I'm not sure if this function is in the right place...
            static std::string urlEncode(const std::string & theUrl);
            static std::string urlDecode(const std::string & theUrl);

        protected:
            // callback hooks
            virtual void onStart();
            virtual size_t onData(const char * theData, size_t theLength);
            virtual void onError(CURLcode theCode);
            virtual bool onProgress(double theDownloadTotal, double theCurrentDownload,
                double theUploadTotal, double theCurrentUpload);  // return false to abort transfer
            virtual void onDone();
            virtual bool onResponseHeader(const std::string & theHeader);
        private:

            // static callback hooks
            static size_t curlWriteCallback( void *theData, size_t theBlockCount,
                                    size_t theBlockSize, void *theRequestObject);
            static int curlProgressCallback( void *theRequestObject, double theDownloadTotal,
                                    double theCurrentDownload, double theUploadTotal,
                                    double theCurrentUpdate);
            static size_t curlHeaderCallback( void *theData, size_t theBlockCount,
                                    size_t theBlockSize, void *theRequestObject);
            //
            Request();
            void checkCurlStatus(CURLcode theStatusCode, const std::string & theWhere) const ;
            std::string         _myURL;
            std::string         _myUserAgent;
            CURL *              _myCurlHandle;
            unsigned            _myLowSpeedLimit;
            unsigned            _myLowSpeedTimeout;
            struct curl_slist * _myHttpHeaderList;
            std::string         _myPostBuffer;
            asl::Ptr<asl::ReadableBlock> _myPostBlock;
            std::string         _myResponse;
            std::vector<char>   _myErrorBuffer;
            std::string         _mySSLCertificateFilename;
            std::multimap<std::string, std::string> _myResponseHeaders;
            std::vector<std::string> _myHttpHeaderBuffers;
            std::string         _myCookieBuffer;
    };

    typedef asl::Ptr<Request> RequestPtr;
    typedef asl::WeakPtr<Request> RequestWeakPtr;

}

#endif
