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

#include "World.h"

#include <asl/xpath/xpath_api.h>

namespace y60 {

    void
    World::updateLights() {
        _myLights.clear();

        dom::Node & myWorldNode = this->getNode();

        std::vector<LightPtr> myAllLights = myWorldNode.getAllFacades<Light>(LIGHT_NODE_NAME);

        for(unsigned i = 0; i < myAllLights.size(); i++) {
            LightPtr myLight = myAllLights[i];
            dom::Node * myNode = &myLight->getNode();
            while(myNode->parentNode()) {
                if(myNode->parentNode() == &myWorldNode) {
                    _myLights.push_back(myLight);
                    break;
                }
                myNode = myNode->parentNode();
            }
        }
    }

}
