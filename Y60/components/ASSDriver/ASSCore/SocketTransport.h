//============================================================================
// Copyright (C) 2007, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//============================================================================

#ifndef Y60_ASS_SOCKET_TRANSPORT_INCLUDED
#define Y60_ASS_SOCKET_TRANSPORT_INCLUDED

#include "TransportLayer.h"

#include <asl/TCPClientSocket.h>

namespace y60 {

class ASSDriver;

class SocketTransport : public TransportLayer {
    public:
        SocketTransport(const dom::NodePtr & theSettings);
        ~SocketTransport();


    protected:
        virtual bool settingsChanged(dom::NodePtr theSettings);
        void init(dom::NodePtr theSettings);

        void establishConnection();
        void readData();
        void closeConnection();
        void writeData(const char * theData, size_t theSize);
    private:

        //asl::SerialDevice * _mySerialPort;
        std::string  _myIpAddressString;
        unsigned int _myPort;
        inet::TCPClientSocketPtr  _mySocket;
        unsigned _myNumReceivedBytes;
};

} // end of namespace

#endif // Y60_ASS_SOCKET_TRANSPORT_INCLUDED
