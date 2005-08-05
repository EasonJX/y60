//============================================================================
// Copyright (C) 2004, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//============================================================================
//
//   $Id: PdF1Projector.h,v 1.1 2004/09/15 15:03:45 ulrich Exp $
//   $Author: ulrich $
//   $Revision: 1.1 $
//   $Date: 2004/09/15 15:03:45 $
//
// projectiondesign F1 Projector controller.
//
//=============================================================================

#ifndef _ac_watchdog_PdF1Projector_h_
#define _ac_watchdog_PdF1Projector_h_

#include "Projector.h"

class PdF1Projector : public Projector
{
public:
    explicit PdF1Projector(int thePortNumber);

    virtual void power(bool thePowerFlag);
    virtual void selectInput(VideoSource theSource);

private:
    bool checkHeader(const unsigned char* packet) const;

    void setOpValue(unsigned char* packet, unsigned short value);
    void sendPacket(unsigned char* packet, unsigned int packetLen);
};

#endif
