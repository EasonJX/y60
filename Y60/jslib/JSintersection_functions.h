//=============================================================================
// Copyright (C) 2003, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================
//
//   $RCSfile: JSintersection_functions.h,v $
//   $Author: martin $
//   $Revision: 1.1 $
//   $Date: 2004/11/10 13:34:26 $
//
//   Description: Base class wrapper for AbstractRenderWindow
//   TODO: this is not (yet) used by the SDL-based app
//
//=============================================================================

#ifndef __y60_jslib_jsintersection_functions_h_included_
#define __y60_jslib_jsintersection_functions_h_included_

#include "JScppUtils.h"
#include "JSWrapper.h"

#include <y60/Primitive.h>
#include <y60/Scene.h>

namespace jslib {

jsval as_jsval(JSContext *cx, const y60::Primitive::Intersection & theIntersection, 
        const asl::Matrix4f & myTransformation, const asl::Matrix4f & myInverseTransformation);

jsval as_jsval(JSContext *cx, const y60::Primitive::IntersectionList & theVector, 
        const asl::Matrix4f & theMatrix, const asl::Matrix4f & theInverseMatrix);

jsval as_jsval(JSContext *cx, const y60::IntersectionInfo  & theInfo);

jsval as_jsval(JSContext *cx, const asl::SweptSphereContact<float> & theContact);

jsval as_jsval(JSContext *cx, const y60::Primitive::SphereContacts & theContacts, 
        const asl::Matrix4f & theTransformation);

jsval as_jsval(JSContext *cx, const y60::Primitive::SphereContactsList & theVector, 
        const asl::Matrix4f & theTransformation);

jsval as_jsval(JSContext *cx, const y60::CollisionInfo & theInfo);

}

#endif
