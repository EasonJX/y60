//============================================================================
// Copyright (C) 2000-2003, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//============================================================================
//
//   $RCSfile: watchdog.cpp,v $
//   $Author: ulrich $
//   $Revision$
//   $Date: 2005/04/19 10:02:40 $
//
//
//  Description: The watchdog restarts the application, if it is closed
//               manually or by accident
//
//
//=============================================================================

#include "watchdog.h"
#include "system_functions.h"
#include "Projector.h"

#include <dom/Nodes.h>
#include <asl/string_functions.h>
#include <asl/file_functions.h>
#include <asl/os_functions.h>
#include <asl/Exception.h>
#include <asl/Arguments.h>
#include <asl/Time.h>


#ifdef WIN32
#include <windows.h>
#endif

#include <time.h>
#include <iostream>
#include <fstream>
#include <algorithm>

using namespace std;

const string ourDefaultConfigFile = "watchdog.xml";

asl::Arguments ourArguments;
const asl::Arguments::AllowedOption ourAllowedOptions[] = {
    {"--configfile", "XML configuration file"},
    {"", ""}
};

WatchDog::WatchDog()
    : _myWatchFrequency(30),
      _myAppToWatch(_myLogger),
      _myPowerUpProjectorsOnStartup(true),
      _myRebootTimeInSecondsToday(-1),
      _myHaltTimeInSecondsToday(-1),
      _myUDPCommandListenerThread(0)
{
    _mySplashScreen = SDLSplashScreen::getInstance();
}

void
WatchDog::arm() {
    if (_mySplashScreen->isEnabled())
            _mySplashScreen->show();
        
#ifdef WIN32
    // allow the foreground window to be set instead of blinking in the taskbar
    // see q97925
    SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (LPVOID)0, SPIF_SENDWININICHANGE | SPIF_UPDATEINIFILE);
#endif

    if (_myProjectors.size() && _myPowerUpProjectorsOnStartup) {
        cerr << "Watchdog - Powering up projectors..." << endl;
        for (unsigned i = 0; i < _myProjectors.size(); ++i) {
            _myProjectors[i]->powerUp();
        }

        cerr << "Watchdog - Setting all projectors' inputs" << endl;
        for (unsigned i = 0; i < _myProjectors.size(); ++i) {
            // set projector's input to the configured one
            _myProjectors[i]->selectInput();
        }
    }

    if (_myAppToWatch.performECG()) {
        _myLogger.logToFile(std::string("Monitoring heartbeat file: ") + _myAppToWatch.getHeartbeatFile());
    }

    if (getElapsedSecondsToday() > _myAppToWatch.getRestartTimeInSecondsToday()) {
        _myAppToWatch.setRestartedToday(true);
    }
}

bool
WatchDog::watch() {
    try {
        // Run UDP command listener thread
        if (_myUDPCommandListenerThread) {
            cerr << "Watchdog - Starting udp command listener thread" << endl;
            _myUDPCommandListenerThread->fork();
            asl::msleep(100);
        }

        unsigned myStartDelay = _myAppToWatch.getStartDelay();
        if (myStartDelay > 0) {
            cerr << "Watchdog - Application will start in " << myStartDelay << " seconds." << endl;
            for (unsigned i = 0; i < myStartDelay * 2; ++i) {
                cerr << ".";
                asl::msleep(500);
            }
        }
        
        //if (_mySplashScreen->isEnabled())
        //    _mySplashScreen->hide();
                
        // Main loop
        while (true) {
            bool myRestarted = false;
            std::string myReturnString;
            if (!_myAppToWatch.paused()) {
                myReturnString = "Internal quit.";
                _myAppToWatch.launch();
            } else {
                myReturnString = "Application paused.";
            }
      
            while (myRestarted == false && _myAppToWatch.getProcessResult() == WAIT_TIMEOUT) {
                // update projector state
                for (unsigned i = 0; i < _myProjectors.size(); ++i) {
                    _myProjectors[i]->update();
                }

                // watch application
                myReturnString = _myAppToWatch.runUntilNextCheck(_myWatchFrequency);
                _myAppToWatch.checkHeartbeat();
                _myAppToWatch.checkState();
                myRestarted = _myAppToWatch.checkForRestart();
                
                // system halt & reboot
                checkForHalt();
                checkForReboot();
            }
            
            if (_mySplashScreen->isEnabled())
                    _mySplashScreen->show();
                    
            _myAppToWatch.shutdown();

            _myLogger.logToFile(_myAppToWatch.getFilename() + string(" exited: ") + myReturnString + "\nRestarting application.");

            unsigned myRestartDelay = _myAppToWatch.getRestartDelay();
            
            if (!_myAppToWatch.paused()) {
                
                cerr << "Watchdog - Restarting application in " << myRestartDelay << " seconds" << endl;
                for (unsigned i = 0; i < myRestartDelay * 2; ++i) {
                    cerr << ".";
                    asl::msleep(500);
                }
                
                cerr << endl;
            } else {
                cerr << "Watchdog - Application is currently paused" << endl;
                while (_myAppToWatch.paused()) {
                    asl::msleep(1000);
                }
            }
            
            //if (_mySplashScreen->isEnabled())
            //    _mySplashScreen->hide();
        }
    } catch (const asl::Exception & ex) {
        cerr << "### Exception: " << ex << endl;
        _myLogger.logToFile(string("### Exception: " + ex.what()));
    } catch (...) {
        cerr << "### Error while starting:\n\n" + _myAppToWatch.getFilename() + " " + _myAppToWatch.getArguments() << endl;
        _myLogger.logToFile(string("### Error while starting:\n\n" + _myAppToWatch.getFilename() + " " + _myAppToWatch.getArguments()));
        exit(-1);
    }

    return FALSE;
}

