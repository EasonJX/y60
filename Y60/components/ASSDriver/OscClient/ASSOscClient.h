//============================================================================
// Copyright (C) 2006, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//============================================================================

#ifndef ASS_EVENT_SOURCE_INCLUDED
#define ASS_EVENT_SOURCE_INCLUDED

#include <y60/ASSDriver.h>
#include <asl/UDPConnection.h>

#include <oscpack/osc/OscOutboundPacketStream.h>

namespace y60 {


    class ASSOscClient : public ASSDriver {
    public:
        ASSOscClient();
        void poll();


        const char * ClassName() {
            static const char * myClassName = "ASSOscClient";
            return myClassName;
        }

        enum {
            BUFFER_SIZE = 1024
        };

        virtual void onUpdateSettings( dom::NodePtr theSettings );

        void createTransportLayerEvent( const std::string & theType );
    protected:
        void createEvent( int theID, const std::string & theType,
                          const asl::Vector2f & theRawPosition, const asl::Vector3f & thePosition3D,
                          const asl::Box2f & theROI, float theIntensity,
                          const ASSEvent & theEvent);
    private:

        void connectToServer(int theIndex);

        struct Receiver{
            Receiver(std::string theAddress, inet::UDPConnectionPtr theConnection):
                address(theAddress),
                udpConnection(theConnection){}
            std::string       address;
            inet::UDPConnectionPtr  udpConnection;
        };

        char myBuffer[ BUFFER_SIZE ];
        osc::OutboundPacketStream _myOSCStream;

        int                      _myClientPort;
        int                      _myServerPort;
        std::vector<Receiver>    _myReceivers;
    };

} // end of namespace y60

#endif // ASS_EVENT_SOURCE_INCLUDED
