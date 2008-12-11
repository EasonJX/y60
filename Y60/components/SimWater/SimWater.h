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

#ifndef Y60_SIM_WATER_INCLUDED
#define Y60_SIM_WATER_INCLUDED

#include <y60/glutil/GLUtils.h>

#include "WaterSimulation.h"
#include "WaterRepresentation.h"

#include <asl/base/PlugInBase.h>
#include <y60/jslib/IRendererExtension.h>
#include <y60/jsbase/JSScriptablePlugin.h>
#include <y60/jsbase/Documentation.h>

class SimWater :
    public asl::PlugInBase,
    public y60::IRendererExtension,
    public jslib::IScriptablePlugin
{
    public:
        SimWater(asl::DLHandle theDLHandle);
        virtual ~SimWater(); 

        JSFunctionSpec * Functions();

        void onUpdateSettings(dom::NodePtr theConfiguration);
        void onGetProperty(const std::string & thePropertyName,
                y60::PropertyValue & theReturnValue) const;

        void onSetProperty(const std::string & thePropertyName,
                const y60::PropertyValue & thePropertyValue);

        void onStartup(jslib::AbstractRenderWindow * theWindow);
        bool onSceneLoaded(jslib::AbstractRenderWindow * theWindow);
        void handle(jslib::AbstractRenderWindow * theWindow, y60::EventPtr theEvent);
        void onFrame(jslib::AbstractRenderWindow * theWindow , double t);

        void onPreRender(jslib::AbstractRenderWindow * theRenderer);
        void onPostRender(jslib::AbstractRenderWindow * theRenderer);

        const char * ClassName();
        
        void splash(const asl::Vector2i & thePosition, float theMagnitude, int theRadius);
        int addFloormap(const std::string & theFilename);
        int addCubemap(const std::string theFilenames[]);
        void reset();

    private:
        SimWater();
        void loadTexturesFromConfig(const dom::Node & theConfig,
                                    y60::WaterRepresentation::TextureClass theClassID);
        void setWaterProjection();

        asl::Vector2i convertMouseCoordsToSimulation( const asl::Vector2i & theMousePos );

        y60::WaterSimulationPtr     _myWaterSimulation;
        y60::WaterRepresentationPtr _myWaterRepresentation;

        asl::Vector2i _mySimulationSize;
        asl::Vector2i _mySimulationOffset;
        asl::Vector2i _myDisplaySize;
        asl::Vector2i _myDisplayOffset;
        asl::Vector2i _myViewportSize;

        //float _myWaterDamping;

        asl::Time _myStartTime;
        bool _myRunSimulationFlag;
        int _myIntegrationsPerFrame;
        float _myTimeStep;

        int _myFloormapCounter;    
        int _myCubemapCounter;    

        bool _isRunning;
};
#endif // Y60_SIM_WATER_INCLUDED

