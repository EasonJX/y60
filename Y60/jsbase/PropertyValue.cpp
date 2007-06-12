//=============================================================================
// Copyright (C) 1993-2005, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================

#include "PropertyValue.h"

#include "JSVector.h"
#include "JSBox.h"
#include "JSBox2f.h"
#include "JSMatrix.h"
#include "JSNode.h"
#include "PropertyValue.impl"

namespace y60 {

template int PropertyValue::get() const;
template unsigned PropertyValue::get() const;
template asl::Unsigned16 PropertyValue::get() const;
template char PropertyValue::get() const;
template unsigned char PropertyValue::get() const;
template bool PropertyValue::get() const;
template float PropertyValue::get() const;
template long PropertyValue::get() const;
template std::string PropertyValue::get() const;
template asl::Vector2i PropertyValue::get() const;
template asl::Vector2f PropertyValue::get() const;
template asl::Vector3i PropertyValue::get() const;
template asl::Vector3f PropertyValue::get() const;
template asl::Vector4i PropertyValue::get() const;
template asl::Vector4f PropertyValue::get() const;
template asl::Matrix4f PropertyValue::get() const;
template std::vector<std::string> PropertyValue::get() const;
template std::vector<asl::Vector2f> PropertyValue::get() const;
template std::vector<asl::Box2f> PropertyValue::get() const;
template dom::NodePtr PropertyValue::get() const;
//template asl::Box2i PropertyValue::get() const;
template asl::Box2f PropertyValue::get() const;
//template asl::Box3i PropertyValue::get() const;
template asl::Box3f PropertyValue::get() const;

template bool PropertyValue::get(int& theReturnValue) const;
template bool PropertyValue::get(unsigned& theReturnValue) const;
template bool PropertyValue::get(asl::Unsigned16& theReturnValue) const;
template bool PropertyValue::get(char & theReturnValue) const;
template bool PropertyValue::get(unsigned char & theReturnValue) const;
template bool PropertyValue::get(bool& theReturnValue) const;
template bool PropertyValue::get(float& theReturnValue) const;
template bool PropertyValue::get(long& theReturnValue) const;
template bool PropertyValue::get(std::string& theReturnValue) const;
template bool PropertyValue::get(asl::Vector2i& theReturnValue) const;
template bool PropertyValue::get(asl::Vector2f& theReturnValue) const;
template bool PropertyValue::get(asl::Vector3i& theReturnValue) const;
template bool PropertyValue::get(asl::Vector3f& theReturnValue) const;
template bool PropertyValue::get(asl::Vector4i& theReturnValue) const;
template bool PropertyValue::get(asl::Vector4f& theReturnValue) const;
template bool PropertyValue::get(asl::Matrix4f& theReturnValue) const;
template bool PropertyValue::get(std::vector<std::string> & theReturnValue) const;
template bool PropertyValue::get(std::vector<asl::Vector2f> & theReturnValue) const;
template bool PropertyValue::get(dom::NodePtr& theReturnValue) const;
//template bool PropertyValue::get(asl::Box2i & theReturnValue) const;
template bool PropertyValue::get(asl::Box2f & theReturnValue) const;
//template bool PropertyValue::get(asl::Box3i & theReturnValue) const;
template bool PropertyValue::get(asl::Box3f & theReturnValue) const;
template bool PropertyValue::get(std::vector<asl::Box2f> & theReturnValue) const;

template void PropertyValue::set(const int & theValue);
template void PropertyValue::set(const unsigned & theValue);
template void PropertyValue::set(const asl::Unsigned16 & theValue);
template void PropertyValue::set(const char & theValue);
template void PropertyValue::set(const unsigned char & theValue);
template void PropertyValue::set(const bool & theValue);
template void PropertyValue::set(const float & theValue);
template void PropertyValue::set(const long & theValue);
template void PropertyValue::set(const std::string & theValue);
template void PropertyValue::set(const asl::Vector2i & theValue);
template void PropertyValue::set(const asl::Vector2f & theValue);
template void PropertyValue::set(const asl::Vector3i & theValue);
template void PropertyValue::set(const asl::Vector3f & theValue);
template void PropertyValue::set(const asl::Vector4i & theValue);
template void PropertyValue::set(const asl::Vector4f & theValue);
template void PropertyValue::set(const asl::Matrix4f & theValue);
template void PropertyValue::set(const std::vector<std::string> & theValue);
template void PropertyValue::set(const std::vector<asl::Vector2f> & theValue);
template void PropertyValue::set(const dom::NodePtr & theValue);
//template void PropertyValue::set(const asl::Box2i & theValue);
template void PropertyValue::set(const asl::Box2f & theValue);
//template void PropertyValue::set(const asl::Box3i & theValue);
template void PropertyValue::set(const asl::Box3f & theValue);
template void PropertyValue::set(const std::vector<asl::Box2f> & theValue);

}
