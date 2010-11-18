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
#include <asl/base/PlugInBase.h>
#include <y60/jsbase/JSScriptablePlugin.h>

using namespace std;
using namespace asl;

namespace jslib {
    class JSCSVImporter : public PlugInBase, public IScriptablePlugin {
    public:
        JSCSVImporter(asl::DLHandle theDLHandle) : asl::PlugInBase(theDLHandle) {}

        virtual ~JSCSVImporter() {};

        const char * ClassName() {
            static const char * myClassName = "CSVImporter";
            return myClassName;
        }
        
        JSFunctionSpec* Functions();

        vector<vector<string> > csv2array(string theFileName);
    private:
    };
};// jslib

#endif // _ac_CSVImporter_JSCSVImporter_h_
