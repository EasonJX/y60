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

#ifndef _Y60_WIIMOTE_DRIVER_INCLUDED_
#define _Y60_WIIMOTE_DRIVER_INCLUDED_

#ifdef WIN32
#   include "Win32mote.h"
#elif defined( LINUX )
#   include "Liimote.h"
#elif defined( OSX )
// TODO
#endif

#include "WiiScanner.h"
#include "WiiEvent.h"

#include <asl/base/Ptr.h>
#include <asl/base/PlugInBase.h>
#include <asl/math/Vector234.h>
#include <asl/base/ThreadLock.h>
#include <y60/input/IEventSource.h>
#include <y60/jsbase/IScriptablePlugin.h>

#include <vector>
#include <queue>
#include <map>

namespace y60 {


class WiimoteDriver :
    public asl::PlugInBase,
    public jslib::IScriptablePlugin,
    public y60::IEventSource
{
    public:

        WiimoteDriver(asl::DLHandle theDLHandle);
        ~WiimoteDriver();
        virtual y60::EventPtrList poll();

        void requestStatusReport(const std::string & theId);
        void requestStatusReport();

        void setLEDs(const std::string & theId, bool led1, bool led2, bool led3, bool led4);
        void setLED(const std::string & theId, int theLEDIndex, bool theState);
        void setRumble(const std::string & theId, bool on);

        unsigned getNumWiimotes() const;

        void onSetProperty(const std::string & thePropertyName,
                const PropertyValue & thePropertyValue);
        void onGetProperty(const std::string & thePropertyName,
                PropertyValue & theReturnValue) const;
        void onUpdateSettings(dom::NodePtr theSettings);

        JSFunctionSpec * Functions();

        static JSBool SetRumble(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
        static JSBool SetLEDs(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
        static JSBool SetLED(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
        static JSBool RequestStatusReport(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

        const char * ClassName();

    private:
        WiiRemotePtr getWiiById( const std::string & theId );

        WiiReportMode _myDefaultReportMode;

        typedef std::map<std::string, WiiRemotePtr> DeviceMap;
        DeviceMap _myWiimotes;
        y60::WiiScanner                _myScanner;

        dom::NodePtr                    _myEventSchema;
        asl::Ptr<dom::ValueFactory>     _myValueFactory;
};

typedef asl::Ptr<WiimoteDriver> WiimotePtr;

}
#endif // _Y60_WIIMOTE_DRIVER_INCLUDED_