void
WatchDog::checkForReboot() {
    long myElapsedSecondsToday = getElapsedSecondsToday();
    if ((_myRebootTimeInSecondsToday!= -1) && (_myRebootTimeInSecondsToday< myElapsedSecondsToday) ) {
        if (myElapsedSecondsToday - _myRebootTimeInSecondsToday < (_myWatchFrequency * 3)) {
            initiateSystemReboot();
        }
    }
}

void
WatchDog::checkForHalt() {
    long myElapsedSecondsToday = getElapsedSecondsToday();
    if ((_myHaltTimeInSecondsToday!= -1) && (_myHaltTimeInSecondsToday< myElapsedSecondsToday) ) {
        if (myElapsedSecondsToday - _myHaltTimeInSecondsToday < (_myWatchFrequency * 3)) {
            initiateSystemShutdown();
        }
    }
}

void
WatchDog::dumpWinError(const string& theErrorLocation) {
    LPVOID lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                  FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL,
                  GetLastError(),
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR) &lpMsgBuf,
                  0,
                  NULL
                  );

    cerr << "Warning: " << theErrorLocation << " failed." << endl;
    cerr << "         Error was " << *(char*)lpMsgBuf << endl;
}

bool
WatchDog::init(dom::Document & theConfigDoc) {
    try {
        if (theConfigDoc("WatchdogConfig")) {
            const dom::NodePtr & myConfigNode = theConfigDoc.childNode("WatchdogConfig");

            // Setup logfile
            {
                _myLogFilename = asl::expandEnvironment(myConfigNode->getAttribute("logfile")->nodeValue());
                std::string::size_type myDotPos = _myLogFilename.rfind(".", _myLogFilename.size());
                if (myDotPos == std::string::npos) {
                    myDotPos = _myLogFilename.size();
                }
#ifdef LINUX
                time_t ltime = time(NULL);
                struct tm* today = localtime(&ltime);
#else
                __time64_t ltime;
                _time64( &ltime );
                struct tm *today = _localtime64( &ltime );
#endif
                char myTmpBuf[128];
                strftime(myTmpBuf, sizeof(myTmpBuf), "%Y_%m_%d_%H_%M", today);
                _myLogFilename = _myLogFilename.substr(0, myDotPos) + "_" + myTmpBuf + _myLogFilename.substr(myDotPos, _myLogFilename.size());
                AC_DEBUG <<"_myLogFilename: " << _myLogFilename;
                _myLogger.openLogFile(_myLogFilename);
            }

            // Setup watch frequency
            {
                _myWatchFrequency = asl::as<int>(myConfigNode->getAttribute("watchFrequency")->nodeValue());
                AC_DEBUG << "_myWatchFrequency: " << _myWatchFrequency ;
                if (_myWatchFrequency < 1){
                    cerr <<"### ERROR: WatchFrequency must have a value greater then 0 sec." << endl;
                    return false;
                }
            }

            // Setup UDP control
            if (myConfigNode->childNode("UdpControl")) {
                const dom::NodePtr & myUdpControlNode = myConfigNode->childNode("UdpControl");
                // Setup projector control
                if (myUdpControlNode->childNode("ProjectorControl")) {
                    const dom::NodePtr & myProjectorsNode = myUdpControlNode->childNode("ProjectorControl");
                    _myPowerUpProjectorsOnStartup = asl::as<bool>(myProjectorsNode->getAttribute("powerUpOnStartup")->nodeValue());
                    for (unsigned i = 0; i < myProjectorsNode->childNodesLength(); ++i) {
                        Projector* myProjector = Projector::getProjector(myProjectorsNode->childNode(i), &_myLogger);
                        if (myProjector) {
                            _myProjectors.push_back(myProjector);
                        }
                    }
                    AC_DEBUG <<"Found " << _myProjectors.size() << " projectors";
                }
                
                _myUDPCommandListenerThread = new UDPCommandListenerThread(_myProjectors, _myAppToWatch, myUdpControlNode);
            }

            // check for system reboot time command configuration
            if (myConfigNode->childNode("RebootTime")) {
                std::string myRebootTime = (*myConfigNode->childNode("RebootTime"))("#text").nodeValue();
                std::string myHours = myRebootTime.substr(0, myRebootTime.find_first_of(':'));
                std::string myMinutes = myRebootTime.substr(myRebootTime.find_first_of(':')+1, myRebootTime.length());
                _myRebootTimeInSecondsToday = atoi(myHours.c_str()) * 3600;
                _myRebootTimeInSecondsToday += atoi(myMinutes.c_str()) * 60;
                AC_DEBUG <<"_myRebootTimeInSecondsToday : " << _myRebootTimeInSecondsToday;
            }

            // check for system halt time command configuration
            if (myConfigNode->childNode("HaltTime")) {
                std::string myHaltTime = (*myConfigNode->childNode("HaltTime"))("#text").nodeValue();
                std::string myHours = myHaltTime.substr(0, myHaltTime.find_first_of(':'));
                std::string myMinutes = myHaltTime.substr(myHaltTime.find_first_of(':')+1, myHaltTime.length());
                _myHaltTimeInSecondsToday = atoi(myHours.c_str()) * 3600;
                _myHaltTimeInSecondsToday += atoi(myMinutes.c_str()) * 60;
                AC_DEBUG <<"_myHaltTimeInSecondsToday : " << _myHaltTimeInSecondsToday;
            }

            // Setup application
            if (myConfigNode->childNode("Application")) { 
                const dom::NodePtr & myApplicationNode = myConfigNode->childNode("Application");
                       
                if (!_myAppToWatch.setup(myConfigNode->childNode("Application"))) {
                    return false;
                }
                
                // WaitingScreen setup
                if (myApplicationNode->childNode("WaitingScreenBMP")) {
                    std::string myWaitingScreenPath = asl::expandEnvironment((*myApplicationNode->childNode("WaitingScreenBMP"))("#text").nodeValue());
                    int myWaitingScreenPosX = 0;
                    int myWaitingScreenPosY = 0;

                    if (myApplicationNode->childNode("WaitingScreenPosX")) {
                        int myValue = atoi(((*myApplicationNode->childNode("WaitingScreenPosX"))("#text").nodeValue()).c_str());
                        myWaitingScreenPosX = myValue;
                    }

                    if (myApplicationNode->childNode("WaitingScreenPosY")) {
                        int myValue = atoi(((*myApplicationNode->childNode("WaitingScreenPosY"))("#text").nodeValue()).c_str());
                        myWaitingScreenPosY = myValue;
                    }
                    
                    _mySplashScreen->enable(myWaitingScreenPath, myWaitingScreenPosX, myWaitingScreenPosY);
                    
                }
            } 
        }
        
    } catch (const asl::Exception & ex) {
        cerr << "### Exception: " << ex;
    } catch (...) {
        cerr << "Error, while parsing xml config file" << endl;
        exit(-1);
    }
    return true;
}

