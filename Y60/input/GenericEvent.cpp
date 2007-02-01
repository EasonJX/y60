//=============================================================================
// Copyright (C) 2007, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================

#include "GenericEvent.h"

namespace y60 {

GenericEvent::GenericEvent(const std::string & theCallback, 
                           const dom::NodePtr & theSchemaDoc,
                           const dom::ValueFactoryPtr & theValueFactory) :
    Event( GENERIC ),
    _myDocument( new dom::Document() )
{


    if ( theValueFactory ) {
        _myDocument->setValueFactory( theValueFactory );
    }
    if ( theSchemaDoc ) {
        _myDocument->addSchema( * theSchemaDoc, "" );
    }

    _myDocument->appendChild( dom::NodePtr( new dom::Element("generic")) );
    _myDocument->firstChild()->appendAttribute("when", double( when ));
    if ( ! theCallback.empty()) {
        _myDocument->firstChild()->appendAttribute("callback", theCallback);
    }
}

GenericEvent::GenericEvent(const dom::NodePtr & theNode) :
    Event( GENERIC ),
    _myDocument( new dom::Document() )
{
    _myDocument->appendChild( theNode );
}

dom::NodePtr
GenericEvent::asNode() const {
    return _myDocument->firstChild();
}

dom::NodePtr 
GenericEvent::getNode() const {
    return _myDocument->firstChild();
}

dom::NodePtr 
GenericEvent::getDocument() const {
    return _myDocument;
}

}
