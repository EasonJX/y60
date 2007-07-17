//============================================================================
// Copyright (C) 2007, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//============================================================================

#ifndef Y60_ASS_SERIAL_TRANSPORT_INCLUDED
#define Y60_ASS_SERIAL_TRANSPORT_INCLUDED

#include "TransportLayer.h"

#include <asl/SerialDevice.h>

namespace y60 {

class ASSDriver;

class SerialTransport : public TransportLayer {
    public:
        SerialTransport(ASSDriver * theDriver, const dom::NodePtr & theSettings);
        ~SerialTransport();

        void onUpdateSettings(dom::NodePtr theSettings);

    protected:
        void establishConnection();
        void readData();
        void closeConnection();
        void writeData(const char * theData, size_t theSize);
    private:

        asl::SerialDevice * _mySerialPort;
        bool _myUseUSBFlag; // used by the linux implementation, because
                            // USB TTYs have a diffrent naming scheme
        int  _myPortNum;
        int  _myBaudRate;
        int  _myBitsPerSerialWord;
        int  _myStopBits;
        bool _myHandshakingFlag;
        asl::SerialDevice::ParityMode _myParity;

        unsigned _myNumReceivedBytes;
        double _myLastComTestTime;

};

} // end of namespace

#endif // Y60_ASS_SERIAL_TRANSPORT_INCLUDED
