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
//
// Description: TODO  
//
*/

#include "CurlMultiAdapter.h"
#include "HttpClient.h"
#include "NetAsync.h"

#include <asl/base/Logger.h>

using namespace asl;

namespace y60 {
namespace async {
namespace http {
        


CurlMultiAdapter::CurlMultiAdapter(boost::asio::io_service & theIOService) :
    io(theIOService)
{
    _curlMulti = curl_multi_init(); 
    CURLMcode myStatus = curl_multi_setopt(_curlMulti, CURLMOPT_SOCKETFUNCTION, &CurlMultiAdapter::curl_socket_callback);
    checkCurlStatus(myStatus, PLUS_FILE_LINE);
    myStatus = curl_multi_setopt(_curlMulti, CURLMOPT_SOCKETDATA, this); 
    checkCurlStatus(myStatus, PLUS_FILE_LINE);

    myStatus = curl_multi_setopt(_curlMulti, CURLMOPT_TIMERFUNCTION, &CurlMultiAdapter::curl_timer_callback);
    checkCurlStatus(myStatus, PLUS_FILE_LINE);
    myStatus = curl_multi_setopt(_curlMulti, CURLMOPT_TIMERDATA, this); 
    checkCurlStatus(myStatus, PLUS_FILE_LINE);

};
CurlMultiAdapter::~CurlMultiAdapter() {
    AC_TRACE << "~CurlMultiAdapter done";
};

void
CurlMultiAdapter::shutdown() {
    AC_TRACE << "CurlMultiAdapter::shutdown";
    
    while (!_allClients.empty()) {
        (*_allClients.begin())->onDone(this, CURLE_ABORTED_BY_CALLBACK );
    }
    
    CurlSocketInfo::abort();
    CURLMcode myStatus = curl_multi_cleanup(_curlMulti); 
    checkCurlStatus(myStatus, PLUS_FILE_LINE);
    if (timeout_timer) {
        timeout_timer->cancel();
    }
    AC_TRACE << "CurlMultiAdapter::shutdown done";
};

void
CurlMultiAdapter::checkCurlStatus(CURLMcode theStatusCode, const std::string & theWhere) {
    if (theStatusCode != CURLM_OK) {
        throw asl::Exception(curl_multi_strerror(theStatusCode), theWhere);
    }
};

void 
CurlMultiAdapter::addClient(boost::shared_ptr<async::http::Client> theClient) { 
    AC_DEBUG << "adding client " << theClient;
    CURLMcode myStatus = curl_multi_add_handle(_curlMulti,  theClient->_curlHandle);
    checkCurlStatus(myStatus, PLUS_FILE_LINE);
    _allClients.insert(theClient);
    //int i;
    //myStatus = curl_multi_socket_action(_curlMulti, 0, 0, &i);
    //checkCurlStatus(myStatus, PLUS_FILE_LINE);
};
void 
CurlMultiAdapter::removeClient(boost::shared_ptr<async::http::Client> theClient ){ 
    AC_DEBUG << "removeClient client " << theClient;
    CURLMcode myStatus = curl_multi_remove_handle(_curlMulti,  theClient->_curlHandle); 
    checkCurlStatus(myStatus, PLUS_FILE_LINE);
    _allClients.erase(theClient);
};


int 
CurlMultiAdapter::curl_socket_callback(CURL *easy, /* easy handle */   
                               curl_socket_t theCurlSocket, /* socket */   
                               int action, /* see values below */   
                               void *userp, /* private callback pointer */   
                               void *socketp) /* private socket pointer */ 
{
    async::http::Client * curClient = 0;
    curl_easy_getinfo(easy, CURLINFO_PRIVATE, &curClient);
    AC_DEBUG << "Curl Socket "<< theCurlSocket << " Callback: " << action << " on " << userp << "," << curClient;
    CurlSocketInfo::Ptr s = CurlSocketInfo::find(theCurlSocket);
    if (s) {
        s->readyState = action;
        CurlSocketInfo::handleOperations(s, theCurlSocket);
    }
    return 0;
};

int 
CurlMultiAdapter::curl_timer_callback(CURLM *multi,  long timeout_ms, CurlMultiAdapter * self) {
    AC_TRACE << "multi_timer_cb: Setting timeout to " << timeout_ms << " ms";
    if ( ! self->timeout_timer) {
        self->timeout_timer = boost::shared_ptr<boost::asio::deadline_timer>(new boost::asio::deadline_timer(self->io)); 
    }
    self->timeout_timer->expires_from_now(boost::posix_time::milliseconds(timeout_ms));
    // self->onTimeout(boost::asio::error::operation_aborted);
    self->timeout_timer->async_wait(bind(&CurlMultiAdapter::onTimeout, self, boost::asio::placeholders::error));
    return 0;
};

void
CurlMultiAdapter::onTimeout(const boost::system::error_code& error) {
    AC_TRACE << "onTimeout " << error;
    if (error != 0) {
        return;
    }
    int i;
    CURLMcode myStatus = curl_multi_socket_action(_curlMulti, CURL_SOCKET_TIMEOUT, 0, &i);
    checkCurlStatus(myStatus, PLUS_FILE_LINE);
};

void
CurlMultiAdapter::processCallbacks() {
    for (std::set<boost::shared_ptr<Client> >::iterator it = _allClients.begin(); it != _allClients.end(); ++it) {
        (*it)->onProgress();
    }
};

void 
CurlMultiAdapter::setSocketInfo(curl_socket_t s, void * data) {
    CURLMcode myStatus = curl_multi_assign(_curlMulti, s, data);
    checkCurlStatus(myStatus, PLUS_FILE_LINE);
};

void
CurlMultiAdapter::processCompleted() {
    // take care of completed requests
    int myMessageCount = 0;
    CURLMsg * myMessage = 0;
    do {
        myMessage = curl_multi_info_read(_curlMulti, &myMessageCount);
        if (myMessage) {
            CURL * myEasyHandle = myMessage->easy_handle;
            async::http::Client * curClient = 0;
            curl_easy_getinfo(myEasyHandle, CURLINFO_PRIVATE, &curClient);
            if (myMessage->msg == CURLMSG_DONE) {
                AC_DEBUG << "calling onDone for " << curClient;
                curClient->onDone(this, myMessage->data.result);
            } else {
                throw asl::Exception("Unknown CURL message encountered");
            }
        }
    } while (myMessage);
};


}
}
}
