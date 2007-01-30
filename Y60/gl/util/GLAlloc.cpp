// __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
//
// Copyright (C) 1993-2005, ART+COM AG Berlin, Germany
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
// __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
//
//    $RCSfile: GLAlloc.cpp,v $
//
//   $Revision: 1.1 $
//
// Description: Block Allocator
//
//
//
// __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __

#include "GLAlloc.h"
// #include <y60/Primitive.h>


#define DB(x) // x
#define DB2(x) // x

using namespace std;

#ifdef AC_USE_X11
// hack to allow old glx header without the prototypes as well as new ones
extern "C" {   
#ifndef PFNGLXALLOCATEMEMORYNVPROC
typedef void * ( * PFNGLXALLOCATEMEMORYNVPROC) (int size, float readfreq, float writefreq, float priority);
#endif

#ifndef PFNGLXFREEMEMORYNVPROC
typedef void ( * PFNGLXFREEMEMORYNVPROC) (GLvoid *pointer);
#endif


PFNGLXALLOCATEMEMORYNVPROC _ac_glXAllocateMemoryNV = 
    (PFNGLXALLOCATEMEMORYNVPROC) glXGetProcAddressARB((const GLubyte *) "glXAllocateMemoryNV");
#define glXAllocateMemoryNV _ac_glXAllocateMemoryNV

PFNGLXFREEMEMORYNVPROC _ac_glXFreeMemoryNV = 
    (PFNGLXFREEMEMORYNVPROC) glXGetProcAddressARB((const GLubyte *) "glXFreeMemoryNV");
#define glXFreeMemoryNV _ac_glXFreeMemoryNV

}

#endif

namespace asl
{    
    bool tryInitializeGLMemoryExtensions() {
#ifdef WIN32
        wglAllocateMemoryNV = (PFNWGLALLOCATEMEMORYNVPROC) wglGetProcAddress("wglAllocateMemoryNV");
        wglFreeMemoryNV = (PFNWGLFREEMEMORYNVPROC) wglGetProcAddress("wglFreeMemoryNV");
        glVertexArrayRangeNV = (PFNGLVERTEXARRAYRANGENVPROC) wglGetProcAddress("glVertexArrayRangeNV");
        glFlushVertexArrayRangeNV = (PFNGLFLUSHVERTEXARRAYRANGENVPROC) wglGetProcAddress("glFlushVertexArrayRangeNV");
        return (wglAllocateMemoryNV && wglFreeMemoryNV && glVertexArrayRangeNV && glFlushVertexArrayRangeNV);
#else
#ifdef AC_USE_X11
        return (glXAllocateMemoryNV != NULL &&
                glXFreeMemoryNV != NULL &&
                glVertexArrayRangeNV != NULL &&
                glFlushVertexArrayRangeNV != NULL);
#else
#ifdef AC_USE_OSX_CGL
		return false;
#endif
#endif
#endif
    }

    bool initializeGLMemoryExtensions() {
        static bool initialized = false;
        static bool failed = false;
        if (!initialized && !failed) {
            failed = !tryInitializeGLMemoryExtensions();
            if (failed) {
                std::cerr << "### ERROR: GLAlloc::initialize failed" << std::endl;
            }
            initialized = true;
        }
        return !failed;
    }

    void * gfxAllocateMemoryNV(asl::AC_SIZE_TYPE theSize,
        float theReadFrequency,
        float theWriteFrequency,
        float thePriority)
    {
        initializeGLMemoryExtensions();
#ifdef WIN32
        return (void*)(wglAllocateMemoryNV(
            theSize, theReadFrequency, theWriteFrequency, thePriority));
#else
#ifdef AC_USE_X11
        return (void*)(glXAllocateMemoryNV(
            theSize, theReadFrequency, theWriteFrequency, thePriority));
#else
#ifdef AC_USE_OSX_CGL
		return 0;
#endif
#endif
#endif
    }

    void gfxFreeMemoryNV(void * theMemory) {
        initializeGLMemoryExtensions();
#ifdef WIN32
        if (wglFreeMemoryNV) {
            wglFreeMemoryNV(theMemory);
        }
#else
#ifdef AC_USE_X11
        glXFreeMemoryNV(theMemory);
#else
#ifdef AC_USE_OSX_CGL
#endif
#endif
#endif
    }

    void AGPMemoryFlushSingleton::resetGLAGPMemoryFlush() {
       _myAGPMemoryIsFlushed = false;
    }
    void
    AGPMemoryFlushSingleton::flushGLAGPMemory() {
        if (!_myAGPMemoryIsFlushed) {    
            _myAGPMemoryIsFlushed = true;
            initializeGLMemoryExtensions();
            glFlushVertexArrayRangeNV();    
        }
    }        
        
