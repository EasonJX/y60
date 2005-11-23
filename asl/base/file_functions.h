/* __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
//
// Copyright (C) 1993-2001, ART+COM Berlin GmbH
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM GmbH Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM GmbH Berlin.
// __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
//
//    $RCSfile: file_functions.h,v $
//
//   $Revision: 1.17 $
//
// Description: file helper functions
//
//
// __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
*/
#ifndef _included_asl_file_functions_
#define _included_asl_file_functions_

#include "Exception.h"
#include "Block.h"
#include <asl/PlugInManager.h>

#include <string>
#include <sys/types.h>
#include <sys/stat.h>

namespace asl {
    
/*! \addtogroup aslbase */
/* @{ */
    
/// returns filename without directory     
std::string getFilenamePart(const std::string & theFileName);
/// returns directory without filename     
std::string getDirectoryPart(const std::string & theFileName);

/// return filename extension, or "" if none was found
std::string getExtension(const std::string & theFileName);

/// returns theFileName without extension
std::string removeExtension(const std::string & theFileName);

/// splits a delimited path list (semicolon or colon delimited) into its components
unsigned splitPaths(const std::string & theDelimitedPaths, std::vector<std::string> & thePathVector);

/// searches a file in a semicolon-seperated list of pathes, 
/// returns the path + file. Embedded environment variables 
/// in the form ${VARNAME} are expanded.
/// @deprecated see asl::PackageManager, which can also search in zip files.
std::string searchFile(const std::string & theFileName, const std::string & theSearchPath);

/// read a complete file into a string
std::string readWholeFile(const std::string& theFileName);
bool readWholeFile(const std::string& theFileName, std::string & theContent);
bool readWholeFile(const std::string& theFileName, asl::ResizeableBlock & theContent);
bool writeWholeFile(const std::string& theFileName, const std::string & theContent);
bool writeWholeFile(const std::string& theFileName, const asl::ReadableBlock & theContent);

bool deleteFile(const std::string& theFileName);
bool moveFile(const std::string& theOldFileName, const std::string & theNewFileName);
bool setLastModified(const std::string & theFilename, time_t theModificationDate);
time_t getLastModified(const std::string & theFilename);

#ifdef OSX
#  define STAT64   stat
#  define STAT64F  stat
#endif
#ifdef LINUX
#  define STAT64   stat64
#  define STAT64F  stat64
#endif
#ifdef WIN32
#  define STAT64   __stat64
#  define STAT64F  _stat64
#endif

inline
bool fileExists(const std::string& theFileName) {
    struct STAT64 myStat;
    return STAT64F(theFileName.c_str(), &myStat) != -1;
}

DEFINE_EXCEPTION(IO_Failure,asl::Exception)


// Warning: off_t is 32 bit (at least) under windows. This function will
// return incorrect values for files > 2 gb.
inline
off_t getFileSize(const std::string& theFileName) {
    struct STAT64 myStat;
    if (STAT64F(theFileName.c_str(), &myStat) != -1) {
        return static_cast<off_t>(myStat.st_size);
    };
    throw asl::IO_Failure("getFileSize","can't stat file");
}

/* @} */

} //Namespace asl

#undef STAT64
#undef STAT64F

#endif

