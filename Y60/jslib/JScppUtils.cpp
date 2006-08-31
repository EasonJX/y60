//=============================================================================
// Copyright (C) 1993-2005, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================
//
//   $RCSfile: JScppUtils.cpp,v $
//   $Author: pavel $
//   $Revision: 1.10 $
//   $Date: 2005/04/24 00:41:19 $
//
//
//=============================================================================

#include "QuitFlagSingleton.h"
#include "JScppUtils.h"

#include <asl/error_functions.h>
#include <js/jsapi.h>
#include <js/jsprf.h>
#include <js/jsparse.h>
#include <js/jsscan.h>
#include <js/jsemit.h>
#include <js/jsscript.h>
#include <js/jsarena.h>
#include <js/jscntxt.h>
#include <js/jsdbgapi.h>
#ifndef WIN32
#include <glib.h>
#endif

using namespace std;
using namespace asl;

#define DB(x) // x;

namespace asl {

std::string 
as_string(JSContext *cx, jsval theVal) {
    JSString *myJSStr = JS_ValueToString(cx, theVal);
    if (!myJSStr) {
        throw asl::Exception("Value is not a string", PLUS_FILE_LINE);
    }
    size_t srcLen = JS_GetStringLength(myJSStr);
#ifdef WIN32
    const LPWSTR myWChars = static_cast<LPWSTR>(JS_GetStringChars(myJSStr));
    AC_SIZE_TYPE myUTF8Size = WideCharToMultiByte(CP_UTF8, 0, myWChars, -1, 0, 0, 0, 0);
    char * myUTF8Chars = new char[myUTF8Size];
    WideCharToMultiByte(CP_UTF8, 0, myWChars, -1, myUTF8Chars, myUTF8Size, 0, 0);
    std::string myResult = std::string(myUTF8Chars);
    delete [] myUTF8Chars;
    return myResult;
#else    
    // get pointer to 16-bit chars
    gunichar2 * myData = reinterpret_cast<gunichar2*>(JS_GetStringChars(myJSStr));

    // now convert to utf-8 encoded c-string
    glong targetLen;
    gchar * myUTF8 = g_utf16_to_utf8(myData, srcLen * sizeof(gunichar2), 0, &targetLen, 0); 

    // now convert to std::string
    if ( ! myUTF8) {
        throw asl::Exception("Failed to convert UTF8 from UTF16.", PLUS_FILE_LINE);
    }
    
    std::string myResult = std::string(myUTF8);

    // clean up
    g_free(myUTF8);
    return myResult;
#endif    
}

std::string as_string(JSContext *cx, JSObject *theObj) {
    return as_string(cx,OBJECT_TO_JSVAL(theObj));
}

std::string as_string(JSType theType) {
    switch (theType) {
        case JSTYPE_VOID: return "JSTYPE_VOID"; /* undefined */
        case JSTYPE_OBJECT: return "JSTYPE_OBJECT";  /* object */
        case JSTYPE_FUNCTION: return "JSTYPE_FUNCTION"; /* function */
        case JSTYPE_STRING: return "JSTYPE_STRING";  /* string */
        case JSTYPE_NUMBER: return "JSTYPE_NUMBER"; /* number */
        case JSTYPE_BOOLEAN: return "JSTYPE_BOOLEAN";             /* boolean */
        case JSTYPE_LIMIT: return "JSTYPE_LIMIT";
    }
    return "ILLEGAL";
}

}

namespace jslib {

jsval 
as_jsval(JSContext *cx, const char * theU8String) {
    // convert from UTF8 to WideChars/UTF16
#ifdef WIN32
    AC_SIZE_TYPE myWCharSize = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, theU8String, -1, 0, 0);
    if (myWCharSize == 0) {
        DWORD myLastError = GetLastError();
        throw UnicodeException(errorDescription(myLastError), PLUS_FILE_LINE); 
    }
    LPWSTR myWChars = new WCHAR[myWCharSize];
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, theU8String, -1, myWChars, myWCharSize);
    JSString * myString = JS_NewUCStringCopyZ(cx,reinterpret_cast<jschar*>(myWChars));
    delete [] myWChars;
    return STRING_TO_JSVAL(myString);
#else
    gunichar2 * myUTF16 = g_utf8_to_utf16(theU8String, -1,0,0,0);
    
    JSString * myString = JS_NewUCStringCopyZ(cx,reinterpret_cast<jschar*>(myUTF16));
    g_free(myUTF16);

    return STRING_TO_JSVAL(myString);
