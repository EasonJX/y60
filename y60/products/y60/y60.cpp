/* __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
//
// Copyright (C) 1993-2008, ART+COM AG Berlin, Germany <www.artcom.de>
//
// These coded instructions, statements, and computer programs contain
// proprietary information of ART+COM AG Berlin, and are copy protected
// by law. They may be used, modified and redistributed under the terms
// of GNU General Public License referenced below. 
//    
// Alternative licensing without the obligations of the GPL is
// available upon request.
//
// GPL v3 Licensing:
//
// This file is part of the ART+COM Y60 Platform.
//
// ART+COM Y60 is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// ART+COM Y60 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with ART+COM Y60.  If not, see <http://www.gnu.org/licenses/>.
// __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
//
// Description: TODO  
//
// Last Review: NEVER, NOONE
//
//  review status report: (perfect, ok, fair, poor, disaster, notapplicable, unknown)
//    usefullness            : unknown
//    formatting             : unknown
//    documentation          : unknown
//    test coverage          : unknown
//    names                  : unknown
//    style guide conformance: unknown
//    technical soundness    : unknown
//    dead code              : unknown
//    readability            : unknown
//    understandabilty       : unknown
//    interfaces             : unknown
//    confidence             : unknown
//    integration            : unknown
//    dependencies           : unknown
//    cheesyness             : unknown
//
//    overall review status  : unknown
//
//    recommendations: 
//       - unknown
// __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
*/

#include <y60/jsbase/jssettings.h>
#include <y60/sdlengine/SDLApp.h>

#ifdef AC_BUILT_WITH_CMAKE
#include <SDL.h>
#else
#include <SDL/SDL.h>
#endif

#include <asl/base/SingletonManager.h>

#include <asl/base/Arguments.h>
#include <asl/base/StdOutputRedirector.h>
#include <asl/base/string_functions.h>
#include <asl/base/os_functions.h>
#ifdef AC_BUILT_WITH_CMAKE
#   include <asl/base/buildinfo.h>
#endif

#include <iostream>

using namespace std;
using namespace jslib;

asl::Arguments ourArguments;

#include <iostream>
#include <sstream>

const asl::Arguments::AllowedOptionWithDocumentation ourAllowedOptions[] = {
    {"--jsversion", "version", ""}, // TODO: check was this does exactly
    {"-I", "include path", "semicolon-separated list of directories"},
    {"--no-jswarnings", "", "disable javascript warnings"},
    {"--pause-on-error", "", "if an exception occurs wait for input before quitting"},
    {"--std-logfile", "basename", "logfile basename for stdout/stderr"},
#ifdef USE_TRACEMONKEY
    {"--jit", "", "enable javascript just-in-time compiler"},
    {"--xml", "", ""}, // TODO: check was this does exactly
#endif
#ifdef AC_BUILT_WITH_CMAKE
    {"--buildinfo","component|'all'", "print details about this build and exit"},
#endif
//    {"--exec", "", "treat first argument expression and execute it"},
    {"--help", "", "print this text and exit."},
     //y60 will quit automatically after some frames taking a screen shot.
    {"", "jsmain",   "file containing javascript code"},
    //{"", "args,...", "arguments passed to the script"},
    {"", "", ""}
};

int
main(int argc, char **argv) {

    int rv = EXIT_FAILURE;

    try {
        ourArguments.addAllowedOptionsWithDocumentation(ourAllowedOptions);
        if (!ourArguments.parse(argc, argv)) {
            return 0;
        }
        ourArguments.setShortDescription(ourArguments.getProgramName() +
                " is a javascript, xml, and OpenGL based multimedia framework");
        std::ostringstream myLongDescription;
        myLongDescription << ourArguments.getProgramName() << " executes "
                << "its first argument as a javascript file. All other "
                << "arguments are passed to the script." << endl
                << "See http://y60.artcom.de/ for more information.";
        ourArguments.setLongDescription(myLongDescription.str());


        asl::StdOutputRedirector::get().init(ourArguments);

        SDLApp myApp;
        if (ourArguments.haveOption("--no-jswarnings")) {
            myApp.setReportWarnings(false);
        }

        myApp.setStrictFlag(true);

        if (ourArguments.haveOption("--jsversion")) {
            myApp.setJSVersion(asl::as<int>(ourArguments.getOptionArgument("--jsversion")));
        }
#ifdef USE_TRACEMONKEY
        if (ourArguments.haveOption("--jit")) {
            myApp.enableJIT(true);
        }
        if (ourArguments.haveOption("--xml")) {
            myApp.enableXML(true);
        }
#endif

        if (ourArguments.haveOption("--help")) {
            ourArguments.printHelp();
            return EXIT_SUCCESS;
        }

#ifdef AC_BUILT_WITH_CMAKE
        if (ourArguments.haveOption("--buildinfo")) {
            using asl::build_information;
            std::string component = ourArguments.getOptionArgument("--buildinfo");
            if (component == "all") {
                std::cout << build_information::get();
                return EXIT_SUCCESS;
            } else {
                build_information::const_iterator it = build_information::get().find(component);
                if (it != build_information::get().end()) {
                    std::cout << it->second;
                    return EXIT_SUCCESS;
                }
                std::cerr << "Unknown component '" << component << "'" << std::endl
                          << "Try '" << ourArguments.getProgramName()
                          << " --buildinfo all'" << std::endl;
                return EXIT_FAILURE;
            }
        }
#endif // AC_BUILT_WITH_CMAKE

        string myIncludePath;
        if (ourArguments.haveOption("-I")) {
            myIncludePath = ourArguments.getOptionArgument("-I");
        }

        if (ourArguments.getCount() < 1) {
            ourArguments.printUsage();
            return EXIT_FAILURE;
        }

        std::string myFilename = ourArguments.getArgument(0);
        std::vector<std::string> myScriptArgs;

        unsigned myArgCount = ourArguments.getCount();
        for (unsigned i = 1; i < myArgCount; ++i) {
            myScriptArgs.push_back(ourArguments.getArgument(i));
        }

        if (SDL_Init(SDL_INIT_NOPARACHUTE) == -1) { // prevents SDL from catching fatal signals
            AC_FATAL << "Could not initialize SDL: " << SDL_GetError() << endl;
            rv = EXIT_FAILURE;
        } else {
            myApp.setProgramName(ourArguments.getProgramName());
            rv = myApp.run(myFilename, myIncludePath, myScriptArgs);
            SDL_Quit();
        }
    } catch (asl::Exception & ex) {
        rv = EXIT_FAILURE;
        AC_FATAL << "Exception caught in main: " << ex << endl;
    } catch (std::exception & ex) {
        rv = EXIT_FAILURE;
        AC_FATAL << "std::exception caught in main: " << ex.what() << endl;
    } catch (...) {
        rv = EXIT_FAILURE;
        AC_FATAL << "unknown exception caught in main" << endl;
    }

    if (rv != EXIT_SUCCESS && ourArguments.haveOption("--pause-on-error")) {
        char temp[100];
        AC_FATAL << "Some Errors occured. Hit RETURN to Exit." << endl;
        cin.getline(temp, sizeof(temp));
    }
#ifdef _WIN32
    // [CH]: For some strange reason windows will not pass the error code to the shell if it is returned.
    // But this should not hurt, because MSDN says:
    // "Issuing a return statement from the main function is equivalent to calling the exit
    // function with the return value as its argument."
    exit(rv);
#else
    return rv;
#endif

}

#if 0
#ifdef _WIN32
int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
            LPSTR lpCmdLine, int nCmdShow)
{
    return main(__argc, __argv);
}
#endif

#endif