    struct AGPMemorySingleton : public asl::Singleton<AGPMemorySingleton> {
        AGPMemorySingleton() {
            ourGLMemory = GLMemoryPtr(new GLMemory(GLMemory::AGPMemoryType));
        }
		GLMemoryPtr ourGLMemory;
    };
    struct GPUMemorySingleton : public asl::Singleton<GPUMemorySingleton> {
        GPUMemorySingleton() {
            ourGLMemory = GLMemoryPtr(new GLMemory(GLMemory::GPUMemoryType));
        }
		GLMemoryPtr ourGLMemory;
    };
    struct MainMemorySingleton : public asl::Singleton<MainMemorySingleton> {
        MainMemorySingleton() {
            ourGLMemory = GLMemoryPtr(new GLMemory(GLMemory::MainMemoryType));
        }
		GLMemoryPtr ourGLMemory;
    };

	GLMemoryPtr AGPMemory() {
        return AGPMemorySingleton::get().ourGLMemory;
	}
	GLMemoryPtr GPUMemory() {
        return GPUMemorySingleton::get().ourGLMemory;
	}
	GLMemoryPtr MainMemory() {
        return MainMemorySingleton::get().ourGLMemory;
	}

	bool
	GLMemory::claim(asl::AC_SIZE_TYPE theCapacity) {
		if (_myMem && _myCapacity == theCapacity) {
			return true;
		}
		if (_myMem) {
			std::cerr << "#ERROR: can not claim new memory, you must release already claimed memory before" << std::endl;
			return false;
		}
		switch (_myMemoryType) {
			case MainMemoryType:
				_myMem = new unsigned char[theCapacity];
				break;
			case AGPMemoryType:
                cerr << "trying to allocate " << theCapacity/1024 << " KBytes AGP Memory" << endl;
				_myMem = static_cast<unsigned char*>(gfxAllocateMemoryNV(theCapacity,0.1f,0.1f,0.5f));
				break;
			case GPUMemoryType:
                cerr << "trying to allocate " << theCapacity/1024 << " KBytes GPU Memory" << endl;
				_myMem = static_cast<unsigned char*>(gfxAllocateMemoryNV(theCapacity,0.1f,0.1f,1.0f));
				break;
		}
		if (_myMem != 0) {
			_myCapacity = theCapacity;
			enableDMA();
			return true;
		}
		return false;
	}

	void
	GLMemory::release() {
		if (_myMem) {

			if (_myMemoryType == MainMemoryType) {
				delete _myMem;
			} else {
			    disableDMA();
				gfxFreeMemoryNV(_myMem);
			}
			_myMem = 0;
		}
	}
    
	bool
	GLMemory::enableDMA() {
		DB(std::cerr << "GLMemory::enableDMA() @" << (void*)_myMem << ", capacity="<<_myCapacity<<std::endl);
    	if (_myMemoryType != MainMemoryType) {
    		glVertexArrayRangeNV(_myCapacity,_myMem);
	    	glEnableClientState(GL_VERTEX_ARRAY_RANGE_NV);
            return true;
		}
        return false;
	}

	void
	GLMemory::disableDMA() {
		DB(std::cerr << "GLMemory::disableDMA() @" << (void*)_myMem << ", capacity="<<_myCapacity<<std::endl);
		//glFlushVertexArrayRangeNV();
		glDisableClientState(GL_VERTEX_ARRAY_RANGE_NV);
		glVertexArrayRangeNV(0,_myMem);
	}

