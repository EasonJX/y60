//=============================================================================
//
// Copyright (C) 2000-2004, ART+COM AG Berlin
//
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.

#include <y60/GLUtils.h>
#include <y60/SDLWindow.h>
#include <y60/GLResourceManager.h>

#include <y60/PixelEncoding.h>
#include <y60/TextureCompressor.h>
#include <y60/I60Header.h>
#include <y60/PLFilterResizePadded.h>

#if 0
#ifdef OSX
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif
#endif

#include <asl/Arguments.h>
#include <asl/numeric_functions.h>
#include <asl/file_functions.h>
#include <asl/MappedBlock.h>
#include <asl/os_functions.h>

#include <dom/Nodes.h>

#include <paintlib/planydec.h>
#include <paintlib/Filter/plfilterresizebilinear.h>
#include <paintlib/Filter/plfilterflip.h>
#include <paintlib/Filter/plfiltercrop.h>
#include <paintlib/plpngenc.h>

#include <SDL/SDL.h>

using namespace std;
using namespace asl;
using namespace dom;
using namespace y60;

void printVersion();

asl::Arguments::AllowedOption myOptions[] = {{"--xml-config",   "%s"},
                                             {"--outfile",      "%s"},
                                             {"--img-dir",      "%s"},
                                             {"--out-dir",      "%s"},
                                             {"--targetdir",    "%s"},
                                             {"--target-size",  "%s"},
                                             {"--compression",  "%s"},
                                             {"--xsize",        "%d"},
                                             {"--ysize",        "%d"},
                                             {"--xaspect",      "%d"},
                                             {"--yaspect",      "%d"},
                                             {"--pre-flip",     ""},
                                             {"--post-flip",    ""},
                                             {"--pre-xsize",    "%d"},
                                             {"--pre-ysize",    "%d"},
                                             {"--crop",         ""},
                                             {"--cropleft",     "%d"},
                                             {"--croptop",      "%d"},
                                             {"--cropright",    "%d"},
                                             {"--cropbottom",   "%d"},
                                             {"--layer",        "%d"},
                                             {"--no-overwrite", ""},
                                             {"--version",      ""  },
                                             {"--help",         ""  },
                                             {"", ""}
                                            };
asl::Arguments myArguments(myOptions);

void
printHelp() {
    myArguments.printUsage();
    cout << "Command line options:" << endl
         << "  --xml-config    FILE   use xml-formatted config file" << endl
         << "  --outfile       FILE   write frame to FILE" << endl
         << "  --img-dir       DIR    search for input files in DIR. files on the command line will be ignored." << endl
         << "  --targetdir     DIR    prepend DIR to output files." << endl
         << "  --target-size   BYTES  skip file if all targets have BYTES length" << endl
         << "  --out-dir       DIR    subdir inside targetdir where to write files" << endl
         << "  --compression   MODE   set OpenGL texture compression mode to MODE" << endl
         <<                           "(default for images with alpha channel: S3TC_DXT5, for images without: S3TC_DXT1)." << endl
         << "  --xsize         PIXEL  set frame-width to PIXEL." << endl
         << "  --ysize         PIXEL  set frame-height to PIXEL." << endl
         << "  --xaspect       PIXEL  set replay x-aspect ratio to PIXEL." << endl
         << "  --yaspect       PIXEL  set replay y-aspect ratio to PIXEL." << endl
         << "  --pre-xsize     PIXEL  resize source image width to PIXEL before doing anything else." << endl
         << "  --pre-ysize     PIXEL  resize source image height to PIXEL before doing anything else." << endl
         << "  --pre-flip             flip source image upside down before applying rotation." << endl
         << "  --post-flip            flip source image upside down after applying rotation." << endl
         << "  --crop                 crop the frame to xsize * ysize instead of scaling it." << endl
         << "  --cropleft      PIXEL  set left border to PIXEL." << endl
         << "  --croptop       PIXEL  set top border to PIXEL." << endl
         << "  --cropright     PIXEL  set right border to PIXEL." << endl
         << "  --cropbottom    PIXEL  set bottom border to PIXEL" << endl
         << "  --layer         N      create i60 with N layer" << endl
         << "  --no-overwrite         enable existing image skipping" << endl
         << "  --version              print version information and exit" << endl
         << "  --help                 print this help text and exit" << endl
         << endl
         << "Valid OpenGL compression modes:" << endl
         << "GENERIC_RGB, GENERIC_RGBA, GENERIC_ALPHA, GENERIC_LUMINANCE," << endl
         << "GENERIC_LUMINANCE_ALPHA, GENERIC_INTENSITY, S3TC_DXT1, S3TC_DXT1A,"<< endl
         << "S3TC_DXT3, S3TC_DXT5, NONE" << endl;
}

