#ifndef __log_h__
#define __log_h__

#include <stdlib.h>
#include <assert.h>
#include "TcError.h"
#include "Platform.h"

#define adAllocLog          "[adAlloc]"                 ,1
#define adLog               "[AD]"                      ,1
#define allocModOnline      "[allocModOnline]"          ,1
#define allocModVmm         "[allocModVmm]"             ,1
#define allocModVtcam       "[allocModVtcam]"           ,1
#define allocProRam         "[allocProRam]"             ,1
#define apiListLog          "[apiListLog]"              ,1
#define apiReadLog          "[apiReadLog]"              ,1
#define appLog              "[app]"                     ,1
#define cascadeLog          "[cascade]"                 ,1
#define commonLibLog        "[cmn]"                     ,1
#define eccLog              "[ECC]"                     ,1
#define genSearchLog        "[genSearch]"               ,0
#define gidMapCountLog      "[gidMapCount]"             ,1
#define gidMapLog           "[gidMap]"                  ,0
#define gidMapModeLog       "[gidMapMode]"              ,1
#define goodBitsLog         "[goodBits]"                ,0
#define ilkLog              "[ilk]"                     ,1
#define logLog              "[log]"                     ,1
#define lpmAllocLog         "[lpmAlloc]"                ,1
#define lpmSearchLog        "[lpmSearch]"               ,1
#define mdioLog             "[deviceConfig]"            ,1
#define nseLibLog           "[nseLib]"                  ,1
#define nseTestLog          "[nseTest]"                 ,1
#define offlineLog          "[offline]"                 ,0
#define onlineCmdLog        "[onlineCmd]"               ,0
#define onlineLog           "[online]"                  ,0
#define quartDevLog			"[quartDev]"				,1
#define quartLog            "[quartLog]"                ,1
#define queueAddLog         "[queueAdd]"                ,0
#define queueProcessLog     "[queueProcess]"            ,0
#define queueRemoveLog      "[queueRemove]"             ,0
#define realTcamLog         "[realTcam]"                ,0
#define recordLog           "[record]"                  ,0
#define runOffLineLog       "[runOffline]"              ,1
#define skrLog              "[skr]"                     ,0
#define sqLog               "[alloc]"                   ,1
#define syncLog             "[sync]"                    ,0
#define splitLog            "[split]"                   ,1
#define tcamLog             "[tcam]"                    ,0
#define tcamMasterKey       "[tcamMasterKey]"           ,1
#define tcamModule          "[tcamModule]"              ,1
#define tcamSearchMode      "[tcamSearchMode]"          ,1
#define tcamSearchPrio      "[tcamSearchPrio]"          ,1
#define tcamSearch          "[tcamSearch]"              ,1
#define tcCmdLog            "[tcCmd]"                   ,1
#define testHooksLog        "[testHooks]"               ,1
#define tpfLog              "[tpf]"                     ,0
#define transportLog        "[transport]"               ,1
#define trieLog             "[trie]"                    ,1
#define verifyActivationBit "[verifyActivationBit]"     ,1
#define verifySplit         "[verifySplit]"             ,1
#define verifyXC            "[verifyXC]"                ,1

typedef enum {
    eIoTpf,
    eIoIlk,
    eIoIlkOut,
    eIoPdOut,
    eIoCnt
} eIo;

#ifndef __FILENAME__
#define __FILENAME__ __FILE__
#endif // __FILENAME__

#define exitLog(expr)       \
            do {            \
                exitWithLog(__FUNCTION__,__FILENAME__,__LINE__, #expr); \
                rc=TcE_Failed; goto done; \
            } while (0)
#define printError(label, ...)                              \
                 errorLog(label, __FUNCTION__,__FILENAME__,__LINE__,__VA_ARGS__)


#define assertLog(expr)                     \
            do {                            \
                int lexpr = ((expr) != 0);  \
                if (!lexpr)  {              \
                    assertWithLog(__FUNCTION__,__FILENAME__,__LINE__, #expr); \
                    rc=TcE_Failed; goto done; \
                }                           \
            } while (0)

#define CHECK_ERROR(tag, a)                                 \
            do {                                            \
                if ((rc=(a)) != 0) {                        \
                    if (rc > 0) {                           \
                        warnLog(tag, __FUNCTION__,__FILENAME__,__LINE__,"rc=%X(%s)\n", rc, TcErrorName(rc)); \
                    }                                       \
                    else {                                  \
                        errorLog(tag, __FUNCTION__,__FILENAME__,__LINE__,"rc=%X(%s)\n", rc, TcErrorName(rc)); \
                        goto done;                          \
                    }                                       \
                }                                           \
            } while (0)

#define CHECK_ERROR_WARN(tag, a)                            \
            do {                                            \
                if ((rc=(a)) != 0) {                        \
                    if (rc > 0) {                           \
                        warnLog(tag, __FUNCTION__,__FILENAME__,__LINE__,"rc=%X(%s)\n", rc, TcErrorName(rc)); \
                    }                                       \
                    else  {                                 \
                        errorLog(tag, __FUNCTION__,__FILENAME__,__LINE__,"rc=%X(%s)\n", rc, TcErrorName(rc)); \
                    }                                       \
                    goto done;                              \
                }                                           \
            } while (0)

#ifdef DEBUG_NSELIB

#define printLog(label, ...)                                                    \
                 msgLogEx(label, __FUNCTION__,__FILENAME__,__LINE__,__VA_ARGS__)

#define printWarn(label, ...)                                                   \
                 warnLog(label, __FUNCTION__,__FILENAME__,__LINE__,__VA_ARGS__)

#else

#define printLog(log, ...)
#define printWarn(log, ...)
#endif




#ifdef __cplusplus
extern "C" {
#endif

API TcError openLog(const char* name);
API void closeLog(void);

API void msgLog(const char* label, int enable, const char* fmt, ...);
API void msgLogEx(const char* label, int enable, const char* func, const char* file, int line, const char* fmt, ...);
API void warnLog(const char* label, int enable, const char* func, const char* file, int line, const char* fmt, ...);
API void errorLog(const char* label, int enable, const char* func, const char* file, int line, const char* fmt, ...);
API void assertWithLog(const char* func, const char* file, int line, const char* expr);
API void exitWithLog(const char* func, const char* file, int line, const char* expr);

API TcError openIo(eIo io, const char* name);
API void closeIo(eIo io);

API void printIo(eIo io, const char* fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* __log_h__ */