	unsigned char *
	GLMemory::allocateChunk(asl::AC_SIZE_TYPE theRequiredBytes) {
		if (_myMemoryType == MainMemoryType) {			
            return new unsigned char[theRequiredBytes];
		} 

		unsigned char * myResult = 0;
		DB2(std::cerr << "GLMemory::allocateChunk: theRequiredBytes =" << theRequiredBytes<<", fresh free ="<< bytesfree() <<", free stored = "<< _myFreeStoreBytes<<", used="<<_myTotalUsedBytes<<std::endl);
		if (!theRequiredBytes) {
            return 0;
        }
        if (bytesfree() + _myFreeStoreBytes < theRequiredBytes) {
			std::cerr << "#ERROR: definitely out of GL Memory: theRequiredBytes = "<<theRequiredBytes<<", total used = " << _myTotalUsedBytes<<", capacity = "<< _myCapacity <<std::endl;
			std::cerr << "#ERROR: try set Y60_AGP_VERTEX_MEMORY to a higher value eg. 128 (MB), or 0 to use main memory" <<std::endl;
            return 0;
		}
		
        // be sure opengl is not reading from AGP-memory
		if (_myMemoryType == AGPMemoryType) {
            AGPMemoryFlushSingleton::get().flushGLAGPMemory();
    	}
		if (bytesfree() >= theRequiredBytes) {
			DB2(std::cerr << "GLMemory::allocateChunk: using fresh mem, bytesfree =" << bytesfree() << std::endl);
            myResult = extendEnd(theRequiredBytes);
			_myUsedChunks[myResult]= theRequiredBytes;
			_myTotalUsedBytes+=theRequiredBytes;
			DB2(std::cerr << "GLAlloc::allocateChunk: allocated "<< theRequiredBytes << " bytes gfx memory @" << (void*)myResult<<", total used = " << _myTotalUsedBytes<<std::endl);
			_myHint = _myFreeChunks.end();
			return myResult;
		} else {
			DB2(std::cerr << "GLMemory::allocateChunk: try to reclaim from free store"<<std::endl);
			if (_myHint == _myFreeChunks.end()) {
				_myHint = _myFreeChunks.begin();
			}
			for (ChunkMap::iterator it =_myHint; it!=_myFreeChunks.end() || _myHint != _myFreeChunks.begin();) {
				DB2(std::cerr << "GLMemory::allocateChunk: considering free store chunk @"<<(void*)it->first<<" size = "<<it->second<<std::endl);
				// try to reclaim used buffer
				if (it->second >= theRequiredBytes) {
					// fits
					asl::AC_SIZE_TYPE overSize = it->second - theRequiredBytes;
					myResult = it->first+overSize;
					_myUsedChunks[myResult] = theRequiredBytes;
					DB2(std::cerr << "GLMemory::allocateChunk: free store chunk @"<<(void*)it->first<<" fits, oversize = "<<overSize<<std::endl);
					if (overSize) {
						// keep rest in free list
						it->second = overSize;
						_myHint = it;
					} else {
						// exact match, remove from free list
						_myFreeChunks.erase(it);
						_myHint = _myFreeChunks.begin();
					}
					_myTotalUsedBytes+=theRequiredBytes;
					_myFreeStoreBytes-=theRequiredBytes;
					return myResult;
				} else {
					// Block too small, try to join with next Block
					ChunkMap::iterator adjacentChunk = _myFreeChunks.find(it->first+it->second);
					if (adjacentChunk != _myFreeChunks.end()) {
						// found adjacent Chunk, join
						DB2(std::cerr << "GLMemory::allocateChunk: joining chunk @"<<(void*)it->first<<" size = "<<it->second <<" with adjacent chunk @"<<(void*)adjacentChunk->first<<", size "<<adjacentChunk->second<<std::endl);
						it->second += adjacentChunk->second;
						_myFreeChunks.erase(adjacentChunk);
					} else {
						// no adjacent free chunk exists, try next
						DB2(std::cerr << "GLMemory::allocateChunk: no adjacent free chunk, advance to next chunk"<<std::endl);
						++it;
					}
				}
				if (it==_myFreeChunks.end()) {
					_myHint = _myFreeChunks.begin();
				}
			}
        }
		std::cerr << "#ERROR: out of GL Memory: theRequiredBytes = "<<theRequiredBytes<<", total used = " << _myTotalUsedBytes<<", capacity = "<< _myCapacity <<std::endl;
		return 0;
	}
	void
	GLMemory::deallocateChunk(unsigned char * theChunk) {
		if (_myMemoryType == MainMemoryType) {
			delete [] theChunk;
            return;
		} 

		ChunkMap::iterator usedChunk = _myUsedChunks.find(theChunk);
		if (usedChunk != _myUsedChunks.end()) {
			_myFreeChunks[theChunk]=usedChunk->second;
			_myTotalUsedBytes-=usedChunk->second;
			_myFreeStoreBytes+=usedChunk->second;
			_myUsedChunks.erase(usedChunk);
			return;
		}
		std::cerr << "#ERROR: GLMemory::deallocateChunk: Memory Corruption, could not find used chunk @ "<<(void*)theChunk<< std::endl;
	}

	GLMemoryPtr
	VertexMemory(GLMemory::MemoryType theMemoryType, asl::AC_SIZE_TYPE theCapacity) {
	    if (!initializeGLMemoryExtensions() || theCapacity == UINT_MAX) {
	        theMemoryType = GLMemory::MainMemoryType;
	    }

		GLMemoryPtr myMem;
		switch (theMemoryType) {
			case GLMemory::MainMemoryType:
				return MainMemory();
			case GLMemory::AGPMemoryType:
				myMem = AGPMemory();
				break;
			case GLMemory::GPUMemoryType:
				myMem = GPUMemory();
				break;
			default:
				std::cerr << "#ERROR: unknown memory type:"<<(int)theMemoryType << std::endl;
				return MainMemory();
		}
		if (theCapacity) {
		    if (myMem->capacity()) {
		        return myMem;
		    }
		    asl::AC_SIZE_TYPE originalCapacity = theCapacity;
		    while (!myMem->claim(theCapacity) && theCapacity > 0) {
		        theCapacity = (asl::AC_SIZE_TYPE)(0.8f*theCapacity);
			}
			if (theCapacity == 0) {
			    std::cerr << "### ERROR: VertexMemory: could not allocate any GL Memory, unsing normal memory, DMA not enabled"<<std::endl;
			    myMem = MainMemory();
			    if (!myMem->claim(originalCapacity)) {
			        std::cerr << "### ERROR: VertexMemory: could not allocate any main memory, system probably will not work"<<std::endl;
			    }
			} else {
    			if (theCapacity < originalCapacity) {
    			    std::cerr << "### WARNING: VertexMemory: could not allocate all requested GL Memory, requested =  "<< originalCapacity <<std::endl;
    			}
			}
			std::cerr << "VertexMemory() - Allocated "<< theCapacity << " bytes of GL Memory"<<std::endl;
		}
		return myMem;
	}
}