#if 0

bool
initExtensions() {
    int missingRequiredExtension = 0;
    if (!glutExtensionSupported("GL_ARB_texture_compression")) {
        printf("OpenGL extension GL_ARB_texture_compression required.\n");
        missingRequiredExtension = 1;
    }
    if (!glutExtensionSupported("GL_EXT_texture_compression_s3tc")) {
        printf("OpenGL extension GL_EXT_texture_compression_s3tc required.\n");
        missingRequiredExtension = 1;
    }
    if (missingRequiredExtension) {
        printf("\nExiting because required OpenGL extensions are not supported.\n");
        return false;
    }
    return true;
}

#endif

template <class TX>
bool
getParameter(const string & theParamName, const asl::Arguments & theArguments, dom::NodePtr theNode, TX & theResult) {
    if (myArguments.haveOption(string("--")+theParamName)) {
        theResult = asl::as<TX>(myArguments.getOptionArgument(string("--")+theParamName));
        return true;
    }

    if (theNode && theNode->getAttribute(theParamName)) {
        // T myResult =(theNode->getAttributeValue<T>(theParamName));
        theResult = (theNode->dom::Node::getAttributeValue<TX>(theParamName));
        return true;
    }
    return false;
}

string
getTargetFilename(const asl::Arguments & theArguments,
                  const dom::NodePtr & theTargetNode,
                  const string & theSourceFileName,
                  const string & theExtensionName = ".i60")
{
    std::string myTargetDir = ".";
    getParameter("targetdir", theArguments, theTargetNode, myTargetDir);

    std::string myOutDir;
    getParameter("out-dir", theArguments, theTargetNode, myOutDir);
    unsigned myTargetSize = 0;
    getParameter("target-size", theArguments, theTargetNode, myTargetSize);

    string myTargetFilenamePart;
    if (!getParameter("outfile", theArguments, theTargetNode, myTargetFilenamePart)) {

        myTargetFilenamePart =theSourceFileName.substr(0,theSourceFileName.find_last_of("."))+ theExtensionName.c_str();
    }

    if (myOutDir.length()) {
        myTargetDir += theDirectorySeparator + myOutDir;
    }
    return myTargetDir + theDirectorySeparator + myTargetFilenamePart;
}

bool
updateNeeded(const string & theSourceFilename, const asl::Arguments & theArguments, const dom::NodePtr theXmlConfig) {
    if (!theArguments.haveOption("--no-overwrite")) {
        return true;
    }

    if (theXmlConfig) {
        int myTargetCount = theXmlConfig->childNodesLength();
        for (int i=0; i < myTargetCount; ++i) {
            if (theXmlConfig->childNode(i)->nodeType() == dom::Node::ELEMENT_NODE ) {
                dom::NodePtr myTargetNode = theXmlConfig->childNode(i);

                string myTargetFileName = getTargetFilename(theArguments, myTargetNode, theSourceFilename);

                unsigned myTargetSize = 0;
                getParameter("target-size", theArguments, myTargetNode, myTargetSize);

                if (!fileExists(myTargetFileName)) {
                    cout << "Target " << myTargetFileName << " does not exist" << endl;
                    return true;
                }
                if (myTargetSize > 0 && (getFileSize(myTargetFileName) != myTargetSize)) {
                    cout << "Target " << myTargetFileName << " has wrong size" << endl;
                    cout << "Found target size:" << getFileSize(myTargetFileName) << ", expected: " << myTargetSize << endl;
                    return true;
                }
            }
        }
    } else {
        string myTargetFileName = getTargetFilename(theArguments, dom::NodePtr(0), theSourceFilename);

        unsigned myTargetSize = 0;
        getParameter("target-size", theArguments, dom::NodePtr(0), myTargetSize);

        if (!fileExists(myTargetFileName)) {
            cout << "Target " << myTargetFileName << " does not exist" << endl;
            return true;
        }
        if (myTargetSize > 0 && (getFileSize(myTargetFileName) != myTargetSize)) {
            cout << "Target " << myTargetFileName << " has wrong size" << endl;
            cout << "Found target size:" << getFileSize(myTargetFileName) << ", expected: " << myTargetSize << endl;
            return true;
        }
    }
    return false;
}

