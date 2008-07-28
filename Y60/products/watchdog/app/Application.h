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
//   $RCSfile: Application.h,v $
//   $Author: ulrich $
//   $Revision: 1.8 $
//   $Date: 2005/04/07 12:27:16 $
//
//
//  Description: A simple application class representing a watched process
//
//
//=============================================================================

//
// watchdog.h :
//
#ifndef INCL_APPLICATION
#define INCL_APPLICATION

#include "system_functions.h"

#include <dom/Nodes.h>

#include <map>
#include <string>
#include <vector>

#include <dom/Nodes.h>
#include <asl/ThreadLock.h>

long getElapsedSecondsToday();

class Logger;

enum RestartMode {
    UNSET        = 0,
    MEMTHRESHOLD = 2,
    RESTARTDAY   = 4,
    RESTARTTIME  = 8,
    CHECKMEMORYTIME = 16,
    CHECKTIMEDMEMORYTHRESHOLD = 32
};

class Application {
    public:
        Application(Logger & theLogger);
        virtual ~Application();

        bool setup(const dom::NodePtr & theAppNode);
        bool checkForRestart( std::string & myRestartMessage );
        void launch();
        void checkHeartbeat();
        void checkState();
        void restart();
        void terminate(const std::string & theReason, bool theWMCloseAllowed);
        std::string runUntilNextCheck(int theWatchFrequency);
       
        unsigned getRestartDelay() const;
        unsigned getStartDelay() const;
        bool     paused() const;
        bool     performECG() const;
        bool     restartedToday() const;
        std::string getHeartbeatFile() const;
        long     getRestartTimeInSecondsToday() const;
        ProcessResult    getProcessResult() const;
        std::string getFilename() const;
        std::string getArguments() const; 
        
        void setPaused(bool thePausedFlag);
        void setRestartedToday(bool theRestartedTodayFlag);       

        void setupEnvironment(const dom::NodePtr & theEnvironmentSettings);


    private:
        void setEnvironmentVariables();

        std::map<std::string, std::string> _myEnvironmentVariables;

        std::string      _myFileName;
        std::string      _myWorkingDirectory;            
        std::vector<std::string> _myArguments;
        std::string      _myWindowTitle;
        long             _myAppStartTimeInSeconds;

        asl::ThreadLock  _myLock;
        bool             _myRecvRestart;

        int              _myAllowMissingHeartbeats;
        int              _myHeartbeatFrequency;
        std::string      _myHeartbeatFile;
        bool             _myPerformECG;

        int              _myRestartMemoryThreshold;

        std::string      _myRestartDay;
        long             _myRestartTimeInSecondsToday;

        long             _myCheckMemoryTimeInSecondsToday;
        int              _myMemoryThresholdTimed;

        // state
        int              _myRestartMode;
        bool             _myRestartCheck;
        bool             _myApplicationPaused;
        bool             _myCheckedMemoryToday;
        bool             _myRestartedToday;
        bool             _myMemoryIsFull;
        bool             _myItIsTimeToRestart;
        bool             _myHeartIsBroken;
        long             _myStartTimeInSeconds;
        ProcessInfo      _myProcessInfo;
        ProcessResult    _myProcessResult;
        std::string      _myLastWeekday;
        bool             _myDayChanged;
        Logger &         _myLogger;
        
        unsigned         _myStartDelay;
        unsigned         _myRestartDelay;
        unsigned         _myStartupCount;

};
#endif // INCL_APPLICATION
