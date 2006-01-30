//============================================================================
//
// Copyright (C) 2000-2001, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//============================================================================
//
//    $RCSfile: EdgeBlender.cpp,v $
//
//     $Author: pavel $
//
//   $Revision: 1.15 $
//
// Description:
//
//=============================================================================

#include <y60/JSNode.h>
#include <asl/PlugInBase.h>
#include <y60/IRendererExtension.h>
#include <y60/IScriptablePlugin.h>
#include <y60/JSScriptablePlugin.h>
#include <y60/AbstractRenderWindow.h>
#include <y60/Scene.h>
#include <y60/Movie.h>
#include <y60/OffscreenBuffer.h>

#include <asl/string_functions.h>
#include <asl/numeric_functions.h>
#include <asl/Exception.h>
#include <asl/Logger.h>

#include <iostream>

using namespace std;
using namespace asl;
using namespace jslib;
using namespace dom;
using namespace y60;


class EdgeBlender :
    public asl::PlugInBase,
    public y60::IRendererExtension,
    public IScriptablePlugin
{
    public:

        EdgeBlender(asl::DLHandle theDLHandle);

        void onStartup(jslib::AbstractRenderWindow * theWindow);

        bool onSceneLoaded(jslib::AbstractRenderWindow * theWindow);

        void handle(AbstractRenderWindow * theWindow, y60::EventPtr theEvent);
        void onFrame(AbstractRenderWindow * theWindow , double t);

        void onGetProperty(const std::string & thePropertyName, 
                PropertyValue & theReturnValue) const;
        void onSetProperty(const std::string & thePropertyName, 
                const PropertyValue & thePropertyValue);

        void onPreRender(AbstractRenderWindow * theRenderer);
        void onPostRender(AbstractRenderWindow * theRenderer);

        const char * ClassName() {
            static const char * myClassName = "EdgeBlender";
            return myClassName;
        }

        void onUpdateSettings(dom::NodePtr theSettings);

    private:
        float getBlendValue(float theValue);
        void drawBlackLevel();
        void rotateTo(unsigned theEdge);
        void drawBlendedEdge(float theStart, float theEnd, float theMargin = 0.0f);
        void drawBlendedCorner();
        void renderTexture(float theStartX, float theStartY, 
                           float theEndX, float theEndY,
                           float thePosX, float thePosY);
        void copyToTexture();
        void renderBlending();
        void preRender();
        void postRender();
        
        void renderDualScreen();
        void renderMultiPC();

        unsigned _mySceneWidth, _mySceneHeight;
        GLuint _mySceneTexture;
        
        const static unsigned BLEND_EDGE_LEFT  = 1;
        const static unsigned BLEND_EDGE_RIGHT = 2;
        const static unsigned BLEND_EDGE_UP    = 4;
        const static unsigned BLEND_EDGE_DOWN  = 8;
/*        
  enum FruitEnum {
 *      APPLE,
 *      CHERRY,
 *      BANANA,
 *      PASSION_FRUIT,
 *      FruitEnum_MAX
 * };
 *
 * static const char * myFruitStrings[] = {
 *      "apple",
 *      "cherry",
 *      "banana",
 *      "passion_fruit",
 *      ""
 * };
 *
*/       
        enum { MODE_MULTI_PC               = 1,
               MODE_DUAL_SCREEN_HORIZONTAL = 2,
               MODE_DUAL_SCREEN_VERTICAL   = 3
        };

        unsigned _myMode;
        unsigned  _myEdges;
        float _myBlendWidth;
        unsigned _myNumSubdivisions;

        float _myBlackLevel;
        float _myPower;
        float _myGamma;

};

EdgeBlender :: EdgeBlender(asl::DLHandle theDLHandle) :
    asl::PlugInBase(theDLHandle),
    IRendererExtension("EdgeBlender")
{}