void writeI60Header(const PLAnyBmp & theImage, asl::MappedBlock  & theOutputBlock,
    PixelEncoding thePixelEncoding, unsigned int theLayerCount)
{
    I60Header myHeader;
    myHeader.encoding    = unsigned(thePixelEncoding);
    myHeader.width       = theImage.GetWidth();
    myHeader.height      = theImage.GetHeight();
    myHeader.layercount  = theLayerCount;
    theOutputBlock.appendData(myHeader);
}

void
preProcessSource(PLAnyBmp & theSourceBitMap, const asl::Arguments & theArguments) {
    int pre_x_size = theSourceBitMap.GetWidth();
    int pre_y_size = theSourceBitMap.GetHeight();
    bool pre_scale = false;
    if (theArguments.haveOption("--pre-xsize")) {
        pre_x_size = asl::as<int>(theArguments.getOptionArgument("--pre-xsize"));
    }
    if (theArguments.haveOption("--pre-ysize")) {
        pre_y_size = asl::as<int>(theArguments.getOptionArgument("--pre-ysize"));
    }
    if (pre_x_size !=  theSourceBitMap.GetWidth() ||
            pre_y_size !=  theSourceBitMap.GetHeight())
    {
        cout << "pre-scaling source image to x (width) = "
            << pre_x_size << ", y (height) = " << pre_y_size << endl;
        theSourceBitMap.ApplyFilter(PLFilterResizeBilinear(pre_x_size,pre_y_size));
    }

    if (theArguments.haveOption("--pre-flip")) {
        cout << "flipping source image upside down" << endl;
        theSourceBitMap.ApplyFilter(PLFilterFlip());
    }
    if (theArguments.haveOption("--post-flip")) {
        cout << "flipping source image upside down" << endl;
        theSourceBitMap.ApplyFilter(PLFilterFlip());
    }
}

void
createPostProcessedTarget(PLAnyBmp & theSource, PLAnyBmp & theTarget,
                          const asl::Arguments & theArguments, dom::NodePtr theTargetNode)
{
    int cropleft   = 0;
    int croptop    = 0;
    int cropright  = theSource.GetWidth();
    int cropbottom = theSource.GetHeight();
    int now_xsize  = theSource.GetWidth();
    int now_ysize  = theSource.GetHeight();

    cout << "source image has size " << now_xsize << " x " << now_ysize << endl;

    getParameter("cropleft",theArguments,theTargetNode, cropleft);
    if (! (cropleft >= 0) && (cropleft < now_xsize)) {
        cerr << "### ERROR: cropleft must be within 0 and " << now_xsize-1
            << " but is " << cropleft << endl;
    }

    getParameter("croptop",theArguments,theTargetNode, croptop);
    if (! (croptop >= 0) && (croptop < now_ysize)) {
        cerr << "### ERROR: croptop must be within 0 and " << now_ysize-1
            << " but is " << croptop << endl;
    }

    getParameter("cropright",theArguments,theTargetNode, cropright);
    if (! (cropright > cropleft) && (cropright <= now_xsize)) {
        cerr << "### ERROR: cropright must be within " << cropleft
            << " and " << now_xsize << " but is " << cropright << endl;
    }

    getParameter("cropbottom",theArguments,theTargetNode, cropbottom);
    if (! (cropbottom > croptop) && (cropbottom <= now_ysize)) {
        cerr << "### ERROR: cropbottom must be within " << croptop
            << " and " << now_ysize << " but is " << cropbottom << endl;
    }

    bool scale = false;
    int xsize  = cropright-cropleft;
    int ysize  = cropbottom-croptop;

    if (getParameter("xsize",theArguments,theTargetNode, xsize)) {
        scale = true;
    }

    if (getParameter("ysize",theArguments,theTargetNode, ysize)) {
        scale = true;
    }
    cout << "Extracting image from ("
        << cropleft << "," << croptop << ")-("
        << cropright << "," << cropbottom << ")"
        << "; new size is (" << cropright-cropleft
        << "," << cropbottom-croptop << ")" << endl;

    PLFilterCrop myCropFilter(cropleft,croptop,cropright,cropbottom);
    myCropFilter.Apply(&theSource, &theTarget);

    if (scale){
        cout << "Scaling image to size " << xsize << " x " << ysize << endl;
        theTarget.ApplyFilter(PLFilterResizeBilinear(xsize,ysize));
    }
}