void
printUsage() {
    cerr << ourArguments.getProgramName() << " Copyright (C) 2003-2005 ART+COM" << endl;
    ourArguments.printUsage();
    cerr << "Default configfile: " << ourDefaultConfigFile << endl;
}

void
readConfigFile(dom::Document & theConfigDoc,  std::string theFileName) {
    AC_DEBUG << "Loading configuration data..." ;
    std::string myFileStr = asl::readFile(theFileName);
    if (myFileStr.empty()) {
        cerr << "Watchdog::readConfigFile: Can't open configuration file "
             << theFileName << "." << endl;
        exit(-1);
    }
    theConfigDoc.parseAll(myFileStr.c_str());
    if (!theConfigDoc) {
        cerr << "Watchdog:::readConfigFile: Error reading configuration file "
             << theFileName << "." << endl;
        exit(-1);
    }
}

int
main(int argc, char* argv[] ) {
#ifdef WIN32
    // This will turn off uncaught exception handling
    SetErrorMode(SEM_NOGPFAULTERRORBOX);
#endif

    ourArguments.addAllowedOptions(ourAllowedOptions);
    if (!ourArguments.parse(argc, argv)) {
        return 0;
    }

    dom::Document myConfigDoc;
    if (ourArguments.haveOption("--configfile")) {
        readConfigFile (myConfigDoc, ourArguments.getOptionArgument("--configfile"));
    } else {
        if (asl::fileExists(ourDefaultConfigFile)) {
            readConfigFile (myConfigDoc, ourDefaultConfigFile);
        } else {
            printUsage();
            return -1;
        }
    }

    asl::Exception::initExceptionBehaviour();
    
    WatchDog myHasso;
    bool mySuccess = myHasso.init(myConfigDoc);

    if (mySuccess) {
        myHasso.arm();
        myHasso.watch();
    }
    exit(-1);
}