void
EdgeBlender::onPostRender(AbstractRenderWindow * theRenderer) {
    AC_TRACE << "onPostRender";
    switch (_myMode) {
        case MODE_DUAL_SCREEN_HORIZONTAL:
        case MODE_DUAL_SCREEN_VERTICAL:
            renderDualScreen();
        break;
        case MODE_MULTI_PC:
            renderMultiPC();
        break;            
    }        
}

void
EdgeBlender::onStartup(jslib::AbstractRenderWindow * theWindow) {
    _myMode = MODE_DUAL_SCREEN_HORIZONTAL;
    _myEdges = 15; //BLEND_EDGE_LEFT;
    _myBlendWidth = 0.2f;
    _myNumSubdivisions = 32;
    _myBlackLevel = 0.1f;
    _myPower = 1.5f;
    _myGamma = 1.2f;
    
    //setup texture for rendering
    _mySceneWidth = theWindow->getWidth();
    _mySceneHeight = theWindow->getHeight();
    int mySceneTextureWidth = nextPowerOfTwo(_mySceneWidth);
    int mySceneTextureHeight = nextPowerOfTwo(_mySceneHeight);
    
    glGenTextures(1, &_mySceneTexture);
    glBindTexture (GL_TEXTURE_2D, _mySceneTexture );
    glTexImage2D(GL_TEXTURE_2D, 0, 3, mySceneTextureWidth, mySceneTextureHeight, 
                 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);	
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    checkOGLError(PLUS_FILE_LINE);
    AC_INFO << "bind texture " << _mySceneTexture << " " 
            << _mySceneWidth << "x" << _mySceneHeight;
}

void
EdgeBlender::onGetProperty(const std::string & thePropertyName,
        PropertyValue & theReturnValue) const
{
    AC_DEBUG << "onGetProperty " << thePropertyName;
    if (thePropertyName == "edges") {
        theReturnValue.set<unsigned>(_myEdges);
    } else if (thePropertyName == "blendwidth") {
        theReturnValue.set<float>(_myBlendWidth);
    } else if (thePropertyName == "subdivisions") {
        theReturnValue.set<unsigned>(_myNumSubdivisions);
    } else if (thePropertyName == "blacklevel") {
        theReturnValue.set<float>(_myBlackLevel);
    } else if (thePropertyName == "power") {
        theReturnValue.set<float>(_myPower);
    } else if (thePropertyName == "gamma") {
        theReturnValue.set<float>(_myGamma);
    }
}

void
EdgeBlender::onSetProperty(const std::string & thePropertyName,
        const PropertyValue & thePropertyValue)
{
    AC_DEBUG << "onSetProperty " << thePropertyName;
    if (thePropertyName == "edges") {
        _myEdges = thePropertyValue.get<unsigned>();
    } else if (thePropertyName == "blendwidth") {
        _myBlendWidth = thePropertyValue.get<float>();
    } else if (thePropertyName == "subdivisions") {
        _myNumSubdivisions = thePropertyValue.get<unsigned>();
    } else if (thePropertyName == "blacklevel") {
        _myBlackLevel = thePropertyValue.get<float>();
    } else if (thePropertyName == "power") {
        _myPower = thePropertyValue.get<float>();
    } else if (thePropertyName == "gamma") {
        _myGamma = thePropertyValue.get<float>();
    }
}

void 
EdgeBlender::onUpdateSettings(dom::NodePtr theSettings) {
    AC_DEBUG << "onUpdateSettings " << *theSettings;
    _myMode = getSetting(theSettings, "mode", _myMode);
    _myEdges = getSetting(theSettings, "edges", _myEdges);
    _myBlendWidth = getSetting(theSettings, "blendwidth", _myBlendWidth);
    _myNumSubdivisions = getSetting(theSettings, "subdivisions", _myNumSubdivisions);
    _myBlackLevel = getSetting(theSettings, "blacklevel", _myBlackLevel);
    _myPower = getSetting(theSettings, "power", _myPower);
    _myGamma = getSetting(theSettings, "gamma", _myGamma);
    
}

