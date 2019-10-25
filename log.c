#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "log.h"

FILE* logFp = NULL;
char logFn[512];

FILE* ioFps[eIoCnt];
char  ioFns[eIoCnt][512];
int ioFpsInit = -1;

TcError openLog(const char* name)
{
    if (logFp)
    {
        printError(logLog, "log has been opened with %s. Please closelog() then openLog() to reopen with different log file name \n", logFn);
        return TcE_Invalid_Argument;
    }
    strncpy(logFn, name, sizeof(logFn));
    logFn[sizeof(logFn)-1] = 0;

    logFp = fopen(logFn, "w");
    if (NULL == logFp)
    {
        fprintf(stderr, "failed to open %s for writting\n", name);
        return TcE_Not_Found;
    }

    return TcE_OK;
}

void closeLog(void)
{
    if (logFp)
    {
        fclose(logFp);
        logFp = NULL;
    }
}

void msgLog(const char* label, int enable, const char* fmt, ...)
{
    char buff[512] = {0};

    va_list ap;

    if ((0 == enable) || (logFp == NULL))
        return;

    va_start(ap, fmt);
    
    sprintf(buff, "%s ", label);
    strcat(buff, fmt);

    vfprintf(logFp, buff, ap);
    fflush(logFp);
}

void msgLogEx(const char* label, int enable, const char* func, const char* file, int line, const char* fmt, ...)
{
    char buff[512] = {0};
    char msg[1024] = {0};

    va_list ap;
    va_start(ap, fmt);

    if ((logFp == NULL) || (!enable))
        return;

    sprintf(buff, "%s:%d: %s %s(): ", file, line, label, func);
    strcat(buff, fmt);

    vsprintf(msg, buff, ap);
    fprintf(logFp, "%s", msg);
    fflush(logFp);
}

void warnLog(const char* label, int enable, const char* func, const char* file, int line, const char* fmt, ...)
{
    char buff[512] = {0};

    va_list ap;

    va_start(ap, fmt);

    if ((logFp == NULL) || (!enable))
        return;
    
    sprintf(buff, "%s WARNING: %s(), <%s,%d> : ", label, func, file, line);
    strcat(buff, fmt);

    vfprintf(logFp, buff, ap);
    fflush(logFp);
}

void errorLog(const char* label, int enable, const char* func, const char* file, int line, const char* fmt, ...)
{
    char buff[512] = {0};
    char msg[1024] = {0};

    va_list ap;
    va_start(ap, fmt);

    sprintf(buff, "ERROR: %s %s() <%s, %d> ", label, func, file, line);
    strcat(buff, fmt);

    vsprintf(msg, buff, ap);
    if (logFp)
    {
        fprintf(logFp, "%s", msg);
        fflush(logFp);
    }
    fprintf(stderr, "%s", msg);
    fflush(stderr);
}

TcError openIo(eIo io, const char* name)
{
    if (ioFpsInit == -1)
    {
        memset(ioFps, 0, sizeof(ioFps));
        ioFpsInit = 0;
    }
    if (io >= eIoCnt)
    {
        printLog(logLog, "invalid io index=%X max=%X\n", io, eIoCnt);
        return TcE_Invalid_Argument;
    }
    if (ioFps[io])
    {
        printLog(logLog, "opened io=%X filename=%s\n", io, ioFns[io]);
        return TcE_Invalid_Argument;
    }
    if ((ioFps[io] = fopen(name, "w")) == NULL)
    {
        printLog(logLog, "cannot open filename=%s for writting\n", name);
        return TcE_Failed;
    }
    strncpy(ioFns[io], name, sizeof(ioFns[0]));
    ioFns[io][sizeof(ioFns[0])] = 0;
    return TcE_OK;
}

void closeIo(eIo io)
{
    if (io >= eIoCnt)  
        return;
    if (ioFpsInit == -1)
        return;

    if (ioFps[io])
    {
        fclose(ioFps[io]);
        ioFps[io] = NULL;
    }
}

void printIo(eIo io, const char* fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    if (ioFps[io])
    {
        vfprintf(ioFps[io], fmt, ap);
    }
}

void assertWithLog(const char* func, const char* file, int line, const char* expr)
{
    char buff[512] = {0};

    sprintf(buff, "ASSERT: assert(%s) : %s() <%s, %d> \n", expr, func, file, line);
    if (logFp)
    {
        fprintf(logFp, "%s", buff);
        fflush(logFp);
    }
    fprintf(stderr, "%s", buff);
    fflush(stderr);
}

void exitWithLog(const char* func, const char* file, int line, const char* expr)
{
    char buff[512] = {0};

    sprintf(buff, "EXIT: exit(%s) : %s() <%s, %d> \n", expr, func, file, line);
    if (logFp)
    {
        fprintf(logFp, "%s", buff);
        fflush(logFp);
    }
    fprintf(stderr, "%s", buff);
    fflush(stderr);
}
