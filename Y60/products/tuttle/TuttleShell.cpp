
#include <cassert>

#include "TuttleShell.h"

namespace tuttle {

    using namespace std;

    // clish command wrappers
#define CLISH_BUILTIN(name,symbol)                                        \
    static bool_t tuttle_clish_##symbol(const clish_shell_t *theClish,    \
                                        const lub_argv_t *theArguments) { \
        void   *myCookie = clish_shell__get_client_cookie(theClish);      \
        Shell  *myShell = reinterpret_cast<Shell*>(myCookie);             \
        return myShell->command##symbol(theArguments);                    \
    }

#include "TuttleShell.def"

#undef CLISH_BUILTIN


    // clish builtin definitions
#define CLISH_BUILTIN(name,symbol) \
  { #name, tuttle_clish_##symbol },

    static const clish_shell_builtin_t tuttle_clish_builtins[] = {
#include "TuttleShell.def"
        {NULL,NULL}
    };

#undef CLISH_BUILTIN


    // clish command action (for non-builtins)
    static bool_t tuttle_clish_action(const clish_shell_t *theClish, const char *script) {
        return (0 == system(script)) ? BOOL_TRUE : BOOL_FALSE;
    }

    // clish hook table
    static clish_shell_hooks_t tuttle_clish_hooks = {
        /* init     */ NULL,
        /* access   */ NULL,
        /* cmd_line */ NULL,
        /* script   */ tuttle_clish_action,
        /* fini     */ NULL,
        /* builtin  */ tuttle_clish_builtins
    };


    // constructs a shell for theDebugger
    Shell::Shell(Debugger &theDebugger) :
        _myDebugger(theDebugger) {

        _myContext = theDebugger.getContext();
        _myGlobal  = theDebugger.getGlobal();
    }

    // launches the shell in its own thread
    bool_t Shell::launch() {
        pthread_t      *myThread           = &_myClishThread;
        pthread_attr_t *myThreadAttributes = &_myClishThreadAttributes;

        pthread_attr_init(myThreadAttributes);

        return clish_shell_spawn(myThread, myThreadAttributes, &tuttle_clish_hooks, reinterpret_cast<void*>(this));
    }

    // shuts the shell down
    bool_t Shell::shutdown() {
        void *myResult;
        int   mySuccess;
        
        mySuccess = pthread_join(_myClishThread, &myResult);
        
        return mySuccess ? BOOL_FALSE : BOOL_TRUE;
    }

    // XXX: get rid of this
    static JSBool
    Print(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
    {
        uintN i, n;
        JSString *str;
        
        for (i = n = 0; i < argc; i++) {
            str = JS_ValueToString(cx, argv[i]);
            if (!str)
                return JS_FALSE;
            fprintf(stdout, "%s%s", i ? " " : "", JS_GetStringBytes(str));
        }
        n++;
        if (n)
            fputc('\n', stdout);
        return JS_TRUE;
    }


#define CLISH_BUILTIN(symbol) \
    bool_t Shell::command##symbol(const lub_argv_t *theArguments)

    CLISH_BUILTIN(Print) {
        assert(lub_argv__get_count(theArguments) == 1);

        const char *myCode       = lub_argv__get_arg(theArguments, 0);
        const int   myCodeLength = strlen(myCode);
        
        JSContext  *myContext = _myContext;
        JSObject   *myGlobal  = _myGlobal;
        jsval       myReturn;

        if(!JS_EvaluateScript(myContext, myGlobal, myCode, myCodeLength, "shell", 1, &myReturn)) {
            return BOOL_FALSE;
        } else {
            jsval myPrintReturn;
            if(!Print(myContext, _myGlobal, 1, &myReturn, &myPrintReturn))
                return BOOL_FALSE;
        }
        
        return BOOL_TRUE;

    }

    CLISH_BUILTIN(Load) {
        assert(lub_argv__get_count(theArguments) == 1);

        const char *myFile = lub_argv__get_arg(theArguments, 0);

        JSContext *myContext = _myContext;
        JSObject  *myGlobal  = _myGlobal;

        JSScript  *myScript;
        jsval      myReturn;

        myScript = JS_CompileFile(myContext, myGlobal, myFile);

        if(!myScript) {
            return BOOL_FALSE;
        }

        JS_ExecuteScript(myContext, myGlobal, myScript, &myReturn);

        JS_DestroyScript(myContext, myScript);
        
        return BOOL_TRUE;
    }

    CLISH_BUILTIN(ListContexts) {
        return BOOL_TRUE;
    }

    CLISH_BUILTIN(SetContext) {
        return BOOL_TRUE;
    }

    CLISH_BUILTIN(ShowContext) {
        return BOOL_TRUE;
    }

    CLISH_BUILTIN(Trace) {
        return BOOL_TRUE;
    }

#undef CLISH_BUILTIN

}