bool
EdgeBlender::onSceneLoaded(jslib::AbstractRenderWindow * theWindow) {
    AC_DEBUG << "onSceneLoaded";
    return true;
}

void
EdgeBlender::handle(AbstractRenderWindow * theWindow, y60::EventPtr theEvent) {
}

void
EdgeBlender::onFrame(AbstractRenderWindow * theWindow , double t) {
    AC_TRACE << "onFrame";
}

void
EdgeBlender::onPreRender(AbstractRenderWindow * theRenderer) {
    AC_TRACE << "onPreRender";
}

void 
EdgeBlender::renderDualScreen() {
    preRender();
    copyToTexture();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (_myMode == MODE_DUAL_SCREEN_HORIZONTAL) {
        renderTexture(0.0f, 0.0f, 0.5f + _myBlendWidth/2, 1.0f, -_myBlendWidth/2, 0.0f);
        renderTexture(0.5f - _myBlendWidth/2, 0.0f, 1.0f, 1.0f, 0.5f, 0.0f);
        rotateTo(BLEND_EDGE_RIGHT);
        drawBlendedEdge(0.0f, 1.0f, 0.5f); 
        rotateTo(BLEND_EDGE_LEFT);
        drawBlendedEdge(0.0f, 1.0f, 0.5f);

    } else if (_myMode == MODE_DUAL_SCREEN_VERTICAL) {        
        renderTexture(0.0f, 0.0f, 1.0f, 0.5f + _myBlendWidth/2, 0.0f, 0.0f);
        renderTexture(0.0f, 0.5f - _myBlendWidth/2, 1.0f, 1.0f, 0.0f, 0.5f);
        rotateTo(BLEND_EDGE_UP);
        drawBlendedEdge(0.0f, 1.0f, 0.5f);
        rotateTo(BLEND_EDGE_DOWN);
        drawBlendedEdge(0.0f, 1.0f, 0.5f);
    }        
    
    postRender();
}

void 
EdgeBlender::renderMultiPC() {
    preRender();
    renderBlending();
    postRender();
}
    
void 
EdgeBlender::renderBlending() {
    glDisable(GL_TEXTURE_2D);

    //blended edges
    if (_myEdges & BLEND_EDGE_LEFT) {        
        float myStart = (_myEdges & BLEND_EDGE_UP) ?
            _myBlendWidth : 0.0f;
        float myEnd = (_myEdges & BLEND_EDGE_DOWN) ?
            1.0f - _myBlendWidth : 1.0f;
        rotateTo(BLEND_EDGE_LEFT);
        drawBlendedEdge(myStart, myEnd);
    }        
    if (_myEdges & BLEND_EDGE_RIGHT) {
        float myStart = (_myEdges & BLEND_EDGE_DOWN) ?
            _myBlendWidth : 0.0f;
        float myEnd = (_myEdges & BLEND_EDGE_UP) ?
            1.0f - _myBlendWidth : 1.0f;
        rotateTo(BLEND_EDGE_RIGHT);
        drawBlendedEdge(myStart, myEnd);
    }        
    if (_myEdges & BLEND_EDGE_UP) {
        float myStart = (_myEdges & BLEND_EDGE_RIGHT) ?
            _myBlendWidth : 0.0f;
        float myEnd = (_myEdges & BLEND_EDGE_LEFT) ?
            1.0f - _myBlendWidth : 1.0f;
        rotateTo(BLEND_EDGE_UP);
        drawBlendedEdge(myStart, myEnd);
    }        
    if (_myEdges & BLEND_EDGE_DOWN) {
        float myStart = (_myEdges & BLEND_EDGE_LEFT) ?
            _myBlendWidth : 0.0f;
        float myEnd = (_myEdges & BLEND_EDGE_RIGHT) ?
            1.0f - _myBlendWidth : 1.0f;
        rotateTo(BLEND_EDGE_DOWN);
        drawBlendedEdge(myStart, myEnd);
    }        

    //corners
    if (_myEdges & BLEND_EDGE_LEFT && _myEdges & BLEND_EDGE_UP) {
        rotateTo(BLEND_EDGE_LEFT);
        drawBlendedCorner(); 
    }
    if (_myEdges & BLEND_EDGE_UP && _myEdges & BLEND_EDGE_RIGHT) {
        rotateTo(BLEND_EDGE_UP);
        drawBlendedCorner(); 
    }
    if (_myEdges & BLEND_EDGE_RIGHT && _myEdges & BLEND_EDGE_DOWN) {
        rotateTo(BLEND_EDGE_RIGHT);
        drawBlendedCorner(); 
    }
    if (_myEdges & BLEND_EDGE_DOWN && _myEdges & BLEND_EDGE_LEFT) {
        rotateTo(BLEND_EDGE_DOWN);
        drawBlendedCorner(); 
    }

    
    //blacklevel
    drawBlackLevel();
    
}

