//=============================================================================
// Copyright (C) 1993-2005, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================
//
//   $RCSfile: VertexData.h,v $
//   $Author: martin $
//   $Revision: 1.12 $
//   $Date: 2005/04/21 16:25:03 $
//
//  Description: 
//
//=============================================================================

#ifndef _ac_scene_VertexData_h_
#define _ac_scene_VertexData_h_

#include <y60/TypeTraits.h>
#include <y60/DataTypes.h>

#include <asl/Auto.h>
#include <asl/Singleton.h>
#include <asl/Exception.h>
#include <dom/Nodes.h>

namespace y60 {

    DEFINE_EXCEPTION(VertexDataException, asl::Exception);
    DEFINE_EXCEPTION(UnloadVertexArraySizeMismatch, VertexDataException);

    template <class T> struct VertexDataSizeTraits {
        static unsigned long size() {
            return T::size();
        }
    };

    template <>
    struct VertexDataSizeTraits<float> {
        static unsigned long size() {
            return 1;
        }
    };
    
    template <>
    struct VertexDataSizeTraits<double> {
        static unsigned long size() {
            return 1;
        }
    };
    template <class LOCKABLE, class ACCESSIBLE>
    class LockingAccessor /*: public asl::AutoLocker<LOCKABLE> */{
    public:
        LockingAccessor(LOCKABLE & theLockable, ACCESSIBLE & theAccessible,
                        bool forWriting, bool  forReading) 
            : /*asl::AutoLocker<LOCKABLE>(theLockable),*/ _myAccessible(theAccessible) 
        {
            _myAccessible.lock(forWriting, forReading);
        }
        ~LockingAccessor() {
            _myAccessible.unlock();
        }
        ACCESSIBLE & get() {
            return _myAccessible;
        }
        const ACCESSIBLE & get() const {
            return _myAccessible;
        }
    private:
        ACCESSIBLE & _myAccessible;
    };

    template <class T> class VertexData;

    class VertexDataBase;

    template <class T>
    struct VertexDataAccessor : public LockingAccessor<VertexDataBase, VertexData<T> > {
        typedef LockingAccessor<VertexDataBase, VertexData<T> > self;
        VertexDataAccessor(VertexDataBase & theLockable, VertexData<T> & theAccessible,
                           bool forWriting, bool  forReading) 
            : LockingAccessor<VertexDataBase, VertexData<T> >(theLockable, theAccessible, 
                                                              forWriting, forReading) {}
    };
    
    template <class T>
    struct ConstVertexDataAccessor : public LockingAccessor<VertexDataBase, const VertexData<T> > {
        typedef LockingAccessor<const VertexDataBase, VertexData<T> > self;
        ConstVertexDataAccessor(VertexDataBase & theLockable, const VertexData<T> & theAccessible,
                                bool forWriting, bool  forReading) 
            : LockingAccessor<VertexDataBase, const VertexData<T> >(theLockable, theAccessible,
                                                                    forWriting, forReading) {}
    };

    class VertexDataBase  {
        public:
            VertexDataBase(const TypeId & theType) : _myType(theType)  {};
            virtual ~VertexDataBase() {};
            VertexDataBase() {};

            TypeId getType() {
                return _myType;
            }

            virtual size_t size() const = 0;
            virtual size_t getElementSize() const = 0;
#if 0
            virtual const void * getDataPtr() const = 0;
            virtual void * getDataPtr() = 0;

            template <class T>
			typename VertexData<T>::VertexDataVector & getVertexDataCast() {
                return dynamic_cast<VertexData<T>&>(*this).getData();
            }

            template <class T>
			const typename VertexData<T>::VertexDataVector & getVertexDataCast() const {
                return dynamic_cast<const VertexData<T>&>(*this).getData();
            }
#else
            /// returns an accessor object that automatically keeps a lock on the resource that it automatically
            /// released when the accessor object is destroyed
            template <class T>
            asl::Ptr<VertexDataAccessor<T> > getVertexDataAccessor(bool forWriting = true, bool  forReading = false) {
                return asl::Ptr<VertexDataAccessor<T> >(
                    new VertexDataAccessor<T>(*this, dynamic_cast<VertexData<T>&>(*this), forWriting, forReading));
            }

