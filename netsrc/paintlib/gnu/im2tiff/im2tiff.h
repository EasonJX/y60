//----------------------------------------------------------------
//
// FILE   : im2tiff.h
// AUTHOR : Jose Miguel Buenaposada. (jmbuena@dia.fi.upm.es)     
//
//----------------------------------------------------------------


//-------------- RECURSION PROTECTION ----------------------------
#ifndef _IM2TIFF_H_
#define _IM2TIFF_H_

//--------------------- INCLUDE ----------------------------------
#include <iostream>
#include <string>
#include <fstream>
#include "plstdpch.h"    // Paintlib
#include "planybmp.h"    // Paintlib
#include "planydec.h"    // Paintlib
#include "pltiffenc.h"   // Paintlib

//------------------ DEFINITION ----------------------------------
void printusemessage();
void parseargs(int nargs, char** args);

#endif