void 
EdgeBlender::preRender() {
    // load ortho projection
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0f, 1.0f, 1.0f, 0.0f);

    //load texture matrix
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glLoadIdentity();
    
    // load modelview
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
 
    // load attribs
    //glPushAttrib(CURRENT_BIT | GL_ENABLE_BIT);
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    glDisable(GL_LIGHTING); 
    glDisable(GL_DEPTH_TEST);
}

void 
EdgeBlender::postRender() {
    //restore attribs
    glPopAttrib();

    // restore modelview
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
    
    //put into model view
    glMatrixMode(GL_MODELVIEW);
}

float 
EdgeBlender::getBlendValue(float theValue) {
    float myBlendValue;
    // blend value
    if (theValue < 0.5f) {
        myBlendValue = 0.5f * powf(2.0f * theValue, _myPower);
    } else {
        myBlendValue = 1.0f - 0.5f * powf(2.0f * (1.0f - theValue), _myPower);
    }
    
    // gamma correction
    if (_myGamma > 0.0f) {
        myBlendValue = powf(myBlendValue, 1.0f / _myGamma);
    }

    return myBlendValue;
}

void
EdgeBlender::drawBlackLevel() {
    float myLeft = _myEdges & BLEND_EDGE_LEFT ? _myBlendWidth : 0.0f;
    float myRight = _myEdges & BLEND_EDGE_RIGHT ? 1.0f - _myBlendWidth : 1.0f;
    float myUp = _myEdges & BLEND_EDGE_UP ? _myBlendWidth : 0.0f;
    float myDown = _myEdges & BLEND_EDGE_DOWN ? 1.0f - _myBlendWidth : 1.0f;
    glLoadIdentity();
    glBegin(GL_TRIANGLE_STRIP);
    glColor4f(1.0f, 1.0f, 1.0f, _myBlackLevel);
    glVertex2f(myLeft, myUp);
    glVertex2f(myLeft, myDown);
    glVertex2f(myRight, myUp);
    glVertex2f(myRight, myDown);
    glEnd();
}

void
EdgeBlender::rotateTo(unsigned theEdge) {
    float myAngle = 0.0f;
    switch (theEdge) {
        case BLEND_EDGE_LEFT:
            myAngle = 0.0f;
            break;
        case BLEND_EDGE_RIGHT:
            myAngle = 180.0f;
            break;
        case BLEND_EDGE_UP:
            myAngle = 90.0f;
            break;
        case BLEND_EDGE_DOWN:
            myAngle = 270.0f;
            break;
        default:
            AC_WARNING << "theEdge " << theEdge << " unknown.";
    }

    //rotate to the edge
    glLoadIdentity();
    glTranslatef(0.5f, 0.5f, 0.0f);
    glRotatef(myAngle, 0.0f, 0.0f, 1.0f);
    glTranslatef(-0.5f, -0.5f, 0.0f);
}