#endif    
}


void ensureParamCount(uintN argc, int theMinCount, int theMaxCount) {
    if (argc < theMinCount) {
        throw Exception(string("Not enough arguments, ")+as_string(theMinCount)+" expected.");
    }
    if (theMaxCount && argc > theMaxCount) {
        throw Exception(string("Too many arguments, ")+as_string(theMaxCount)+" accepted.");
    }
};

JSStackFrame *
getStackFrame(int i, JSContext *cx) {
    try {
        JSStackFrame* fp;
        JSStackFrame* iter = 0;
        int num = 0;

        while(0 != (fp = JS_FrameIterator(cx, &iter)))
        {
            if (num == i) {
                return fp;
            }
            ++num;
        }
        return 0;
    } HANDLE_CPP_EXCEPTION;
}

bool
getFileLine(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, const char * & filename, int & lineno) {
    try {
        uint16 n = 1;
        if (argc > 0 && JSVAL_IS_INT(argv[0])) {
            JS_ValueToUint16(cx, argv[0], &n);
        }

        JSStackFrame * fp = getStackFrame(n,cx);

        if (fp) {
            if(!JS_IsNativeFrame(cx, fp)) {
                JSScript* script = JS_GetFrameScript(cx, fp);
                jsbytecode* pc = JS_GetFramePC(cx, fp);
                if(script && pc)
                {
                    filename = JS_GetScriptFilename(cx, script);
                    lineno =  JS_PCToLineNumber(cx, script, pc);
                    return true;
                }
            }
        }
        return false;
    } HANDLE_CPP_EXCEPTION;
}


JSBool
JSA_DefineConstInts(JSContext *cx, JSObject *obj, JSConstIntPropertySpec *cds) {
    JSBool ok;
    uintN flags = JSPROP_READONLY | JSPROP_PERMANENT;

    for (ok = JS_TRUE; cds->name; cds++) {
        ok = JS_DefineProperty(cx, obj, cds->name, INT_TO_JSVAL(cds->value), NULL, NULL, flags);
        if (!ok) {
            break;
        }
    }
    return ok;
}

void
JSA_AddFunctions(JSContext *cx, JSObject *theClassCtr, JSFunctionSpec * theFunctions) {
    while (theFunctions->name) {
        DB(AC_TRACE << "adding function " << theFunctions->name << " to " << theClassCtr << endl);
        JSFunction * myFunction = JS_DefineFunction(cx, theClassCtr, theFunctions->name, theFunctions->call,
                theFunctions->nargs, 0);
        DB(AC_TRACE << "return:" << myFunction << endl);
        ++theFunctions;
    }
   // JS_DefineFunctions(cx, theClassCtr, Functions());

}

void
JSA_AddProperties(JSContext *cx, JSObject *theClassCtr, JSPropertySpec * theProperties) {
   JS_DefineProperties(cx, theClassCtr, theProperties);

}

std::string
ValType(JSContext *cx, jsval theVal) {
    std::string result = "{";
    if(JSVAL_IS_BOOLEAN(theVal)) {
        result+="BOOLEAN ";
    }
    if(JSVAL_IS_DOUBLE(theVal)) {
        result+="DOUBLE ";
    }
    if(JSVAL_IS_GCTHING(theVal)) {
        result+="GCTHING ";
    }
    if(JSVAL_IS_INT(theVal)) {
        result+="INT ";
    }
    if(JSVAL_IS_NULL(theVal)) {
        result+="NULL ";
    }
    if(JSVAL_IS_NUMBER(theVal)) {
        result+="NUMBER ";
    }
    if(JSVAL_IS_OBJECT(theVal)) {
        result+="OBJECT ";
    }
    if(JSVAL_IS_PRIMITIVE(theVal)) {
        result+="PRIMITIVE ";
    }
    if(JSVAL_IS_STRING(theVal)) {
        result+="STRING ";
    }
    if(JSVAL_IS_VOID(theVal)) {
        result+="VOID ";
    }
    result+="}";
    return result;
}

