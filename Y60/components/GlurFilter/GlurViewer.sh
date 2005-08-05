#! /bin/bash
if [ "$2" == "" ] ; then
    if [ "$USE_CG" == "1" ] ; then
        SHADERLIB=$PRO/src/Y60/shader/shaderlibrary.xml
    else
    	SHADERLIB=$PRO/src/Y60/shader/shaderlibrary_nocg.xml
    fi
else
    SHADERLIB="$2"
fi

APPLICATION=acxpshellOPT

if [ "$DEBUG" == "1" ] ; then
    APPLICATION=acxpshellDBG
fi

#
# spidermonkey and kernel 2.6 pthreads need this to
# avoid a floating point exception on loading
#
if uname -r | grep -q "^2.6" ; then
    export LD_ASSUME_KERNEL="2.4.99"
    echo "setting LD_ASSUME_KERNEL=2.4.99"
fi

$APPLICATION -I "$PRO/src/Y60/js;$PRO/lib" $PRO/src/Y60/components/GlurFilter//GlurViewer.js $SHADERLIB $*
exit $?

