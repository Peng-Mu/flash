#ifndef __utils_h__
#define __utils_h__

#include <string.h>
#include "types.h"
#include "TcError.h"
#include "log.h"
#include "IoCmd.h"
#include "Io.h"
#include "defines.h"
#include "Platform.h"

#define MASK(a)                             ((1<<(a)) - 1)
#define ALIGN_UP(value, a)                  ((value + (MASK(a))) & (~(MASK(a))))
#define ALIGN(value, a)                     ((value) & (~(MASK(a))))
#define BYTE_COUNT(bits)                    (((bits)+BITS_PER_BYTE-1)>>(BYTE_SHIFT))
#define HEX_COUNT(bytes)                    (2*(bytes) + 1)
#define IS_2POWER(a)                        ((a & (a-1)) == 0)
#define ALL_ONE                             (-1)
#define U8_SHIFT                           (BYTE_SHIFT)
#define U8_BIT_COUNT                       (BITS_PER_BYTE)
#define U8_ALIGN_FLOOR(a)                  ((a) & (~(U8_BIT_COUNT-1)))
#define U8_ALIGN_CEIL(a)                   U8_ALIGN_FLOOR((a)+(U8_BIT_COUNT-1))
#define U8_MOD(a)                          ((a)&(U8_BIT_COUNT-1))
#define U8_COUNT_FLOOR(a)                  ((a)>>U8_SHIFT)
#define U8_COUNT_CEIL(a)                   U8_COUNT_FLOOR((a)+U8_BIT_COUNT-1)
#define U64_SHIFT                          (6)
#define U64_BIT_COUNT                      (1<<U64_SHIFT)
#define U64_ALIGN_FLOOR(a)                 ((a) & (~(U64_BIT_COUNT-1)))
#define U64_ALIGN_CEIL(a)                  U64_ALIGN((a) + U64_BITS_COUNT - 1)
#define U64_MOD(a)                         ((a) & (U64_BIT_COUNT-1))
#define U64_COUNT_FLOOR(a)                 ((a)>>U64_SHIFT)
#define U64_COUNT_CEIL(a)                  U64_COUNT_FLOOR(((a)+U64_BIT_COUNT-1))
#define U64_ALIGN_PTR(a)                   (!((u64)(a) & ((U64_BIT_COUNT>>U8_SHIFT) - 1)))

#ifndef __attribute__
#define __attribute__(a)
#endif //__attribute__

#ifdef BE_PROCESSOR
#define ON_BE_SWAP16(a)      SWAP16(a)
#define ON_BE_SWAP32(a)      SWAP32(a)
#define ON_BE_SWAP64(a)      SWAP64(a)
#define ON_LE_SWAP16(a)
#define ON_LE_SWAP32(a)
#define ON_LE_SWAP64(a)
#else
#define ON_BE_SWAP16(a)
#define ON_BE_SWAP32(a)
#define ON_BE_SWAP64(a)
#define ON_LE_SWAP16(a)      SWAP16(a)
#define ON_LE_SWAP32(a)      SWAP32(a)
#define ON_LE_SWAP64(a)      SWAP64(a)
#endif

#define SWAP64(a)                                                                                 \
             do {                                                                                 \
                a = ((a >> 8)  & 0x00ff00ff00ff00ffull) | ((a << 8  ) & (0xff00ff00ff00ff00ull)); \
                a = ((a >> 16) & 0x0000ffff0000ffffull) | ((a << 16 ) & (0xffff0000ffff0000ull)); \
                a = ((a >> 32) & 0x00000000ffffffffull) | ((a << 32 ) & (0xffffffff00000000ull)); \
            } while (0)

#define SWAP32(a)                           a = (((((a)>>24) & 0xff) << 0)  |\
                                                 ((((a)>>16) & 0xff) << 8)  |\
                                                 ((((a)>>8 ) & 0xff) << 16) |\
                                                 ((((a)>>0 ) & 0xff) << 24))