void
writeCompressedTarget(PLAnyBmp & theSource, asl::MappedBlock & theTargetBlock,
                      PixelEncoding thePixelEncoding)
{
    TextureCompressor compressor(thePixelEncoding);

    if (theSource.GetBitsPerPixel() != 32 && theSource.GetBitsPerPixel() != 24) {
        cerr << "### ERROR: Input bitmap must be 32 or 24 bpp." << endl;
        return;
    }
    if (asl::powerOfTwo(theSource.GetWidth()) && asl::powerOfTwo(theSource.GetHeight())) {
        compressor.compress(theSource, theTargetBlock);
    } else {
        unsigned myOldWidth  = theSource.GetWidth();
        unsigned myOldHeight = theSource.GetHeight();
        unsigned myPowerOfTwoWidth  = asl::nextPowerOfTwo(myOldWidth);
        unsigned myPowerOfTwoHeight = asl::nextPowerOfTwo(myOldHeight);

        cerr << "Padding bitmap to next power of two: " << myPowerOfTwoWidth << " "
             << myPowerOfTwoHeight << "." << endl;
        theSource.ApplyFilter(PLFilterResizePadded(myPowerOfTwoWidth, myPowerOfTwoHeight));
        cerr << "Bitmap size after padding: " << theSource.GetWidth() << " " << theSource.GetHeight() << endl;

        compressor.compress(theSource, theTargetBlock);
    }
}

void
loadAndPreProcessSourceBitmaps(vector<asl::Ptr<PLAnyBmp> > & theSourceBitmaps, vector<string> & theSourceFiles,
                               const asl::Arguments & theArguments)
{
    PLAnyPicDecoder myDecoder;
    for(unsigned i = 0; i < theSourceFiles.size(); ++i) {
        asl::Ptr<PLAnyBmp> myNewBitmap(new PLAnyBmp());
        const string & mySourceFile = theSourceFiles[i];

        // assemble filename
        string myFilename;
        if (theArguments.haveOption("--img-dir")) {
            myFilename = string(theArguments.getOptionArgument("--img-dir") + "/" + mySourceFile);
        } else {
            myFilename = mySourceFile;
        }
        if (!fileExists(myFilename) || isDirectory(myFilename)) {
            cerr << "skipping " << myFilename << endl;
            continue;
        }
        myDecoder.MakeBmpFromFile(myFilename.c_str(), & (*myNewBitmap));

        preProcessSource(*myNewBitmap, theArguments);
        theSourceBitmaps.push_back(myNewBitmap);
        cout << mySourceFile << " loaded" << endl;
    }
}

void
writeI60(const string & theKeyFrameSourceFile, vector<asl::Ptr<PLAnyBmp> > theSourceBitmaps,
         const asl::Arguments & theArguments, dom::NodePtr theConfigNode,
         PixelEncoding thePixelEncoding)
{
    string myTargetFileName = getTargetFilename(theArguments, theConfigNode, theKeyFrameSourceFile);
    asl::MappedBlock myOutputBlock(myTargetFileName);
    unsigned int myLayerCount = theSourceBitmaps.size();

    for (unsigned int myLayer = 0; myLayer < myLayerCount; ++myLayer) {
        asl::Ptr<PLAnyBmp> mySourceBitmap = theSourceBitmaps[myLayer];
        PLAnyBmp myTarget;
        createPostProcessedTarget(*mySourceBitmap, myTarget, theArguments, theConfigNode);
        if (myLayer == 0) {
            writeI60Header(myTarget, myOutputBlock, thePixelEncoding, myLayerCount);
        }
        writeCompressedTarget(myTarget, myOutputBlock, thePixelEncoding);
        cout << "O.k. Texture: " << myTargetFileName << " created." << endl;
    }
}

