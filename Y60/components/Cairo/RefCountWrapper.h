
#ifndef _Y60_CAIRO_REFCOUNTWRAPPER_INCLUDED_
#define _Y60_CAIRO_REFCOUNTWRAPPER_INCLUDED_

#include <map>

#include <asl/Ptr.h>

namespace jslib {

template <typename WRAPPED>
class CairoWrapper {
public:

    typedef CairoWrapper<WRAPPED> WRAPPER;
    typedef asl::WeakPtr<WRAPPER> WEAKPTR;
    typedef asl::Ptr<WRAPPER>     STRONGPTR;

    static WRAPPER* get(WRAPPED* theWrapped) {
        WRAPPER* myWrapper = _myWrappers[theWrapped];
        if(!myWrapper) {
            myWrapper = new WRAPPER(theWrapped);
            _myWrappers[theWrapped] = myWrapper;
        }
        return myWrapper;
    }

    CairoWrapper(WRAPPED *theWrapped) :
        _myWrapped(theWrapped), _myWeakReference() {
        reference();
    }

    ~CairoWrapper() {
        _myWrappers.erase(_myWrapped);
        unreference();
    }

    WRAPPED* getNative() {
        return _myWrapped;
    }

    STRONGPTR getPtr() {
        return _myWeakReference.lock();
    }

    void self(WEAKPTR thePtr) {
        _myWeakReference = thePtr;
    }

protected:

    static std::map<WRAPPED *, WRAPPER *> _myWrappers;

    WRAPPED               *_myWrapped;
    WEAKPTR                _myWeakReference;

    virtual void reference() {
        throw "XXX";
    }

    virtual void unreference() {
        throw "XXX";
    }
};

}

#endif