            template <class T>
            asl::Ptr<ConstVertexDataAccessor<T> > getVertexDataAccessor() const {
                return asl::Ptr<ConstVertexDataAccessor<T> >(
                    new ConstVertexDataAccessor<T>(const_cast<VertexDataBase&>(*this), 
                                                   dynamic_cast<const VertexData<T>&>(*this), false, true)); // readonly access
            }
#endif
            virtual void unlock() const = 0;
            virtual void lock(bool forWriting, bool forReading) const = 0;
            
            virtual void useAsPosition() const = 0;
            virtual void useAsNormal() const = 0;
            virtual void useAsColor() const = 0;
            virtual void useAsTexCoord() const = 0;
            virtual const void * getDataPtr() const = 0;
            /// load vertex data from dom source arrays into gfx memory
            virtual void load(const VectorOfUnsignedInt & theIndices,
                              dom::NodePtr theVertexDataNode,
                              unsigned theBeginIndex, 
                              unsigned theEndIndex) = 0;

            /// write back vertex data from gfx memory to dom source arrays
            virtual void unload(const VectorOfUnsignedInt & theIndices,
                      dom::NodePtr theVertexDataNode,
                      unsigned theBeginIndex, unsigned theEndIndex) const = 0;

        private:
            VertexDataBase(const VertexDataBase &);
            VertexDataBase & operator=(const VertexDataBase &);
            TypeId _myType;
         
    };

    typedef asl::Ptr<VertexDataBase> VertexDataBasePtr;

    template <class T>
    class VertexData : public VertexDataBase {
    public:
        VertexData() : VertexDataBase(TypeIdTraits<T>::type_id())  {};
        virtual ~VertexData() {};

        virtual size_t getElementSize() const {
            return VertexDataSizeTraits<T>::size();
        }
        virtual void push_back(const T & theElement) = 0;
        virtual T & operator[](unsigned n) = 0;
        virtual const T & operator[](unsigned n) const = 0;
    private:
        VertexData(const VertexData &);
        VertexData & operator=(const VertexData &);
    };

    template <class T>
    class VertexDataFactory {
        public:
            typedef asl::Ptr<VertexData<T> > (*FactoryMethod)(const VertexBufferUsage &); 

            VertexDataFactory() : _myFactoryMethod(0) {};
            
            asl::Ptr<VertexData<T> > create(const VertexBufferUsage & theUsage) {

                if (_myFactoryMethod) {
                    return (*_myFactoryMethod)(theUsage);
                } else {
                    return asl::Ptr<VertexData<T> >(0);
                    //throw asl::Exception("No Data Factories added yet!", PLUS_FILE_LINE);
                }
            }
            void setFactoryMethod(FactoryMethod theFactoryMethod) {
                _myFactoryMethod = theFactoryMethod;
            }
        private:
            FactoryMethod _myFactoryMethod;
    };

    typedef VertexData<float> VertexData1f;
    typedef VertexData<asl::Vector2f> VertexData2f;
    typedef VertexData<asl::Vector3f> VertexData3f;
    typedef VertexData<asl::Vector4f> VertexData4f;

    typedef asl::Ptr<VertexData1f> VertexData1fPtr;
    typedef asl::Ptr<VertexData2f> VertexData2fPtr;
    typedef asl::Ptr<VertexData3f> VertexData3fPtr;
    typedef asl::Ptr<VertexData4f> VertexData4fPtr;

    typedef VertexDataFactory<float> VertexDataFactory1f;
    typedef VertexDataFactory<asl::Vector2f> VertexDataFactory2f;
    typedef VertexDataFactory<asl::Vector3f> VertexDataFactory3f;
    typedef VertexDataFactory<asl::Vector4f> VertexDataFactory4f;
}


#endif // _ac_scene_VertexData_h_


