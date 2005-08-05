/*
/--------------------------------------------------------------------
|
|      $Id: plfilter3x3.cpp,v 1.3 2004/10/05 13:53:43 uzadow Exp $
|
|      Copyright (c) 1996-1998 Ulrich von Zadow
|
\--------------------------------------------------------------------
*/

#include "plfilter3x3.h"

#include <math.h>

#include <paintlib/plbitmap.h>
#include <paintlib/plpoint.h>


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PLFilter3x3::PLFilter3x3(const asl::Matrix3f & theConvolution)
    : PLFilter(),
      _myConvolution(theConvolution)
{}

PLFilter3x3::~PLFilter3x3()
{}


void PLFilter3x3::Apply(PLBmpBase * pBmpSource, PLBmp * pBmpDest) const
{
  int newWidth = pBmpSource->GetWidth()-2;
  int newHeight= pBmpSource->GetHeight()-2;

  // Calculate the size of the new bitmap
  pBmpDest->Create (newWidth, newHeight, pBmpSource->GetPixelFormat(),
                    NULL, 0, pBmpSource->GetResolution());
  for (int y = 0; y < newHeight; ++y)
  {
    for (int x = 0; x < newWidth; ++x)
    {
      float newR = 0;
      float newG = 0;
      float newB = 0;
      float newA = 0;

      for (int j = 0; j < 3; j++) {
        for (int i = 0; i < 3; i++) {
          PLPixel32 srcPixel = pBmpSource->GetPixel32(x+i, y+j);
          newR += _myConvolution[j][i] * float(srcPixel.GetR());
          newG += _myConvolution[j][i] * float(srcPixel.GetG());
          newB += _myConvolution[j][i] * float(srcPixel.GetB());
          newA += _myConvolution[j][i] * float(srcPixel.GetA());
        }
      }

      pBmpDest->SetPixel(x,y,PLPixel32(PLBYTE(newR), PLBYTE(newG), PLBYTE(newB), PLBYTE(newA)));
    }
  }
}

/*
/--------------------------------------------------------------------
|
|      $Log: plfilter3x3.cpp,v $
|      Revision 1.3  2004/10/05 13:53:43  uzadow
|      paintlib linux rgb oder change.
|
|      Revision 1.2  2004/09/17 08:48:30  janbo
|      fixed CompareImage
|
|      Revision 1.1  2004/08/03 14:30:09  martin
|      better image compare algorithem
|
|
\--------------------------------------------------------------------
*/