void
printParams(const char *name, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    cerr << name
         << "("
         //<<"obj = "<<as_string(cx,obj)
         <<"obj = "<<(void*)obj
         <<", obj->classname = "<<JSA_GetClass(cx,obj)->name
         <<", id='"<<as_string(cx,id)<<"'"
         <<",val="<<ValType(cx,*vp)<<as_string(cx,*vp)
         <<")"<<endl;
}
void
printParams(const char *name, JSContext *cx, JSObject *obj, JSType type, jsval *vp) {
    cerr << name
         << "("
         //<<"obj = "<<as_string(cx,obj)
         <<"obj = "<<(void*)obj
         <<", obj->classname = "<<JSA_GetClass(cx,obj)->name
         <<", type='"<<as_string(type)<<"'"
         <<",val="<<ValType(cx,*vp)<<as_string(cx,*vp)
         <<")"<<endl;
}
void
printParams(const char *name, JSContext *cx, JSObject *obj, jsval id) {
    cerr << name
         << "("
         //<<"obj = "<<as_string(cx,obj)
         <<"obj = "<<(void*)obj
         <<", obj->classname = "<<JSA_GetClass(cx,obj)->name
         <<", id='"<<as_string(cx,id)<<"'"
         <<")"<<endl;
}
void
printParams(const char *name, JSContext *cx, JSObject *obj) {
    cerr << name
         << "("
         //<<"obj = "<<as_string(cx,obj)
         <<"obj = "<<(void*)obj
         <<")"<<endl;
}

JSBool
NoisyAddProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    IF_NOISY(printParams("addProperty",cx,obj,id,vp));
    return JS_TRUE;
}
JSBool
NoisyDelProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    IF_NOISY(printParams("delProperty",cx,obj,id,vp));
    return JS_TRUE;
}
JSBool
NoisyGetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    IF_NOISY(printParams("getProperty",cx,obj,id,vp));
    return JS_TRUE;
}
JSBool
NoisySetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    IF_NOISY(printParams("setProperty",cx,obj,id,vp));
    return JS_TRUE;
}
JSBool
NoisyEnumerate(JSContext *cx, JSObject *obj) {
    IF_NOISY(printParams("enumerate",cx,obj));
    return JS_TRUE;
}

JSBool
NoisyResolve(JSContext *cx, JSObject *obj, jsval id) {
    IF_NOISY(printParams("resolve",cx,obj,id));
    return JS_TRUE;
}

JSBool
NoisyNewResolve(JSContext *cx, JSObject *obj, jsval id, uintN flags, JSObject **objp) {
    IF_NOISY(
        printParams("newresolve",cx,obj,id);
        if (flags & JSRESOLVE_QUALIFIED) {
            AC_TRACE << "JSRESOLVE_QUALIFIED"<<endl;
        }
        if (flags & JSRESOLVE_ASSIGNING) {
            AC_TRACE << "JSRESOLVE_ASSIGNING"<<endl;
        }
    )
    *objp = 0;
    return JS_TRUE;
}

JSBool
NoisyConvert(JSContext *cx, JSObject *obj, JSType type, jsval *vp) {
    IF_NOISY_CONV(printParams("convert",cx,obj,type,vp));
    return JS_ConvertStub(cx,obj,type,vp);
}

JS_STATIC_DLL_CALLBACK(void)
NoisyFinalize(JSContext *cx, JSObject *obj) {
    IF_NOISY(AC_TRACE << "finalize " << (void*)obj << endl);
}


// all those following functions are slighly modified versions of private js engine
// functions to allow report of uncaught exception messages when
// calling js functions from c++

typedef JSBool
(* JS_DLL_CALLBACK JSDebugErrorHook)(JSContext *cx, const char *message,
                                     JSErrorReport *report, void *closure);


void
JSA_ReportErrorAgain(JSContext *cx, const char *message, JSErrorReport *reportp, JSErrorReporter onError)
{
    if (!message)
        return;

    if (cx->lastMessage)
        free(cx->lastMessage);
    cx->lastMessage = JS_strdup(cx, message);
    if (!cx->lastMessage)
        return;

    /*
     * If debugErrorHook is present then we give it a chance to veto
     * sending the error on to the regular ErrorReporter.
     */
    if (onError) {
        JSDebugErrorHook hook = cx->runtime->debugErrorHook;
        if (hook &&
            !hook(cx, cx->lastMessage, reportp,
                  cx->runtime->debugErrorHookData)) {
            onError = NULL;
        }
    }
    if (onError)
        onError(cx, cx->lastMessage, reportp);
}

 typedef struct JSExnPrivate {
     JSErrorReport *errorReport;
 } JSExnPrivate;


