//============================================================================
// Copyright (C) 2000-2003, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//
//  Description: XML-File-Export Plugin
//
//=============================================================================

#include "BuilderBase.h"

#include <y60/NodeNames.h>
#include <y60/CommonTags.h>
#include <dom/Nodes.h>

namespace y60 {

    BuilderBase::BuilderBase() {};

    BuilderBase::BuilderBase(const std::string & theNodeName)
        : _myNode(dom::NodePtr(new dom::Element(theNodeName)))
    {}

    BuilderBase::BuilderBase(const dom::NodePtr & theNode) :
        _myNode(theNode)
    {}

    BuilderBase::~BuilderBase() {};

    void
    BuilderBase::reset(const std::string & theNodeName) {
        _myNode = dom::NodePtr(new dom::Element(theNodeName));
    }

    const std::string & 
	BuilderBase::getId() const {
		return _myNode->getAttributeString(ID_ATTRIB);
	}

    const std::string &
    BuilderBase::appendNodeWithId(const BuilderBase & theChildBuilder,
            dom::NodePtr theListNode)
    {
        // With assimilate child, the object already has an id.
        if (!(theChildBuilder.getNode()->getAttribute(ID_ATTRIB))) {
            theChildBuilder.getNode()->appendAttribute(ID_ATTRIB, IdTag::getDefault());
        }
        theListNode->appendChild(theChildBuilder.getNode());
        return theChildBuilder.getNode()->getAttribute(ID_ATTRIB)->nodeValue();
    }

    void
    BuilderBase::removeNodeById(dom::NodePtr theListNode, const std::string & theId) {
        unsigned int myNumChilds = theListNode->childNodesLength();
        for(int i = 0; i< myNumChilds; i++) {
            dom::NodePtr myChild = theListNode->childNode(i);
            if (myChild->getAttribute(ID_ATTRIB)->nodeValue() == theId) {
                theListNode->removeChild(myChild);
            }
        }
    }

    dom::NodePtr
    BuilderBase::getNode() const {
        return _myNode;
    }
    void
    BuilderBase::setNode(const dom::NodePtr & theNode) {
        _myNode = theNode;
    }
}
