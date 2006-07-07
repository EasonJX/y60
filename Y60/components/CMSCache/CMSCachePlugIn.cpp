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


#include "JSCMSCache.h"

#include <y60/IScriptablePlugin.h>
#include <asl/PlugInBase.h>

namespace y60 {
	class CMSCachePlugIn :
		public asl::PlugInBase,
        public jslib::IScriptablePlugin
	{
	public:
		CMSCachePlugIn (asl::DLHandle theDLHandle);
		virtual ~CMSCachePlugIn();

   		void initClasses(JSContext * theContext, JSObject *theGlobalObject);
        const char * ClassName() {
            static const char * myClassName = "CMSCachePlugIn";
            return myClassName;
        }

	};

}

#include <iostream>

using namespace std;
using namespace asl;
using namespace y60;

CMSCachePlugIn :: CMSCachePlugIn(DLHandle theDLHandle) :
			PlugInBase(theDLHandle)
{
    //DJDecoderRegistration::registerCodecs();
}

CMSCachePlugIn :: ~CMSCachePlugIn() {
    //DJDecoderRegistration::cleanup();
}

void 
CMSCachePlugIn::initClasses(JSContext * theContext, JSObject *theGlobalObject) {
    jslib::JSCMSCache::initClass(theContext, theGlobalObject);
}



extern "C"
EXPORT PlugInBase* y60CMSCache_instantiatePlugIn(DLHandle myDLHandle) {
	return new y60::CMSCachePlugIn(myDLHandle);
}
