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
*/
#include "Client.h"
#include "../NetAsync.h"

#include <string>
#include <y60/jsbase/JScppUtils.h>
#include <y60/jsbase/JSBlock.h>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>

#include <netsrc/spidermonkey/jsapi.h>

#ifdef __APPLE__
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#include <openssl/sha.h>
#include <openssl/rand.h>

using namespace std;
using namespace asl;
using namespace jslib;
using boost::asio::ip::tcp;

namespace y60 {
namespace async {
namespace websocket {

#define Magic_UUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11" 
    Client::Client(JSContext * cx, JSObject * theOpts, boost::asio::io_service & io) :
        _jsContext(cx),
        _resolver(io),
        _socket(io),
        _writeStrand(io),
        _jsOptsObject(theOpts),
        _readyState(CONNECTING)
    {
        ostringstream s;
        s << "WebSocketClient::" << hex << this;
        jsval propValue;
        std::string theURL;

        JS_GetProperty(_jsContext, _jsOptsObject, "url", &propValue);
        if (JSVAL_IS_STRING(propValue)) {
            jslib::convertFrom(_jsContext, propValue, theURL);
        }
        s << " '" << theURL << "'";
        debugIdentifier = s.str();
        AC_TRACE << "creating client " << debugIdentifier;
       
        /*
        if(!JS_AddNamedRoot(_jsContext, &_jsOptsObject, debugIdentifier.c_str())) {
            AC_WARNING << "failed to root request object!";
        }
        */

        // parse the URI and start an async name resolver query
        string myProtocol, myLogin;
        asl::parseURI(theURL, &myProtocol, &myLogin, &_path);
        if (myProtocol != "ws") {
            throw asl::NotYetImplemented("Only ws:// supported for now", PLUS_FILE_LINE);
        };
        string::size_type myCredentialServerDelimiter = myLogin.find("@");
        if (myCredentialServerDelimiter != string::npos) {
            throw asl::NotYetImplemented("Authetification not yet supported for websockets", PLUS_FILE_LINE);
        };
        _hostport = myLogin;
        string::size_type myHostPortDelimiter = _hostport.find(":");
        if (myHostPortDelimiter == string::npos) {
            _host = _hostport;
            _port = "80";
        } else {
            _host = _hostport.substr(0, myHostPortDelimiter);
            _port = _hostport.substr(myHostPortDelimiter+1);
        }

        AC_TRACE << "Resolving " << _host << " on Port " << _port << "...";
        
        tcp::resolver::query query(_host, _port);
        _resolver.async_resolve(query,
            boost::bind(&Client::onResolved, this, 
                boost::asio::placeholders::error, 
                    boost::asio::placeholders::iterator));
    }

    void 
    Client::onResolved(const boost::system::error_code& error, tcp::resolver::iterator endpoint_iterator) {
        if (!error) {
            AC_TRACE << "resolved!" << endpoint_iterator->host_name() << ":" << endpoint_iterator->service_name();
            AC_TRACE << "now established TCP connection...";
            async_connect(_socket, endpoint_iterator,
                boost::bind(&Client::onTCPConnection, this,
                boost::asio::placeholders::error));
            // prepare the secure key
            //
            Unsigned8 nonce[16]; 
            RAND_bytes(nonce, sizeof(nonce));
            binToBase64(nonce, sizeof(nonce), _secWebSocketKey, cb64);
        } else {
             AC_ERROR << error.message();
             // TODO: call onError
        }
    }

    void 
    Client::onTCPConnection(const boost::system::error_code& error) {
        if (!error) {
            AC_TRACE << "TCP connection established. Sending WS Handshake...";
            std::ostream request_stream(&_send_buffer);
            request_stream << "GET " << _path << " HTTP/1.1\r\n";
            request_stream << "Host: " << _hostport << "\r\n";
            request_stream << "Upgrade: websocket\r\n";
            request_stream << "Connection: Upgrade\r\n";
            request_stream << "Sec-WebSocket-Key: " << _secWebSocketKey << "\r\n";
            request_stream << "Sec-WebSocket-Version: 13\r\n";
            request_stream << "\r\n";

            // send the request. Note: since we are still in half-duplex (http) mode,
            // we don't yet have to worry abound synchronization issues and strands.
            boost::asio::async_write(_socket, _send_buffer,
                boost::bind(&Client::onHeadersSent, this,
                boost::asio::placeholders::error));
        } else {
             AC_ERROR << error.message();
             // TODO: call onError
        }
    }
                