PixelEncoding
getPixelEncoding(const PLAnyBmp & theBmp, const asl::Arguments & theArguments) {
    string myCompressionFormat;
    if (theArguments.haveOption("--compression")) {
        myCompressionFormat = myArguments.getOptionArgument("--compression");
    } else {
        if (theBmp.HasAlpha()) {
            myCompressionFormat = "S3TC_DXT5";
        } else {
            myCompressionFormat = "S3TC_DXT1";
        }
        cout << "Using default compression type '" << myCompressionFormat << "'" << endl;
    }

    try {
        return PixelEncoding(getEnumFromString(myCompressionFormat, PixelEncodingString));
    } catch (asl::ParseException & ex) {
        cerr << "### ERROR: Format '" << myCompressionFormat << "' not supported!" << endl << ex << endl;
        printHelp();
        exit(1);
    }
}

void
convertFile(const string & theKeyFrameSourceFile, vector<string> &  theSourceFiles,
            const asl::Arguments & theArguments, dom::NodePtr theXmlConfig)
{
    if (!updateNeeded(theKeyFrameSourceFile, theArguments, theXmlConfig)) {
        cout << "Skipping image : " << theKeyFrameSourceFile << endl;
        return;
    }
    vector<asl::Ptr<PLAnyBmp> > mySourceBitmaps;
    loadAndPreProcessSourceBitmaps(mySourceBitmaps, theSourceFiles, theArguments);
    if (mySourceBitmaps.empty()) {
        return;
    }
    PixelEncoding myPixelEncoding = getPixelEncoding(*mySourceBitmaps[0], theArguments);

    if (theXmlConfig) {
        int myTargetCount = theXmlConfig->childNodesLength();
        for (int i=0; i < myTargetCount; ++i) {
            if (theXmlConfig->childNode(i)->nodeType() == dom::Node::ELEMENT_NODE ) {
                writeI60(theKeyFrameSourceFile, mySourceBitmaps, theArguments, theXmlConfig->childNode(i), myPixelEncoding);
            }
        }
    } else {
        writeI60(theKeyFrameSourceFile, mySourceBitmaps, theArguments, dom::NodePtr(0), myPixelEncoding);
    }
}

