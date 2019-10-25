#ifndef __types_h__
#define __types_h__

#include <stdio.h>

#define BACKDOOR

typedef unsigned char       u8;
typedef char                s8;
typedef unsigned short      u16;
typedef short               s16;
typedef unsigned int        u32;
typedef int                 s32;
typedef unsigned long long  u64;
typedef long long           s64;

typedef u8 U8;
typedef s8 S8;
typedef u16 U16;
typedef s16 S16;
typedef u32 U32;
typedef s32 S32;
typedef u64 U64;
typedef s64 S64;

//typedef enum { FALSE, TRUE } boolean;

typedef struct {
    int testmode;
    const char* forceFn;
    const char* checkFn;
} option;

#endif /* __types_h__ */