void 
EdgeBlender::drawBlendedEdge(float theStart, float theEnd, float theMargin) {
    AC_DEBUG << "drawBlendedEdge start " << theStart << " end " << theEnd;

    float myDelta = 1.0f / _myNumSubdivisions;

    glBegin(GL_TRIANGLE_STRIP);
    for (unsigned i = 0; i <= _myNumSubdivisions; ++i) {
        float myX = i * myDelta;

        float myBlendValue = getBlendValue(myX);

        glColor4f(0.0f, 0.0f, 0.0f, 1.0f - myBlendValue);
        //glColor4f(1.0f, 1.0f, 0.0f, 1.0f); // - myBlendValue);

        float myXPos = theMargin + myX * _myBlendWidth;
        glVertex2f(myXPos, theStart);
        glVertex2f(myXPos, theEnd);
        
        AC_DEBUG << "x " << myX << " blend=" << myBlendValue;

    }
    glEnd();
    
}

void 
EdgeBlender::drawBlendedCorner() {

    float myDelta = 1.0f / _myNumSubdivisions;
    
    for (unsigned i = 0; i < _myNumSubdivisions; ++i) {
        float myY = i * myDelta;
        float myY1BlendValue = getBlendValue( myY);
        float myY2BlendValue = getBlendValue( myY + myDelta);

        glBegin(GL_TRIANGLE_STRIP);
        for (unsigned j = 0; j <= _myNumSubdivisions; ++j) {
            float myX = j * myDelta;
            float myXBlendValue = getBlendValue( myX);

            float myXPos = myX * _myBlendWidth;
            float myYPos = myY * _myBlendWidth;

            float myBlendValue = myXBlendValue * myY1BlendValue;
            glColor4f(0.0f, 0.0f, 0.0f, 1.0f - myBlendValue);
            glVertex2f(myXPos, myYPos);
            
            myBlendValue = myXBlendValue * myY2BlendValue;
            glColor4f(0.0f, 0.0f, 0.0f, 1.0f - myBlendValue);
            glVertex2f(myXPos, myYPos + _myBlendWidth / _myNumSubdivisions);

            AC_DEBUG << "x " << myX << "y " << myY << " blend=" << myBlendValue;
        }
        glEnd();
    }

}

void 
EdgeBlender::copyToTexture() {
    
    glEnable(GL_TEXTURE_2D);

    glBindTexture (GL_TEXTURE_2D, _mySceneTexture );
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, _mySceneWidth, _mySceneHeight, 0);

    CHECK_OGL_ERROR;
}

void 
EdgeBlender::renderTexture(float theStartX, float theStartY, float theEndX, float theEndY,
                           float thePosX, float thePosY) 
{
    float myZ = -0.5f; //behind the blending    
    float myWidth = theEndX - theStartX;
    float myHeight = theEndY - theStartY;
    //glColor4f(1.0f, 1.0f, 0.0f, 0.5f);
    glBegin(GL_QUADS);
    glTexCoord2f(theStartX, 1.0f - theStartY); 
    glVertex3f(thePosX, thePosY, myZ);
    
    glTexCoord2f(theStartX, 1.0f - theEndY); 
    glVertex3f(thePosX, thePosY + myHeight, myZ);
    
    glTexCoord2f(theEndX, 1.0f - theEndY); 
    glVertex3f(thePosX + myWidth, thePosY + myHeight, myZ);
    
    glTexCoord2f(theEndX, 1.0f - theStartY); 
    glVertex3f(thePosX + myWidth, thePosY, myZ);
    glEnd();
}

extern "C"
EXPORT asl::PlugInBase* EdgeBlender_instantiatePlugIn(asl::DLHandle myDLHandle) {
    return new EdgeBlender(myDLHandle);
}