JSErrorFormatString AC_ErrorFormatString[JSErr_Limit] = {
#if JS_HAS_DFLT_MSG_STRINGS
#define MSG_DEF(name, number, count, exception, format) \
    { format, count } ,
#else
#define MSG_DEF(name, number, count, exception, format) \
    { NULL, count } ,
#endif
#include <js/js.msg>
#undef MSG_DEF
};

static const JSErrorFormatString *
AC_GetErrorMessage(void *userRef, const char *locale, const uintN errorNumber)
{
    if ((errorNumber > 0) && (errorNumber < JSErr_Limit))
        return &AC_ErrorFormatString[errorNumber];
    return NULL;
}


JSErrorReport *
JSA_ErrorFromException(JSContext *cx, jsval exn)
{
    JSObject *obj;
    JSExnPrivate *privateData;
    //jsval privateValue;

    if (JSVAL_IS_PRIMITIVE(exn))
        return NULL;
    obj = JSVAL_TO_OBJECT(exn);
//    if (OBJ_GET_CLASS(cx, obj) != &ExceptionClass)
//        return NULL;

    privateData = (JSExnPrivate*) JS_GetPrivate(cx, obj);
    if (!privateData)
        return NULL;

//    JS_ASSERT(privateData->errorReport);
    return privateData->errorReport;
}

// from mozilla/jsexn.c
JSBool
JSA_reportUncaughtException(JSContext *cx, JSErrorReporter onError)
{
    JSObject *exnObject;
    JSString *str;
    jsval exn;
    JSErrorReport *reportp;
    const char *bytes;

    if (!JS_IsExceptionPending(cx))
        return JS_FALSE;

    if (!JS_GetPendingException(cx, &exn))
        return JS_FALSE;

    /*
     * Because js_ValueToString below could error and an exception object
     * could become unrooted, we root it here.
     */
    if (JSVAL_IS_OBJECT(exn) && exn != JSVAL_NULL) {
        exnObject = JSVAL_TO_OBJECT(exn);
        if (!JS_AddNamedRoot(cx, &exnObject, "exn.report.root"))
            return JS_FALSE;
    } else {
        exnObject = NULL;
    }

#if JS_HAS_ERROR_EXCEPTIONS
    reportp = JSA_ErrorFromException(cx, exn);
#else
    reportp = NULL;
#endif

    str = JS_ValueToString(cx, exn);
    bytes = str ? JS_GetStringBytes(str) : "null";

    if (reportp == NULL) {
        /*
         * XXXmccabe todo: Instead of doing this, synthesize an error report
         * struct that includes the filename, lineno where the exception was
         * originally thrown.
         */
        //JS_ReportErrorNumber(cx, js_GetErrorMessage, NULL,
        JS_ReportErrorNumber(cx, AC_GetErrorMessage, NULL,
                             JSMSG_UNCAUGHT_EXCEPTION, bytes);
    } else {
        /* Flag the error as an exception. */
        reportp->flags |= JSREPORT_EXCEPTION;
        JSA_ReportErrorAgain(cx, bytes, reportp, onError);
    }

    if (exnObject != NULL)
        JS_RemoveRoot(cx, &exnObject);
    return JS_TRUE;
}

JSBool
JSA_CallFunctionName(JSContext * cx, JSObject * obj, const char * theName, int argc, jsval argv[], jsval* rval) {
    try {
        JSBool ok = JS_CallFunctionName(cx, obj, theName, argc, argv, rval);
        if (!ok && !QuitFlagSingleton::get().getQuitFlag()) {
            AC_ERROR << "Exception while calling js function '" << theName << "'" << endl;
            JSA_reportUncaughtException(cx, cx->errorReporter);
        }
        return ok;
    } HANDLE_CPP_EXCEPTION;
}