#define SWAP16(a)                           a = (((((a)>>8) & 0xff) << 0)  |\
                                                 ((((a)>>0) & 0xff) << 8))

#ifdef DEBUG_NSELIB
#define DEBUG_memset(mem, value, size)   memset(mem, value, size)
#else
#define DEBUG_memset(mem, value, size)   fillData(mem, size)
#endif
#ifdef __cplusplus
extern "C" {
#endif

#define offset_of(type,member)   ((size_t)(&((type*)0)->member))
#define container_of(ptr, type, member)  ((type*)((char*)(ptr) - offset_of(type,member)))

API TcError ValidateNullPointer(const void* buff);

/*
**  
**  int  Index : ... [ int 1                                                                                         ][ int 0                                                                               ]
**  bit  Index : ... [31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0][31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0]
**  bit        : ...  63 62 61 60 59 58 57 56 55 54 53 52 51 50 49 48 47 46 45 44 43 42 41 40 39 38 37 36 35 34 33 32  31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
*/

// BitBuffer<...> format:
// BYTES    ...[   3    ][    2   ][   1    ][    0   ]
//          ...[11111111][11111111][00000000][00000000]
// BITS :   ...[FEDCBA98][76543210][FEDCBA98][76543210]
//                      bitStart <--------------------
//         bitCount  <-----------
//
API TcError BitBufferSetBit(void* buff, s32 buffSize, s32 bit);
API TcError BitBufferSetBits(void* buff, s32 buffSize, s32 bitStart, s32 bitCount, const void* in, s32 inSize);
API TcError BitBufferClearBit(void* buff, s32 buffSize, s32 bit);
API TcError BitBufferClearBits(void* buff, s32 buffSize, s32 bitStart, s32 bitCount);
API TcError BitBufferGetBit(const void* buff, s32 buffSize, s32 bit, s8* bitValue);
API TcError BitBufferGetBits(const void* buff, s32 buffSize, s32 bitStart, s32 bitCount, void* out, s32 outSize);
API TcError BitBufferAsLittleEndian16(const void* buff, s32 buffSize, s32 bitStart, s16* pResult);
API TcError BitBufferAsLittleEndian32(const void* buff, s32 buffSize, s32 bitStart, s32* pResult);
API TcError BitBufferAsBitEndian8(char* buff, s32 bitSize);
API TcError BitBufferAsBigEndian16(const void* buff, s32 buffSize, s32 bitStart, s16* pResult);
API TcError BitBufferAsBigEndian32(const void* buff, s32 buffSize, s32 bitStart, s32* pResult);
API TcError BitBufferIsAllOne(const void* in, s32 inSize, s32 width, s8* pResult);
API TcError BitBufferIsAllZero(const void* in, s32 inSize, s32 width, s8* pResult);
API TcError BitBufferAND(void* dst, const void* src, s32 width);
API TcError BitBufferXOR(void* dst, const void* src, s32 width);
API TcError BitBufferOR(void* dst, const void* src, s32 width);
API TcError BitBufferShiftRight(const void* in, s32 width, s32 cnt);
API TcError BitBufferShiftLeft(const void* in, s32 width, s32 cnt);
API TcError BitBufferStr(const void* in, s32 inSize, char* out, s32 outSize);
API TcError BitBufferStrWithBitCnt(const void* in, s32 bitCnt, char* out, s32 outSize);
API TcError BitBufferCountOne(const void* in, s32 inSize, s32 width, int* count);
API TcError BitBufferCountZero(const void* in, s32 inSize, s32 width, int* count);
API TcError BitBufferCountLeadingZero(const void* in, s32 inSize, s32 width, int* count);
API TcError BitBufferStrToBinary(char* key, s32* count);       // convert ascii string of 0..9A..F to binary equivalent
API TcError BitBufferToBinStr(const char* buffer, int bufferSize, int width, char* out, int* outSize);  
API TcError BitDigitToBinStr(const char* buffer, int bufferSize, int width, char* out, int* outSize);
API TcError BitBufferValidateHexaDecimalStr(const char* buffer);
API TcError EccAppend(void* buffer, int bufferSize, int* pWidth);
API int EccWidth(int dataWidth);
API TcError EccPriority(u8 memType, u8 module, int* priority);
API TcError EccRegWrite(void* buffer, int bufferSize, u8 module, u16 address, u8 memType, u8 eccStatus);
API TcError EccRegRead(const void* buffer, int bufferSize, u8 *module, u16 *address, u8* memType, u8* eccStatus);
API TcError EccRegReset(void* buffer, int bufferSize);
API TcError EccRegCheckAndUpdate(void* buffer, int bufferSize, u8 newFlag, u8 newModule, u16 newAddr, u8 newMemType, u8 isModuleSignicant, int* isUpdated);
API char *strRev(char *str);
API void digitRev(char* src, char* des, int length);
API TcError BitBufferStrToBinStr(const char *buff, char *out, s32 outputSize);
API TcError BitBufferBinStrToHex(char *buffBitStr, u32 bitStrLen, char *buffHexStr, u32 buffHexStrSize);
API char __Ch2Num(char ch);
API int __Chs2Num(char* chs, int width);
API TcError BitBufferToBigEndianBitBuffer(const void* littleBuffer, int width, void* bigBuffer, int bigBufferWidth);
//API TcError createRDStr(char *ldOutput, s32 octopus, s32 module, s32 memType, s32 address);
//API TcError createEccStr(char *ldOutput, s32 module, s32 address, s32 memType, s32 flag, s32 octopus);
//API TcError calcEccBits(char *buffReadOutput, char *buffReadOutputIlk, s32 outputSize, s32 memType, s16 ramReadRegisterBit);

API TcError BigEndianBitBufferToBitBuffer(const void* bigEBuffer, int bigEBufferWidth, void* littleEBuffer, int littleEBufferWith);
API TcError BigEndianBitBufferStr(const void* bigEBuffer, int width, char* outStr, int outStrSize);
API TcError BigEndianBitBufferStrToBinary(void* bigEBuffer, s32* count);

API TcError createEccBin(void* eccBuff, int* pEccBuffSize, s32 module, s32 address, s32 memType, s32 flag, s32 octopus);

API TcError StrAppend(char* outStr, const int outStrSize, int* pUsed, const char* newStr);
API void fillData(void* buff, int size);
API TcError ByteBitsReversal(u8* buff);
API int BitBufferCmp(const void *in1, const void *in2, s32 start, s32 bitCnt);

// BEBitBuffer<...> format:
// BYTES    [   0    ][    1   ][   2    ][    3   ]...
//          [00000000][00000000][11111111][11111111]...
// BITS:    [01234567][89ABCDEF][01234567][89ABCDEF]...
//           ---------------------------------> WIDTH
//           -----------> bitStart
//                       -----------> bitCount
//
API TcError BEBitBufferSetBit(void* buff, s32 buff_width, s32 bit);
API TcError BEBitBufferSetBits(void* buff, s32 buff_width, s32 bitStart, s32 bitCount, const void* in);
API TcError BEBitBufferClearBit(void* buff, s32 buff_width, s32 bit);
API TcError BEBitBufferClearBits(void* buff, s32 buff_width, s32 bitStart, s32 bitCount);
API TcError BEBitBufferGetBit(const void* buff, s32 buff_width, s32 bit, s8* bitValue);
API TcError BEBitBufferGetBits(const void* buff, s32 buff_width, s32 bitStart, s32 bitCount, void* out);
API TcError BEBitBufferXOR(void* dst, const void* src, s32 width);
API int BEBitBufferCmp(const void *in1, const void *in2, s32 start, s32 bitCnt, s32 *pPrefixLen);
API TcError BEBitBufferCountLeadingZero(const void* in, s32 width, int *pCount);
API TcError BEBitBufferShiftLeft(const void *buff, s32 width, int bitCnt);

#ifdef __cplusplus
}
#endif

#endif /* __utils_h__ */
