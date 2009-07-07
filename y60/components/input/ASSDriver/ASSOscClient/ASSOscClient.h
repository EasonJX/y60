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
// Last Review: NEVER, NOONE
//
//  review status report: (perfect, ok, fair, poor, disaster, notapplicable, unknown)
//    usefullness            : unknown
//    formatting             : unknown
//    documentation          : unknown
//    test coverage          : unknown
//    names                  : unknown
//    style guide conformance: unknown
//    technical soundness    : unknown
//    dead code              : unknown
//    readability            : unknown
//    understandabilty       : unknown
//    interfaces             : unknown
//    confidence             : unknown
//    integration            : unknown
//    dependencies           : unknown
//    cheesyness             : unknown
//
//    overall review status  : unknown
//
//    recommendations: 
//       - unknown
// __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
*/

#ifndef ASS_EVENT_SOURCE_INCLUDED
#define ASS_EVENT_SOURCE_INCLUDED

#include "y60_oscclient_settings.h"

#ifdef AC_BUILT_WITH_CMAKE
#   include <y60/components/input/ASSDriver/ASSCore/ASSDriver.h>
#else
#   include <y60/ASSCore/ASSDriver.h>
#endif

#include <asl/net/UDPConnection.h>

#include <oscpack/osc/OscOutboundPacketStream.h>

namespace y60 {
    
    typedef asl::Ptr<osc::OutboundPacketStream, dom::ThreadingModel> OutboundPacketStreamPtr;
    
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
        void resetConnections();

        struct Receiver{
            Receiver(std::string theAddress, inet::UDPConnectionPtr theConnection):
                address(theAddress),
                udpConnection(theConnection){}
            std::string       address;
            inet::UDPConnectionPtr  udpConnection;
        };

        std::vector<OutboundPacketStreamPtr> _myOSCStreams;

        int                        _myClientPort;
        int                        _myServerPort;
        std::vector<Receiver>      _myReceivers;
        std::vector<asl::Vector2i> _myOscRegions;
        unsigned                   _myStreamIndex;
    };

} // end of namespace y60

#endif // ASS_EVENT_SOURCE_INCLUDED
