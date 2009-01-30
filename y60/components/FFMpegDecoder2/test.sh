#!/bin/bash

Y60_DIR="$PRO/src/y60"
APP_DIR=`dirname $0`
PLUGIN_DIR="$PRO/lib"

if [ "$USE_CG" == "1" ]; then
SHADERLIB="$Y60_DIR/shader/shaderlibrary.xml"
else
SHADERLIB="$Y60_DIR/shader/shaderlibrary_nocg.xml"
fi

function startJSTest {
    JSFilename=$1
    ARGS="-I $PRO/lib;$PRO/src/y60/js ${JSFilename} $PRO/src/y60/shader/shaderlibrary.xml"
    COMMAND="$APP $ARGS"

    if [ "$DEBUG" = "vc" ]; then
        echo Visual Studio Debuger Setup
        echo ------------------------------------------------------------------
        echo $PRO/bin/y60DBG.exe
        echo $ARGS
        echo `cmd /C cd`
        echo ------------------------------------------------------------------
        exit 0
    fi

    echo $COMMAND
    $COMMAND

    Result=$?
    if [[ ${Result} -ne 0 ]]; then
        exit ${Result}
    fi
}

APP=y60

if [ "$DEBUG" = "1" ] ; then
    APP=$PRO/bin/y60DBG
fi

#export AC_LOG_MODULE_VERBOSITY=TRACE/FFMpegDecoder2.cpp

startJSTest MovieTest.js
startJSTest SeekTest.js

exit 0 
