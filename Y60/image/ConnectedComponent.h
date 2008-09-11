//============================================================================
// Copyright (C) 2006, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//============================================================================

#ifndef Y60_CONNECTED_COMPONENTS_INCLUDED
#define Y60_CONNECTED_COMPONENTS_INCLUDED

#include <asl/math/Vector234.h>
#include <asl/math/Box.h>
#include <asl/base/Ptr.h>

#include <asl/dom/Value.h>

#include <vector>
#include <list>
#include <map>

namespace y60 {


class Run {
    public:
        Run(int theRow, int theStartCol, int theEndCol);
        asl::Vector2f center();
        int length();

        int _myRow;
        int _myStartCol;
        int _myEndCol;
        int _myLabel;
    private:
        static int _ourLastLabel;
};

typedef asl::Ptr<Run> RunPtr;

typedef std::list<RunPtr> RunList;
typedef std::list<asl::Vector2f> PointList;


class Blob;
typedef asl::Ptr<Blob> BlobPtr;

class Blob {
    public:
        Blob() {}
        Blob(RunPtr theRun);
        ~Blob();
        RunList & getRuns();
        asl::Vector2f center();
        int area();
        asl::Box2i bbox() const;
        void merge( BlobPtr otherBlob);
        RunList & getList();

        BlobPtr _myParent;
    private:
        RunList _myRuns;
};


typedef std::vector<BlobPtr> BlobList;
typedef asl::Ptr<BlobList> BlobListPtr;
typedef std::map<int, BlobPtr> ComponentMap;

BlobListPtr connectedComponents(dom::ResizeableRasterPtr image, int object_threshold);

} // end namespace

#endif // Y60_CONNECTED_COMPONENTS_INCLUDED
