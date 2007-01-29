//============================================================================
// Copyright (C) 2006, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//============================================================================

#include "ConnectedComponent.h"

#ifdef LINUX
#   include <values.h>
#else
#   include <float.h>
#endif


using namespace asl;

int Run::_ourLastLabel = 0;

Run::Run(int theRow, int theStartCol, int theEndCol ) :
    _myRow( theRow ),
    _myStartCol( theStartCol ),
    _myEndCol( theEndCol ),
    _myLabel( ++ _ourLastLabel )
{
}

Vector2f
Run::center() {
    return Vector2f( _myStartCol + length() * 0.5, _myRow );
}

int 
Run::length() {
    return _myEndCol - _myStartCol + 1;
}






Blob::Blob(RunPtr theRun) {
    _myRuns.push_back(theRun);
}

Blob::~Blob() {
}

RunList &
Blob::getList(){
    return _myRuns;
}

void
Blob::merge(BlobPtr otherBlob) {
    _myRuns.splice(_myRuns.end(), otherBlob->getList());
}

Vector2f
Blob::center() {
    Vector2f d(0.0, 0.0);
    int c = 0;
    for(RunList::iterator r = _myRuns.begin(); r != _myRuns.end(); r++){
        d += (*r)->center();
        c++;
    }

    return d/double(c);
}

Box2i
Blob::bbox(){
    int x1 = INT_MAX;
    int y1 = INT_MAX;
    int x2 = 0;
    int y2 = 0;
    for(RunList::iterator r = _myRuns.begin(); r != _myRuns.end(); r++){
        x1 = std::min(x1, (*r)->_myStartCol);
        y1 = std::min(y1, (*r)->_myRow);
        x2 = std::max(x2, (*r)->_myEndCol);
        y2 = std::max(y2, (*r)->_myRow);
    }
    return Box2i(x1,y1,x2,y2);
}

int 
Blob::area(){
    int res = 0;
    for(RunList::iterator r = _myRuns.begin(); r != _myRuns.end(); r++){
        res += (*r)->length();
    }
    return res;
}

int connected(RunPtr theRun1, RunPtr theRun2){
    int res=0;
    if (abs(theRun2->_myRow - theRun1->_myRow) != 1)
        return 0;
    if (theRun1->_myStartCol > theRun2->_myStartCol){
        //use > here to do 8-connectivity
        res = theRun2->_myEndCol >= theRun1->_myStartCol;
    }else{
        res = theRun1->_myEndCol >= theRun2->_myStartCol;
    }
    return res;
}

void store_runs(ComponentMap  & theComponents, RunList & theRuns1, RunList & theRuns2){
   BlobPtr p_blob;
   BlobPtr c_blob;
   for (RunList::iterator run1_it = theRuns1.begin(); run1_it!=theRuns1.end(); run1_it++){
       for (RunList::iterator run2_it = theRuns2.begin(); run2_it!=theRuns2.end(); run2_it++){
           if ( /*((*run1_it)->m_Color == (*run2_it)->m_Color) &&*/ connected(*run1_it, *run2_it)){
               p_blob = theComponents.find((*run1_it)->_myLabel)->second;
               c_blob = theComponents.find((*run2_it)->_myLabel)->second;
                while (p_blob->_myParent)
                    p_blob = p_blob->_myParent;
                while (c_blob->_myParent)
                    c_blob = c_blob->_myParent;
                if (c_blob==p_blob){
                    //pass
                    ;
                }else{
                    p_blob->merge(c_blob); //destroys c_blobs runs_list
                    c_blob->_myParent = p_blob;
                }
           }

       }
   }
}

RunPtr 
new_run(ComponentMap & comps, int row, int col1, int col2)
{
    RunPtr run( new Run(row, col1, col2));
    BlobPtr b(new Blob(run));
    //std::cerr<<"creating new run"<<"row="<<row<<" c1="<<col1<<" c2="<<col2<<" color="<<color<<std::endl;
;
    comps[run->_myLabel] = b;
    return run;
}

BlobListPtr connectedComponents(dom::ResizeableRasterPtr image, int object_threshold){
    ComponentMap comps;
    const unsigned char *pixels = image->pixels().begin();
    int stride = image->width();
    Vector2i size( image->width(), image->height());
    RunList *runs1=new RunList();
    RunList *runs2=new RunList();
    RunList *tmp;

    int run_start=0, run_stop=0;
    unsigned char cur=(pixels[0]>object_threshold)?1:0, p=0;
    //std::cerr<<"w="<<size[0]<<" h="<<size[1]<<std::endl;
    //First line
    for(int x=0; x<size[0] ;x++){
        p = (pixels[x]>object_threshold)?1:0;
        if (cur!=p) {
            run_stop = x - 1;
            if (cur)
                runs1->push_back ( new_run(comps, 0, run_start, run_stop) );
            run_start = x;
            cur = p;
        }
    }
    if (cur)
        runs1->push_back( new_run(comps, 0, run_start, size[0]-1) );
    //All other lines
    for(int y=1; y<size[1]; y++){
        run_start = 0;run_stop = 0;
        cur = (pixels[stride*y+0]>object_threshold)?1:0;
        for(int x=0; x<size[0] ;x++){
            p = (pixels[y*stride+x]>object_threshold)?1:0;
            //std::cerr<<"("<<x<<","<<y<<"):"<<(int)p<<std::endl;
            if (cur!=p) {
                run_stop = x - 1;
                if (cur)
                    runs2->push_back(  new_run(comps, y, run_start, run_stop) );
                run_start = x;
                cur = p;
            }
        }
        {
            if (cur)
                runs2->push_back( new_run(comps,y, run_start, size[0]-1) );
            store_runs(comps, *runs1, *runs2);
            tmp = runs1;
            runs1 = runs2;
            runs2 = tmp;
            runs2->clear();
        }
    }
    BlobListPtr result( new BlobList() );
    for (ComponentMap::iterator b=comps.begin();b!=comps.end();b++){
        if (! b->second->_myParent){
            result->push_back(b->second);
        }
    }
    return result;
}
