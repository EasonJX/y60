//=============================================================================
// Copyright (C) 1993-2006, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================

#include "PowermateInputExtension.h"

#include <asl/Logger.h>
#include <y60/AxisEvent.h>
#include <y60/ButtonEvent.h>

#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

using namespace std;
using namespace y60;


static const char *valid_prefix[2] = {
    "Griffin PowerMate",
    "Griffin SoundKnob"
};

PowermateInputExtension::PowermateInputExtension(asl::DLHandle theDLHandle)
    : asl::PlugInBase(theDLHandle)
{}


PowermateInputExtension::~PowermateInputExtension() {
    for(unsigned i=0; i<_myFileDescriptorList.size(); ++i) {
        close(_myFileDescriptorList[i]);
    }
}


void PowermateInputExtension::init() {

    int powermate = -1;
    _myFileDescriptorList.clear();
    
    findPowermates();
    if(_myFileDescriptorList.empty() ) {
        AC_ERROR << "Unable to locate powermate";
        return;
    }
    
    AC_PRINT << "found powermates " << _myFileDescriptorList.size();
    
 }


EventPtrList
PowermateInputExtension::poll() {

    EventPtrList curEvents;
    struct input_event ibuffer[BUFFER_SIZE];
    int r, events;

    
    for(unsigned i=0; i<_myFileDescriptorList.size(); ++i) {
        
        r = read(_myFileDescriptorList[i], ibuffer, sizeof(struct input_event) * BUFFER_SIZE);
        if( r > 0 ){
            events = r / sizeof(struct input_event);
            for(unsigned myEventIndex=0; myEventIndex<events; myEventIndex++) {
                processEvent(&ibuffer[myEventIndex], i, curEvents);
            }
        }
        //setLED(100.0f, _myFileDescriptorList[i]);
    }
    
    return curEvents;
    
}

void
PowermateInputExtension::findPowermates()
{
    char devname[256];
    int fd;
    
    for(unsigned i=0; i<NUM_EVENT_DEVICES; i++){
        sprintf(devname, "/dev/input/event%d", i);
        AC_PRINT << devname;
        fd = openPowermate(devname);
        if(fd >= 0) {
            _myFileDescriptorList.push_back(fd);
        }
    }
}


int
PowermateInputExtension::openPowermate(const char *dev)
{
    int fd = open(dev, O_NONBLOCK|O_RDWR);
    char name[255];

    AC_PRINT << "filedes" << fd;
    if(fd < 0){
        return -1;
    }
    
    if(ioctl(fd, EVIOCGNAME(sizeof(name)), name) < 0){
        AC_ERROR << dev << " EVIOCGNAME failed: " << strerror(errno);
        close(fd);
        return -1;
    }
    
    // it's the correct device if the prefix matches what we expect it to be:
    for(unsigned i=0; i<2; i++) {
        if(!strncasecmp(name, valid_prefix[i], strlen(valid_prefix[i]))) {
            AC_PRINT << fd << "file des";
            return fd;
        }
    }
    
    close(fd);
    return -1;
}


void
PowermateInputExtension::processEvent(struct input_event *ev, int theID, EventPtrList & theEventList)
{
    switch(ev->type){
    case EV_MSC:
        //printf("The LED pulse settings were changed; code=0x%04x, value=0x%08x\n", ev->code, ev->value);
        break;
    case EV_REL:
        theEventList.push_back(EventPtr(new AxisEvent(theID, 0, ev->value)));
        break;
    case EV_KEY:
        if(ev->code != BTN_0)
            fprintf(stderr, "Warning: unexpected key event; ev->code = 0x%04x\n", ev->code);
        else {
            y60::Event::Type myTypeDown(y60::Event::BUTTON_DOWN);
            y60::Event::Type myTypeUp(y60::Event::BUTTON_UP);
            theEventList.push_back(EventPtr(new ButtonEvent((ev->value ? myTypeDown : myTypeUp),
                                                            theID, 0)));
            //printf("Button was %s\n", ev->value? "pressed":"released");
        }
        break;
    default:
        break;
    }
    
    fflush(stdout);
}


void
PowermateInputExtension::pulseLED(int theStaticBrightness, int thePulseSpeed, int thePulseTable, int thePulseAsleep,
                                  int thePulseAwake, int theFileDescriptor)
{
    struct input_event ev;
    memset(&ev, 0, sizeof(struct input_event));
    
    theStaticBrightness &= 0xFF;
    
    if(thePulseSpeed < 0)
        thePulseSpeed = 0;
    if(thePulseSpeed > 510)
        thePulseSpeed = 510;
    if(thePulseTable < 0)
        thePulseTable = 0;
    if(thePulseTable > 2)
        thePulseTable = 2;
    thePulseAsleep = !!thePulseAsleep;
		thePulseAwake = !!thePulseAwake;
    
    ev.type = EV_MSC;
    ev.code = MSC_PULSELED;
    ev.value = theStaticBrightness | (thePulseSpeed << 8) | (thePulseTable << 17) | (thePulseAsleep << 19) | (thePulseAwake << 20);

    if (write(theFileDescriptor, &ev, sizeof(struct input_event)) != sizeof(struct input_event))
        AC_ERROR << "PowerMate :: Failed to Write to Griffin PowerMate " << strerror(errno);
}

void
PowermateInputExtension::turnLEDOff(int theFileDescriptor) {
    setLED(0.0f, theFileDescriptor);
}

void
PowermateInputExtension::setLED(float thePercent, int theFileDescriptor)
{
    setLED( (unsigned char) (255.0f * (thePercent / 100.0f)), theFileDescriptor);
}

void
PowermateInputExtension::setLED(unsigned char theLevel, int theFileDescriptor)
{
    int myPulseSpeed = 255;
    int myPulseTable = 0;
    int myPulseAsleep = 1;
    int myPulseAwake = 0;

    pulseLED((int)theLevel, myPulseSpeed, myPulseTable, myPulseAsleep, myPulseAwake, theFileDescriptor);
    
    AC_DEBUG << "PowerMate :: LED Brightness to " <<  theLevel;
}



extern "C"
EXPORT asl::PlugInBase* PowermateInputExtension_instantiatePlugIn(asl::DLHandle myDLHandle) {
    return new PowermateInputExtension(myDLHandle);
}