int main(int argc, char *argv[]) {
    string myArgDesc = string("[image ... ]\nSee '") + string(getFilenamePart(argv[0])) +
                              " --help' for more information.";
    myArguments.setArgumentDescription(myArgDesc.c_str());

    if ( ! myArguments.parse( argc, argv )) {
        return 1;
    }
    if (myArguments.haveOption("--help")) {
        printHelp();
        return 0;
    }
    if (myArguments.haveOption("--version")) {
        printVersion();
        return 0;
    }
    dom::NodePtr myXmlConfigDocument(new dom::Document());
    dom::NodePtr myXmlConfig(0);

    vector<string> mySourceFiles;
    if ( myArguments.haveOption("--xml-config")) {
        string myXmlConfigFile = myArguments.getOptionArgument("--xml-config");
        myXmlConfigDocument->parseFile(myXmlConfigFile);
        myXmlConfig = myXmlConfigDocument->childNode(0);

    }

    if ( myArguments.haveOption("--img-dir")) {
        string myImageDirectory = myArguments.getOptionArgument("--img-dir");
        DIR * myDir = opendir(myImageDirectory.c_str());
        dirent * myDirEntry = 0;
        while ( (myDirEntry = readdir(myDir))) {
            string myFilename(myDirEntry->d_name);
            if (myFilename[0] != '.') {
                mySourceFiles.push_back(string(myDirEntry->d_name));
            }
        }
        if (mySourceFiles.size() == 0) {
            string myErrorMsg = string("Can't find source images in directory '")
                                + myImageDirectory + "':";
            perror(myErrorMsg.c_str());
            return 1;
        } else {
            cout << "Found " << mySourceFiles.size() << " files in directory '"
                 << myImageDirectory << "'." << endl;
        }
    } else if (myArguments.getCount() > 0) {
        cout << "Got " << myArguments.getCount() << " files on the command line." << endl;
        for (int i=0; i < myArguments.getCount(); ++i) {
            mySourceFiles.push_back(string(myArguments.getArgument(i)));
        }
    } else {
        cerr << "### ERROR: No input files found." << endl;
        return 1;
    }

    unsigned myLayerCount = 1;
    if (myArguments.haveOption("--layer")) {
        myLayerCount = asl::as<int>(myArguments.getOptionArgument("--layer"));
    }


    // create context to let OpenGL do the work
   //  glutCreateWindow("Compressor");


//    if (initExtensions () ) {
    if (SDL_Init(SDL_INIT_NOPARACHUTE) == -1) { // prevents SDL from catching fatal signals
        AC_FATAL << SDL_GetError() << endl;
        exit(1);
    } else {
       try {
           //myApp.setProgramName(ourArguments.getProgramName());
           //rv = myApp.run(myFilename, myIncludePath, myScriptArgs);
           //std::string myProPath;
           //if (asl::get_environment_var("PRO", myProPath)) {
           //     y60::GLResourceManager::get().prepareShaderLibrary(myProPath+"/src/Y60/shader/shaderlibrary.xml","","");
           //}
           asl::Ptr<y60::SDLWindow> myWindow = y60::SDLWindow::create();
           //myWindow->initDisplay();
           myWindow->activateGLContext();
        
           if (mySourceFiles.size() == 0) {
                cerr << "Warning: No images to process!\n";
            }

            // check amount of images to cover all layers
            if ((mySourceFiles.size() % myLayerCount ) != 0 ) {
                string myErrorMsg = string("Not enough images for ") + as_string(myLayerCount) + " layer.\n (Amount: " +
                                    as_string(mySourceFiles.size()) + " )";
                perror(myErrorMsg.c_str());
                return 1;
            }


            cout << "Info: Starting with image " << mySourceFiles[0] << endl;
            cout << "Info: Processing " << mySourceFiles.size() << " image(s)" << endl;
            cout << "Info: Last image is image "<< mySourceFiles[mySourceFiles.size() - 1] << endl;

            for (unsigned i = 0; i < mySourceFiles.size(); i +=myLayerCount) {
                int myRetryCount = 0;
                bool myOKFlag = false;
                do {
                    myOKFlag = false;
                    try {
                        vector<string> myLayeredFiles;
                        for (unsigned myLayerIndex = 0; myLayerIndex < myLayerCount; myLayerIndex++) {
                            myLayeredFiles.push_back(mySourceFiles[i+myLayerIndex]);
                        }
                        string myKeyFrameSourceFile = mySourceFiles[i];
                        convertFile(myKeyFrameSourceFile, myLayeredFiles, myArguments, myXmlConfig);
                        myOKFlag = true;
                    } catch (const asl::Exception & e) {
                        cerr << "### ERROR: Exception occured while processing " << mySourceFiles[i] << endl
                            << "     " << e << endl;
                    } catch (const PLTextException & e) {
                        cerr << "### ERROR: Paintlib exception occured while reading " << mySourceFiles[i] << endl
                            << "     " << static_cast<const char *>(e) << endl;
                    } catch (const exception& e) {
                        cerr << "### ERROR: Standard exception occured while processing " << mySourceFiles[i] << endl
                            << "     " << e.what() << endl;
                    } catch (...) {
                        cerr << "### ERROR: Unknown exception occured while processing " << mySourceFiles[i] << endl;
                    }
                    ++myRetryCount;
                } while (myRetryCount < 3 && !myOKFlag);
                if (!myOKFlag) {
                    cerr << "### ERROR: skipping file " << mySourceFiles[i] << " after "
                        << myRetryCount << " retrys" << endl;
                }
            }
        }
        catch (asl::Exception & e) {
            cerr << "### ERROR: " << e << endl;
            return -1;
        }
        catch (...) {
            cerr << "### ERROR: Unknown exception occured." << endl;
            return -1;
        }
        SDL_Quit();
    }
    return 0;
}

void
printVersion() {
    cout << "CVS $Revision: 1.24 $ $Date: 2005/03/24 23:36:01 $." << endl;
    cout << "Build at " << __DATE__ << " " << __TIME__ << "." << endl;
}