void
dumpJSObj(JSContext * cx, JSObject * obj) {
    AC_PRINT << "obj = " << (void*)obj
         << ", obj->classname = " << JSA_GetClass(cx,obj)->name;
    AC_PRINT << "Properties: ";
    JSIdArray * myIds = JS_Enumerate(cx, obj);
    if (!myIds) {
        AC_ERROR << "Could not enumerate children for js obj " << (void *)obj;
        return;
    }
    AC_PRINT << "Number of properties: " << myIds->length;
    for (int i=0; i< myIds->length; i++) {
        jsid id = myIds->vector[i];
        AC_PRINT << "  Property " << id << ": ";
        jsval myIdVal;
        if (JS_IdToValue(cx, id, &myIdVal)) {
            string myPropName;
            convertFrom(cx, myIdVal, myPropName);
            AC_PRINT << "    " << myPropName;
//                jsval myValue;
//                JS_GetProperty(cx, myValidOptions, myPropName.c_str(), &myValue);
//                string myValueString;
//                if (myValue != JSVAL_NULL) {
//                    convertFrom(cx, myValue, myValueString);
//                }
        }
    }
    JS_DestroyIdArray(cx, myIds);
}

JSBool
JSA_hasFunction(JSContext * cx, JSObject * obj, const char * theName) {
    try {
        jsval myVal;
        JSBool ok = JS_GetProperty(cx, obj, theName, &myVal);
//        dumpJSObj(cx, obj);
        if  (ok == JS_TRUE && myVal != JSVAL_VOID) {
            if (JS_ValueToFunction(cx, myVal) != NULL) {
                return JS_TRUE;
            } else {
                AC_ERROR << "Property '" << theName << "' is not a function." << endl;
                JSA_reportUncaughtException(cx, cx->errorReporter);
            }
        }
        return JS_FALSE;
    } HANDLE_CPP_EXCEPTION;
}

JSBool
JSA_charArrayToString(JSContext *cx, jsval *argv, string & theResult) {
    // try to convert first argument into an object
    JSObject * myArgument;
    if (!JS_ValueToObject(cx, argv[0], &myArgument)) {
        return JS_TRUE;
    }

    // try to get values from array
    jsuint myArrayLength = 0;
    if (!JS_HasArrayLength(cx, myArgument, &myArrayLength)) {
        return JS_TRUE;
    }

    for (unsigned i = 0; i < myArrayLength; ++i) {
        jsval myArgElement;
        unsigned char  myCharacter;
        if (!JS_GetElement(cx, myArgument, i, &myArgElement)) {
            JS_ReportError(cx, "JSSocket::charArrayToString(): argument %d does not exist", i);
            return JS_FALSE;
        }

        if (!convertFrom(cx, myArgElement, myCharacter)) {
            JS_ReportError(cx, "JSSocket::charArrayToString(): Array element #%d must be a character", i);
            return JS_FALSE;
        }

        theResult.append(1, myCharacter);
    }

    return JS_TRUE;
}

JSBool
JSA_ArrayToString(JSContext * cx, jsval * vp, string & theResult) {
    // try to convert first argument into an object
    JSObject * myJSArray;
    if (!JS_ValueToObject(cx, *vp, &myJSArray)) {
        return JS_FALSE;
    }

    if (!JS_IsArrayObject(cx, myJSArray)) {
        return JS_FALSE;
    }

    // try to get values from array
    jsuint myArrayLength = 0;
    if (!JS_GetArrayLength(cx, myJSArray, &myArrayLength)) {
        return JS_FALSE;
    }

    theResult += "[";
    for (unsigned i = 0; i < myArrayLength; ++i) {
        jsval myArrayElement;
        if (!JS_GetElement(cx, myJSArray, i, &myArrayElement)) {
            JS_ReportError(cx, "JSA_ArrayToString: array element %d does not exist.", i);
            return JS_FALSE;
        }

        string myString;
        if (!convertFrom(cx, myArrayElement, myString)) {
            JS_ReportError(cx, "JSA_ArrayToString: Could not convert array element #%d to string.", i);
            return JS_FALSE;
        }

        theResult += myString;
        if (i < myArrayLength - 1) {
            theResult += ",";
        }
    }
    theResult += "]";

    return JS_TRUE;
}


std::string
searchFileRelativeToJSInclude(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, const std::string & theFile) {
    const char * myCurrentFile;
    int myLine;
    if (!getFileLine(cx, obj, argc, argv, myCurrentFile, myLine)) {
        JS_ReportError(cx, "Failed to determine current JS include file.");
        return JS_FALSE;
    }
    std::string myIncludePath = asl::getDirectoryPart(myCurrentFile);

    std::string myFileWithPath = asl::searchFile(theFile, myIncludePath);
    if (myFileWithPath.empty()) {
          JS_ReportError(cx, "File '%s' not found in %s", theFile.c_str(), myIncludePath.c_str());
          return JS_FALSE;
    }
    return myFileWithPath;
}

}