    void 
    Client::onHeadersSent(const boost::system::error_code& error) {
        if (!error) {
            AC_TRACE << "WS Handshake sent...waiting for response...";
            // Read the response headers, which are terminated by a blank line.
            boost::asio::async_read_until(_socket, _recv_buffer, "\r\n",
                boost::bind(&Client::onHeadersRead, this,
                boost::asio::placeholders::error));
        } else {
             AC_ERROR << error.message();
             // TODO: call onError
        }
    }

    void 
    Client::async_read_if_needed(std::size_t needed, void (Client::*contfunc)(const boost::system::error_code &, std::size_t)) { 
        if (_recv_buffer.size() < needed) {
            boost::asio::async_read(_socket, _recv_buffer, boost::asio::transfer_at_least(needed - _recv_buffer.size()), 
                  boost::bind(contfunc, this,
                  boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
        } else {
            (this->*contfunc)(boost::system::error_code(), 0);
        }
    }

    void 
    Client::onHeadersRead(const boost::system::error_code& error) {
        if (!error) {
            // parse the response headers.
            std::istream response_stream(&_recv_buffer);
            std::string header;
            int statusCode = -1;
            while (std::getline(response_stream, header) && header != "\x0d" && header != "\x0d\x0a") {
                /* enable this for hex dump
                for (int i = 0; i < header.size(); i++) {
                    std::cerr << hex << static_cast<unsigned int>(header[i]) << " ";
                }
                std::cerr << std::endl;
                */
                // first line should be status line
                if (statusCode == -1) {
                    boost::regex expr("HTTP/([^\\s]*)\\s([^\\s]*)\\s([\\w\\s]+)[\r\n]+");
                    boost::smatch what;
                    if (boost::regex_match(header, what, expr)) {
                        statusCode = as<int>(what[2]);
                        AC_TRACE << "Received HTTP status code " << statusCode << " from server";
                    }
                } else {
                    boost::regex expr("([^\\s]*): (.*)[\r\n]+");
                    boost::smatch what;
                    if (boost::regex_match(header, what, expr)) {
                        AC_TRACE << "Received Header: '" << what[1] << "' -> '" << what[2] << "'";
                        _replyHeaders.insert(make_pair(what[1], what[2]));
                    }
                }
            }
            // validate the connection
            if (statusCode != 101) {
                AC_ERROR << "Unsupported status code received from server: " << statusCode;
                // TODO: call onError, close connection
            }
            string concatenatedKey = _secWebSocketKey + Magic_UUID;
            Unsigned8 sha1_digest[SHA_DIGEST_LENGTH];
            SHA1(reinterpret_cast<const Unsigned8*>(concatenatedKey.c_str()),concatenatedKey.size(),sha1_digest);
            string expectedValue;
            binToBase64(sha1_digest, sizeof(sha1_digest), expectedValue, cb64);
            if (expectedValue != _replyHeaders["Sec-WebSocket-Accept"]) {
                AC_ERROR << "Expected " << expectedValue << " from Server, received " << _replyHeaders["Sec-WebSocket-Accept"];
            }
            _readyState = OPEN;
            AC_DEBUG << "WebSocket connection OPEN to " << debugIdentifier;
            AC_TRACE << _recv_buffer.size() << " bytes left in buffer after reading header";


            // Upgrade has now completed. Sending operations will be now be protected by the _writeStrand

            // wait for incoming frame. The header will be at least 2 bytes, possibly more.
            // We won't know how long the header will be until we have the first two bytes. 
            // boost::asio::async_read(_socket, _recv_buffer, boost::asio::transfer_at_least(2),
            async_read_if_needed(2, &Client::onFrameHeaderRead);

        } else {
             AC_ERROR << error.message();
             // TODO: call onError, close connection
        }
    }
   
#if 0
      0                   1                   2                   3
      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
     +-+-+-+-+-------+-+-------------+-------------------------------+
     |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
     |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
     |N|V|V|V|       |S|             |   (if payload len==126/127)   |
     | |1|2|3|       |K|             |                               |
     +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
     |     Extended payload length continued, if payload len == 127  |
     + - - - - - - - - - - - - - - - +-------------------------------+
     |                               |Masking-key, if MASK set to 1  |
     +-------------------------------+-------------------------------+
     | Masking-key (continued)       |          Payload Data         |
     +-------------------------------- - - - - - - - - - - - - - - - +
     :                     Payload Data continued ...                :
     + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
     |                     Payload Data continued ...                |
     +---------------------------------------------------------------+
#endif    
    void 
    Client::onFrameHeaderRead(const boost::system::error_code& error, std::size_t bytes_transferred) {
        if (!error) {
            AC_TRACE << "Transferred " << bytes_transferred << " bytes";
            AC_TRACE << "Buffer contains " << _recv_buffer.size() << " bytes";
            // assume we have at least two bytes. Calculate how long the header really is
            boost::asio::streambuf::const_buffers_type bufs = _recv_buffer.data();

            boost::asio::buffers_iterator<boost::asio::streambuf::const_buffers_type> header = boost::asio::buffers_begin(bufs);
            Unsigned8 firstByte = *header;
            _incomingFrame = FramePtr(new Frame(firstByte & 0x0f, firstByte & 0x80)); 
            Unsigned8 secondByte = header[1];
            Unsigned8 tempPayloadLength = (secondByte & 0x7f);
            
            _incomingFrame->masked = (secondByte & 0x80);
            int frameHeaderSize = 2 + (tempPayloadLength == 126? 2 : 0) + (tempPayloadLength == 127? 8 : 0) + (_incomingFrame->masked? 4 : 0);
            if (_recv_buffer.size() < frameHeaderSize) {
                AC_TRACE << "Need " << frameHeaderSize << " bytes, waiting for more.";
                async_read_if_needed(frameHeaderSize, &Client::onFrameHeaderRead);
                return;
            }
            // now calculate payload length
            int i;
            uint64_t payloadLength;
            if (tempPayloadLength < 126) {
                payloadLength = tempPayloadLength;
                i = 2;
            }
            else if (tempPayloadLength == 126) {
                payloadLength = 0;
                payloadLength |= ((uint64_t) (uint8_t)(header[2]) << 8);
                payloadLength |= ((uint64_t) (uint8_t)(header[3]) << 0);
                i = 4;
            }
            else if (tempPayloadLength == 127) {
                payloadLength = 0;
                payloadLength |= ((uint64_t) (uint8_t)(header[2]) << 56);
                payloadLength |= ((uint64_t) (uint8_t)(header[3]) << 48);
                payloadLength |= ((uint64_t) (uint8_t)(header[4]) << 40);
                payloadLength |= ((uint64_t) (uint8_t)(header[5]) << 32);
                payloadLength |= ((uint64_t) (uint8_t)(header[6]) << 24);
                payloadLength |= ((uint64_t) (uint8_t)(header[7]) << 16);
                payloadLength |= ((uint64_t) (uint8_t)(header[8]) << 8);
                payloadLength |= ((uint64_t) (uint8_t)(header[9]) << 0);
                i = 10;
            }
            /*
            if (ws.mask) {
                ws.masking_key[0] = ((uint8_t) header[i+0]) << 0;
                ws.masking_key[1] = ((uint8_t) header[i+1]) << 0;
                ws.masking_key[2] = ((uint8_t) header[i+2]) << 0;
                ws.masking_key[3] = ((uint8_t) header[i+3]) << 0;
            }
            else {
                ws.masking_key[0] = 0;
                ws.masking_key[1] = 0;
                ws.masking_key[2] = 0;
                ws.masking_key[3] = 0;
            */
            AC_DEBUG << "Payload FIN=" << _incomingFrame->final << ", opcode=" << static_cast<int>(_incomingFrame->opcode) << ", payloadLen=" << payloadLength << " i=" << i;
            _recv_buffer.consume(frameHeaderSize);
            AC_TRACE << _recv_buffer.size() << " bytes left in buffer after reading header";
            _incomingFrame->payload.resize(payloadLength);
            async_read_if_needed(payloadLength, &Client::onPayloadRead);

        } else {
            if (error == boost::asio::error::connection_reset && _readyState == CLOSING) {
                // this is not an error but a proper (clean) close
                AC_DEBUG << "Server closed connection";
                ScopeLocker L(_lockEventQueue, true);
                _readyState = CLOSED;
                _eventQueue.push_back(EventPtr(new CloseEvent(true, 1000, "TODO")));
            } else {
                AC_ERROR << error.message() << " transferred:" << bytes_transferred;
                // TODO: call onError, close connection
            }
        }
    }

    void 
    Client::onPayloadRead(const boost::system::error_code& error, std::size_t bytes_transferred) {
        if (!error) {
            AC_TRACE << "Transferred " << bytes_transferred << " bytes";
            AC_TRACE << "Receive Buffer contains " << _recv_buffer.size() << " bytes";
            // TODO do something with the frame
            switch (_incomingFrame->opcode) {
                case Frame::TEXT:
                    processMessageFrame();
                    break;
                case Frame::CONNECTION_CLOSE:
                    processCloseFrame();
                    break;
                default:
                    AC_WARNING << "Unsupported Frame Type " << static_cast<unsigned int>(_incomingFrame->opcode); 
            };
            _recv_buffer.consume(_incomingFrame->payload.size());
            // ...
            //
            //
            _incomingFrame.reset();
            if (_readyState != CLOSED) {
                // now wait for next frame header
                AC_TRACE << "waiting for next frame header";
                async_read_if_needed(2, &Client::onFrameHeaderRead);
            }
        } else {
             AC_ERROR << error.message();
             // TODO: call onError, close connection
        }
    }

    void
    Client::processMessageFrame() {
        if (_incomingMessage) {
            AC_ERROR << "TODO handle protocol error: received new message while fragment still in buffer";
            return;
        }
        boost::asio::streambuf::const_buffers_type bufs = _recv_buffer.data();
        _incomingMessage = MessagePtr(new TextMessage(boost::asio::buffers_begin(bufs), _incomingFrame->payload.size()));
        if (_incomingFrame->final) {
            processFinalFragment(); 
        }
    }

    void
    Client::processFinalFragment() {
        AC_DEBUG << "incoming Text Message " << boost::dynamic_pointer_cast<TextMessage>(_incomingMessage)->data.size() << " bytes";
        _eventQueue.push_back(EventPtr(new MessageEvent(_incomingMessage)));
        _incomingMessage.reset();
    }

    void
    Client::processCloseFrame() {
        if (_readyState != CLOSING) {
            _writeStrand.post(boost::bind(&Client::_w_confirmClose, this));
            // clear any queued frames
            _readyState = CLOSING;
        }
    }
    
    void
    Client::_w_confirmClose() {
        // clear any queued frames
        _w_sendQueue.empty();
        _w_injectFrame(FramePtr(new Frame(Frame::CONNECTION_CLOSE, true)));
    }

    void
    Client::_w_injectFrame(FramePtr f) {
        _w_sendQueue.push_front(f);
        AC_TRACE << "injecting control frame, queue length now " << _w_sendQueue.size();
        if (!_w_outgoingFrame) {
            AC_TRACE << "starting to send front frame";
            _w_sendNextFrame();
        }
    }

    void
    Client::_w_queueFrame(FramePtr f) {
        // TODO: message fragmentation
        _w_sendQueue.push_back(f);
        AC_TRACE << "queuing message frame, queue length now " << _w_sendQueue.size();
        if (!_w_outgoingFrame) {
            AC_TRACE << "starting to send front frame";
            _w_sendNextFrame();
        }
    }

    void
    Client::_w_sendNextFrame() {
        if (_w_outgoingFrame) {
            throw asl::Exception("trying to start sending while previous send operation still in progress");
        }
        _w_outgoingFrame = _w_sendQueue.front();
        _w_sendQueue.pop_front();
        // prepare outgoing frame for transmission
        
        // RFC: turn on masking for outgoing frames
        _w_outgoingFrame->masked = true;
        RAND_bytes(_w_outgoingFrame->masking_key, sizeof(_w_outgoingFrame->masking_key));

        std::ostream request_stream(&_send_buffer);
        // construct the header
        request_stream.put((_w_outgoingFrame->final? 0x80 : 0) + _w_outgoingFrame->opcode); 
        // variable length payload size field
        AC_DEBUG << "Frame Payload is " << _w_outgoingFrame->payload.size() << " bytes.";
        if (_w_outgoingFrame->payload.size() < 126ul) {
            request_stream.put((_w_outgoingFrame->masked? 0x80 : 0) + _w_outgoingFrame->payload.size()); 
        } else if (_w_outgoingFrame->payload.size() < 0x10000ul) {
            request_stream.put((_w_outgoingFrame->masked? 0x80 : 0) + 126);
            request_stream.put(_w_outgoingFrame->payload.size() >> 8); 
            request_stream.put(_w_outgoingFrame->payload.size() ); 
        } else {
            request_stream.put((_w_outgoingFrame->masked? 0x80 : 0) + 127);
            request_stream.put(_w_outgoingFrame->payload.size() >> 56); 
            request_stream.put(_w_outgoingFrame->payload.size() >> 48); 
            request_stream.put(_w_outgoingFrame->payload.size() >> 40); 
            request_stream.put(_w_outgoingFrame->payload.size() >> 32); 
            request_stream.put(_w_outgoingFrame->payload.size() >> 24); 
            request_stream.put(_w_outgoingFrame->payload.size() >> 16); 
            request_stream.put(_w_outgoingFrame->payload.size() >> 8); 
            request_stream.put(_w_outgoingFrame->payload.size() ); 
        }
        if (_w_outgoingFrame->masked) {
            request_stream.write(reinterpret_cast<const char*>(_w_outgoingFrame->masking_key), sizeof(_w_outgoingFrame->masking_key));
        }
        for (size_t i = 0; i < _w_outgoingFrame->payload.size(); ++i) {
            request_stream.put(_w_outgoingFrame->payload[i] ^ _w_outgoingFrame->masking_key[i % 4]);  
        }
        
        // send the request
        boost::asio::async_write(_socket, _send_buffer,
                _writeStrand.wrap(boost::bind(&Client::_w_onFrameSent, this,
                    boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)));
    }
    
    void 
    Client::_w_onFrameSent(const boost::system::error_code& error, size_t bytes_transferred) {
        if (!error) {
            _w_outgoingFrame.reset();
            if (!_w_sendQueue.empty()) {
                _w_sendNextFrame();
            }
        } else {
             AC_ERROR << error.message();
             // TODO: call onError
        }
    }

    void
    Client::processCallbacks() {
        EventPtr nextEvent;
        do {
            {
                ScopeLocker L(_lockEventQueue, true);
                if (_eventQueue.empty()) {
                    nextEvent.reset();
                } else {
                    nextEvent = _eventQueue.front();
                    _eventQueue.pop_front();
                }
            }
            if (nextEvent) {
                if (boost::shared_ptr<CloseEvent> e = boost::dynamic_pointer_cast<CloseEvent>(nextEvent)) {
                    if (hasCallback("onclose")) {
                        jsval argv[1], rval;
                        argv[0] = OBJECT_TO_JSVAL(e->newJSObject(_jsContext, _jsWrapper));
                        JSBool ok = JSA_CallFunctionName(_jsContext, _jsOptsObject, "onclose", 1, argv, &rval);
                    }
                } else if (boost::shared_ptr<MessageEvent> e = boost::dynamic_pointer_cast<MessageEvent>(nextEvent)) {
                    if (hasCallback("onmessage")) {
                        jsval argv[1], rval;
                        argv[0] = OBJECT_TO_JSVAL(e->newJSObject(_jsContext, _jsWrapper));
                        JSBool ok = JSA_CallFunctionName(_jsContext, _jsOptsObject, "onmessage", 1, argv, &rval);
                    }
                } else {
                    AC_WARNING << "Unknown event type - invoke JS Callback here";
                }
            }
        } while (nextEvent);
    }

    void
    Client::send(const std::string& data) {
        // TODO: implement fragmention
        FramePtr f = FramePtr(new Frame(Frame::TEXT, true));
        f->payload.resize(data.length());
        std::copy(data.begin(), data.end(), f->payload.begin());
        _writeStrand.post(boost::bind(&Client::_w_queueFrame, this, f));
    }

    void 
    Client::setWrapper(JSObject * theWrapper) {
        _jsWrapper = theWrapper;
        // add root to prevent garbage collection of client and its callbacks 
        if(!JS_AddNamedRoot(_jsContext, &_jsWrapper, debugIdentifier.c_str())) {
            AC_WARNING << "failed to root request object!";
        }
        asl::Ptr<NetAsync> parentPlugin = dynamic_cast_Ptr<NetAsync>(Singleton<PlugInManager>::get().getPlugIn(NetAsync::PluginName));
        parentPlugin->getWSManager().addClient(shared_from_this());
    }

    void
    Client::failTheWebSocketConnection() {
        // TODO: SHOULD send close frame if connected
        _socket.close();
    }

    Client::~Client()
    {
        AC_TRACE << "~Client " << this;
    }

    bool
    Client::hasCallback(const char * theName) {
        jsval myValue;
        if (JS_GetProperty(_jsContext, _jsOptsObject, theName, &myValue)) {
            if (JS_TypeOfValue(_jsContext, myValue) == JSTYPE_FUNCTION) {
                return true;
            }
        }
        return false;
    }

    jsval
    Client::getJSOption(const char * theName) const {
        jsval myValue;
        JS_GetProperty(_jsContext, _jsOptsObject, theName, &myValue);
        return myValue;
    }

    JSBool
    Client::setJSOption(const char * theName, jsval * vp) {
        return JS_SetProperty(_jsContext, _jsOptsObject, theName, vp); 
    };
}
}
}
