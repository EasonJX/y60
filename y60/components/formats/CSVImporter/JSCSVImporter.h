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
*/

#ifndef _ac_CSVImporter_JSCSVImporter_h_
#define _ac_CSVImporter_JSCSVImporter_h_

#include "y60_csvimporter_settings.h"

#include <string>
#include <vector>

#include <y60/jsbase/JSWrapper.h>
#include "CSVImporter.h"

#include <string>
#include <asl/base/Ptr.h>
#include <y60/jsbase/JSVector.h>
#include <y60/jslib/JSScene.h>
#include <y60/jsbase/JSNode.h>
#include <y60/jsbase/JSWrapper.impl>
#include <y60/jsbase/JSWrapper.h>
#include <y60/jsbase/JSScriptablePlugin.h>

using namespace std;
using namespace asl;

namespace jslib {
    namespace csv {
        class JSCSVImporter : public jslib::JSWrapper<CSVImporter,
                                CSVImporterPtr, jslib::StaticAccessProtocol> {
            JSCSVImporter();
        public:
            typedef CSVImporter NATIVE;
            typedef CSVImporterPtr OWNERPTR;
            typedef jslib::JSWrapper<NATIVE,OWNERPTR, jslib::StaticAccessProtocol> Base;

            JSCSVImporter(OWNERPTR theOwner, NATIVE * theNative): Base(theOwner, theNative) {}
            virtual ~JSCSVImporter() {};

            static const char * ClassName() {
                return "CSVImporter";
            }

            static JSFunctionSpec* Functions();
            static JSFunctionSpec* StaticFunctions();

            enum PropertyNumbers {
                /*    PROP_volume = -100,*/
            };

            static jslib::JSConstIntPropertySpec * ConstIntProperties();
            static JSPropertySpec * Properties();
            static JSPropertySpec * StaticProperties();

            virtual unsigned long length() const {
                return 1;
            }
            virtual JSBool getPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp);
            virtual JSBool setPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp);
            

            static JSObject * initClass(JSContext *cx, JSObject *theGlobalObject);
            static JSBool Constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
        private:
        };
    };//csv

    template <>
    struct JSClassTraits<csv::JSCSVImporter::NATIVE>
        : public JSClassTraitsWrapper<csv::JSCSVImporter::NATIVE, csv::JSCSVImporter> {};

    bool convertFrom(JSContext *cx, jsval theValue, jslib::csv::CSVImporterPtr& theCSVImporter);

    jsval as_jsval(JSContext *cx, jslib::csv::JSCSVImporter::OWNERPTR theOwner);
    jsval as_jsval(JSContext *cx, jslib::csv::JSCSVImporter::OWNERPTR theOwner, jslib::csv::JSCSVImporter::NATIVE * theNative);
};// jslib


#endif // _ac_CSVImporter_JSCSVImporter_h_
