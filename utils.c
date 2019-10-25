#ifdef __VXWORKS__
#include <memLib.h>
#else
#include <memory.h>
#endif
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "defines.h"
#include "TcTypes.h"

TcError ValidateNullPointer(const void* buff)
{
    if (NULL == buff)
    {
        printError(commonLibLog, "null pointer\n");
        return TcE_Null_Pointer;
    }
    return TcE_OK;
}

inline static TcError BitBufferValidate(const void* buff, s32 buffSize, s32 bit, s32 bitCount) __attribute__((always_inline));
static TcError BitBufferValidate(const void* buff, s32 buffSize, s32 bit, s32 bitCount)
{
    TcError rc = TcE_OK;
    int buffSizeBits = buffSize*BITS_PER_BYTE;
    if (NULL == buff)
    {
        printError(commonLibLog, "nul input buffer\n");
        rc = TcE_Invalid_Argument;
    }
    if (bit<0)
    {
        printError(commonLibLog, "negative bit start. %d\n", bit);
        rc = TcE_Invalid_Argument;
    }
    if (bit >= buffSizeBits)
    {
        printError(commonLibLog, "bit too large: %d max=%d\n", bit, buffSizeBits-1);
        rc = TcE_Invalid_Argument;
    }
    if ((bit + bitCount) > buffSizeBits)
    {
        printError(commonLibLog, "over-run detected: bit end=%d max=%d\n", bit+bitCount, buffSizeBits-1);
        rc = TcE_Invalid_Argument;
    }
    return rc;
}

static void __BitBufferClearBit(void* buff, s32 bit)
{
    s8* iBuff = (s8*)buff + bit/BITS_PER_BYTE;
    s32 iRemain  = bit % BITS_PER_BYTE;

    *iBuff  &= ~(1<<(iRemain));
}

TcError BitBufferClearBit(void* buff, s32 buffSize, s32 bit)
{
    TcError rc = BitBufferValidate(buff, buffSize, bit, 1);
    if (rc<0) goto done;

    __BitBufferClearBit(buff, bit);

done:
    return rc;
}

static void __BitBufferSetBit(void* buff, s32 bit)
{
    s8* iBuff = (s8*)buff + bit/BITS_PER_BYTE;
    s32 iRemain  = bit % BITS_PER_BYTE;

    *iBuff  |= 1<<(iRemain);
}

TcError BitBufferSetBit(void* buff, s32 buffSize, s32 bit)
{
    TcError rc = BitBufferValidate(buff, buffSize, bit, 1);
    if (rc<0) goto done;

    __BitBufferSetBit(buff, bit);

done:
    return rc;
}

static void __BitBufferClearBits8(void* bitBuff, s32 bitStart, s32 bitCount) {
    s8* buff = (s8*)bitBuff;
    const int eSize = sizeof(buff[0])*BITS_PER_BYTE;
    int pre = bitStart % eSize;
    int buffStart = bitStart / eSize;
    int buffEnd   = (bitStart + bitCount) / eSize;
    int post = (bitStart + bitCount) % eSize;
    int i;
    for (i=0; i<bitCount/eSize; i++) {
        if (pre) {
            buff[buffStart + i]     &=  ((1<<pre) - 1);
            buff[buffStart + i + 1] &=  ~((1<<pre) - 1);
        } else {
            buff[buffStart + i] = 0;
        }
    }
    if (bitCount % eSize) {
        if (buffEnd == (buffStart + i)) {
            if (pre) {
                buff[buffStart + i] &=  ~(((1<<post)-1) ^ ((1<<pre) - 1));
            } else {
                buff[buffStart + i] &= ~((1<<post)-1);
            }
        } else if (buffEnd == (buffStart + i + 1)) {
            buff[buffStart + i]     &=  ((1<<pre) - 1);
            if (post) {
                buff[buffStart + i + 1] &=  ~(((1<<post)-1));
            }
        }
    }
}

static void __BitBufferSetBits8(void* bitBuff, s32 bitStart, s32 bitCount, const unsigned char* data)
{
    s8* buff = (s8*)bitBuff;
    const int eSize = sizeof(buff[0])*BITS_PER_BYTE;
    int pre = bitStart % eSize;
    int buffStart = bitStart / eSize;
    int buffEnd   = (bitStart + bitCount) / eSize;
    int post = (bitStart + bitCount) % eSize;
    int i;
    if(buffStart == buffEnd) {
        unsigned char tmps=data[0], tmpd=buff[buffStart];
        for(i = pre; i < post; i++) {
            if(tmps & 0x1)  { tmpd |= (1 << i);}
            else            { tmpd &= ~(1 << i);}
            tmps = tmps >> 1;
        }
        buff[buffStart] = tmpd;
    } else {
        if(pre != 0 ) {
            unsigned char *pdata = data;
            unsigned char *pbuff = buff+buffStart;
            *pbuff  = ((*pbuff) & ((1<<pre)-1)) | ((*pdata) << pre) ;
            pbuff++;
            for( ; pbuff < buff+buffEnd; pbuff++, pdata++) {
                *pbuff = ((*(pdata+1))<<pre) | ((*pdata)>>(eSize-pre));
            }
            if(post > pre) {
                unsigned char tmp = ((*(pdata+1)<<pre) | ((*pdata)>>(eSize-pre)))&((1<<post)-1);
                *pbuff =  ((*pbuff) & ~((1<<post)-1)) | tmp;
            }else if(post > 0){
                *pbuff = ((*pbuff) & ~((1<<post)-1)) | (((*pdata)>>(eSize-pre))&((1<<post)-1));
            }
        }else{
            memcpy(buff+buffStart, data, buffEnd-buffStart);
            if(post != 0) {
                buff[buffEnd] =  (buff[buffEnd] & ~((1<<post)-1)) | (data[buffEnd-buffStart] & ((1<<post)-1));
            }
        }
    }
}
    
//static void __BitBufferSetBits(void* bitBuff, s32 bitStart, s32 bitCount, const void* in)
//{
//    s32* buff = (s32*)bitBuff;
//    const s32* data = (const s32*)in;
//    const int eSize = sizeof(buff[0])*BITS_PER_BYTE;
//    int pre = bitStart % eSize;
//    int buffStart = bitStart / eSize;
//    int i;
//
//    for (i=0; i<bitCount/eSize; i++)
//    {
//        if (pre)
//        {
//            buff[buffStart + i]     &=  ((1<<pre) - 1);
//            buff[buffStart + i]     |=  (data[i] << pre) ;
//            buff[buffStart + i + 1] &=  ~((1<<pre) - 1);
//            buff[buffStart + i + 1] |=  (data[i]>>(eSize - pre)) & ((1<<pre)-1);
//        }
//        else
//        {
//            buff[buffStart + i] = data[i];
//        }
//    }
//
//    if (bitCount%eSize)
//    {
//        const char* in8 = (const char*)(&data[i]);
//        int bitStart8 = bitStart + i*eSize;
//        int bitCount8 = bitCount - i*eSize;
//        __BitBufferSetBits8(bitBuff, bitStart8, bitCount8, in8);
//    }
//}

TcError BitBufferSetBits(void* buff, s32 buffSize, s32 bitStart, s32 bitCount, const void* in, s32 inSize)
{
    TcError rc = TcE_OK;

    if (( rc = BitBufferValidate(buff, buffSize, bitStart, bitCount)) != 0)
           goto done;

    if (bitCount > (inSize * BITS_PER_BYTE))
    {
        printError(commonLibLog, "%s: ERROR: bit count too large. %d max=%d\n", __FUNCTION__, bitCount, inSize * BITS_PER_BYTE);
        rc = TcE_Invalid_Argument; goto done;
    }
    if (NULL == in)
    {
        printError(commonLibLog, "%s: ERROR: null output buffer\n", __FUNCTION__);
        rc = TcE_Invalid_Argument; goto done;
    }

    __BitBufferSetBits8(buff, bitStart, bitCount, in);

done:
    return rc;
}

TcError BitBufferClearBits(void* buff, s32 buffSize, s32 bitStart, s32 bitCount) {
    TcError rc = TcE_OK;
    if ((rc = BitBufferValidate(buff, buffSize, bitStart, bitCount)) != 0) { goto done; }
    __BitBufferClearBits8(buff, bitStart, bitCount);
done:
    return rc;
}

static void __BitBufferGetBit(const void* buff, s32 bit, s8* pBitValue)
{
    const s8* iBuff = (const s8*)buff;
    int  eSize = sizeof(iBuff[0])*BITS_PER_BYTE;
    s32 eIndex = bit / eSize;
    s32 eRemain  = bit % eSize;

    *pBitValue = (iBuff[eIndex] & (1<<(eRemain))) 
                   ? 1 
                   : 0;
}

TcError BitBufferGetBit(const void* buff, s32 buffSize, s32 bit, s8* pBitValue)
{
    TcError rc = BitBufferValidate(buff, buffSize, bit, 1);
    if (rc<0) goto done;

    __BitBufferGetBit(buff, bit, pBitValue);

done:
    return rc;
}

static void __BitBufferGetBits8(const void* bitBuff, s32 bitStart, s32 bitCount, char* out)
{
    s8* in = (s8*)bitBuff;
    const int eSize = sizeof(in[0])*BITS_PER_BYTE;
    int pre = bitStart % eSize;
    int inStart = bitStart / eSize;
    int inEnd = (bitStart + bitCount) / eSize;
    int pos = (bitStart + bitCount) % eSize;
    int i;

    for (i=0; i<bitCount/eSize; i++)
    {
        if (pre)
        {
            out[i] =   (((in[inStart + i]) >> pre) & ((1<<(eSize - pre)) - 1))
                     | (((in[inStart + i + 1]) << (eSize - pre)));
        }
        else
        {
            out[i] = in[inStart + i];
        }
    }

    if (bitCount % eSize)
    {
        if (inEnd == (inStart + i))
        {
            out[i] = ((in[inStart + i]) >> pre) & ((1<<(pos-pre)) - 1);
        }
        else if (inEnd == (inStart + i + 1))
        {
            out[i] =   (((in[inStart + i]) >> pre) & ((1<<(eSize - pre)) - 1)) 
                     | (((in[inEnd]) & ((1<<pos) - 1))  << (eSize - pre));
        }
    }
}

//static void __BitBufferGetBits(const void* bitBuff, s32 bitStart, s32 bitCount, void* oBuff)
//{
//    s32* in = (s32*)bitBuff;
//    s32* out = (s32*)oBuff;
//    const int eSize = sizeof(in[0])*BITS_PER_BYTE;
//    int pre = bitStart % eSize;
//    int inStart = bitStart / eSize;
//    int i;
//
//    for (i=0; i<bitCount/eSize; i++)
//    {
//        if (pre)
//        {
//            out[i] =   (((in[inStart + i]) >> pre) & ((1<<(eSize - pre)) - 1)) 
//                     | (((in[inStart + i + 1]) << (eSize - pre)));
//        }
//        else
//        {
//            out[i] = in[inStart + i];
//        }
//    }
//
//    if (bitCount % eSize)
//    {
//        int bitStart8 = bitStart + i*eSize;
//        int bitCount8 = bitCount - i*eSize;
//        char* oBuf8 = (char*)(&out[i]);
//        __BitBufferGetBits8(bitBuff, bitStart8, bitCount8, oBuf8);
//    }
//}

TcError BitBufferGetBits(const void* buff, s32 buffSize, s32 bitStart, s32 bitCount, void* out, s32 outSize)
{
    TcError rc = BitBufferValidate(buff, buffSize, bitStart, bitCount);
    if (rc<0) goto done;

    if (bitCount > (outSize * BITS_PER_BYTE))
    {
        printError(commonLibLog, "bit count too large. %d max=%d\n", bitCount, outSize * BITS_PER_BYTE);
        rc = TcE_Invalid_Argument;
        goto done;
    }
    if (NULL == out)
    {
        printError(commonLibLog, "null output buffer\n");
        rc = TcE_Invalid_Argument;
        goto done;
    }

    memset(out, 0, outSize);

    __BitBufferGetBits8(buff, bitStart, bitCount, out);

done:
    return rc;
}


TcError BitBufferAsLittleEndian16(const void* buff, s32 buffSize, s32 bitStart, s16* pResult)
{
    TcError rc = BitBufferGetBits(buff, buffSize, bitStart, sizeof(pResult[0])*BITS_PER_BYTE, (char*)pResult, sizeof(pResult[0]));
    return rc;
}

TcError BitBufferAsLittleEndian32(const void* buff, s32 buffSize, s32 bitStart, s32* pResult)
{
    TcError rc = BitBufferGetBits(buff, buffSize, bitStart, sizeof(pResult[0])*BITS_PER_BYTE, (char*)pResult, sizeof(pResult[0]));
    return rc;
}

TcError BitBufferAsBitEndian8(char* buff, s32 bitSize)
{
    int i, j;
    for (i=0; i<BYTE_COUNT(bitSize); i++)
    {
        char byte = 0;
        for (j=0; j<8; j++)
        {
            int idx = 8*i + j;
            char value = (idx < bitSize) 
                            ? buff[idx] - '0' 
                            : 0;
            byte = byte<<1 | value;
        }
        buff[i] = byte;
    }
    buff[i] = 0;
    return TcE_OK;
}

TcError BitBufferAsBigEndian16(const void* buff, s32 buffSize, s32 bitStart, s16* pResult)
{
    s16 result;
    TcError rc = BitBufferGetBits(buff, buffSize, bitStart, sizeof(result)*BITS_PER_BYTE, (char*)&result, sizeof(result));
    if (rc<0) goto done;

    result = ((result & 0xff) << 8 ) 
             | ((result & 0xff00) >> 8);

    *pResult = result;

done:
    return rc;
}

TcError BitBufferAsBigEndian32(const void* buff, s32 buffSize, s32 bitStart, s32* pResult)
{
    s32 result;
    TcError rc = BitBufferGetBits(buff, buffSize, bitStart, sizeof(result)*BITS_PER_BYTE, (char*)&result, sizeof(result));
    if (rc<0) goto done;

    result = ((result & 0xff) << 24)        |
             ((result & 0xff00) << 8)       |
             ((result & 0xff0000) >> 8)     |
             ((result & 0xff000000) >> 24);

    *pResult = result;

done:
    return rc;
}

TcError BitBufferStr(const void* in, s32 inSize, char* out, s32 outSize)
{
    int i;
    if (outSize < (inSize*2+1))
    {
        printError(commonLibLog, "out buffer too small. expect: %X actual: %X\n", inSize*2+1, outSize);
        return TcE_Buffer_Too_Small;
    }

    for (i=0; i<inSize; i++)
    {
        sprintf(&out[i*2],"%02X", ((char*)in)[inSize - 1 - i] & 0xff); 
    }
    out[i*2] = 0;
    return TcE_OK;
}

TcError BitBufferStrWithBitCnt(const void* in, s32 inBitCnt, char* out, s32 outSize)
{
    TcError rc = TcE_OK;
    int outputLen = (inBitCnt + 3) / 4 + 1;   // 4bit per char + 1 null terminal
    int offset = 0;
    if (outSize < outputLen)
    {
        printError(commonLibLog, "out buffer too small. expect: %X actual: %X\n", outputLen, outSize);
        return TcE_Buffer_Too_Small;
    }
   
    offset = 0; 
    if (inBitCnt % 8) 
    {
        const u8* msb = (const u8*)in + inBitCnt / 8;
        int remainBits = inBitCnt % 8;
        int mask = ((1<<remainBits) - 1);
        char msbValue = (*msb) & mask;
        offset = sprintf(out, "%X", msbValue);
    }

    if (0 < (rc = BitBufferStr(in, inBitCnt/8, out+offset, outSize - offset)))
        return rc;

    return TcE_OK;
}

static void __BitBufferIsAllOne(const void* in, s32 width, s8* pResult)
{
    const s32* iBuff = (const s32*)in;
    int eSize = sizeof(iBuff[0])*BITS_PER_BYTE;
    s32 eCount  = width / eSize;
    s32 eRemain = width % eSize;
    s32 i;
    
    *pResult = 1;  /* assume true until found otherwise */
    for (i=0; i<eCount; i++)
    {
        if (iBuff[i] != (~0))
        {
            *pResult = 0;
            goto done;
        }
    }
    if (eRemain)
    {
       s32 mask = (1<<eRemain) - 1;
       ON_BE_SWAP32(mask);
       if ((iBuff[i] & mask) != mask)
       {
           *pResult = 0;
           goto done;
       }
    }

done:
    return;
}

TcError BitBufferIsAllOne(const void* in, s32 inSize, s32 width, s8* pResult)
{
    TcError rc = TcE_OK;
    if ((inSize*BITS_PER_BYTE) < width)
    {
        printError(commonLibLog, "in buffer size too small. %d min=%d\n", inSize, width);
        rc = TcE_Invalid_Argument;
        goto done;
    }

    __BitBufferIsAllOne(in, width, pResult);

done:
    return rc;
}

static void __BitBufferIsAllZero(const void* in, s32 width, s8* pResult)
{
    const s32* iBuff = (const s32*)in;
    int eSize = sizeof(iBuff[0])*BITS_PER_BYTE;
    s32 eCount = width / eSize;
    s32 eRemain = width % eSize;
    s32 i;
    
    *pResult = 1;  /* assume true until found otherwise */
    for (i=0; i<eCount; i++)
    {
        if (iBuff[i] != 0)
        {
            *pResult = 0;
            goto done;
        }
    }
    if (eRemain)
    {
       s32 mask = (1<<eRemain) - 1;
       ON_BE_SWAP32(mask);
       if ((iBuff[i] & mask) != 0)
       {
           *pResult = 0;
           goto done;
       }
    }

done:
    return;
}

TcError BitBufferIsAllZero(const void* in, s32 inSize, s32 width, s8* pResult)
{
    TcError rc = TcE_OK;
    if (inSize*BITS_PER_BYTE < width)
    {
        printError(commonLibLog, "in buffer size too small. %d min=%d\n", inSize, width);
        rc = TcE_Invalid_Argument;
        goto done;
    }

    __BitBufferIsAllZero(in, width, pResult);

done:
    return rc;
}

static s32 countOneMask[] =
{
    0x55555555,
    0x33333333,
    0x0F0F0F0F,
    0x00FF00FF,
    0x0000FFFF,
};

TcError BitBufferCountOne(const void* in, s32 inSize, s32 width, s32* count)
{
    int i;
    int oneCount = 0;
    const int* iBuff = (const int*)in; 
    int iSize = width / (BITS_PER_S32);
    int byteRemain = (width % (BITS_PER_S32));
    const char* byteBuff = (const char*)(&iBuff[iSize]);
    int byteSize = byteRemain / BITS_PER_BYTE;
    int bitRemain = byteRemain % BITS_PER_BYTE;
    const char* lastByte = &byteBuff[byteSize];

    if ((inSize*BITS_PER_BYTE) < width)
    {
        printError(commonLibLog, "buff too small. %d  min=%d\n", inSize, width / BITS_PER_BYTE);
        return TcE_Invalid_Argument;
    }
    for (i=0; i<iSize; i++)
    {
        int j;
        int value = iBuff[i];
        for (j=0; (1<<j) < BITS_PER_S32; j++)
        {
            int shift = 1<<j;

            value =  ( value & countOneMask[j] ) +
                     (( value >> shift) & countOneMask[j]); 
        }
        oneCount += value;
    }
    for (i=0; i<byteSize; i++)
    {
        u32 j;
        char value = byteBuff[i];
        for (j=0; (u32)(1<<j) < sizeof(char)*BITS_PER_BYTE; j++)
        {
            int shift = 1<<j;

            value =  ( value & (char)countOneMask[j] ) +
                        (( value >> shift) & (char) countOneMask[j]); 
        }
        oneCount += value;
    }
    if (bitRemain)
    {
        char value = *lastByte & ((1<<bitRemain) - 1);
        u32 j;
        for (j=0; (u32)(1<<j) < sizeof(char)*BITS_PER_BYTE; j++)
        {
            int shift = 1<<j;

            value =  ( value & (char)countOneMask[j] ) +
                        (( value >> shift) & (char) countOneMask[j]); 
        }
        oneCount += value;
    }

    *count = oneCount;
    return TcE_OK;
}

TcError BitBufferCountLeadingZero(const void* in, s32 inSize, s32 width, int* count) {
    const char* iBuff = (const char*)in; 
    int iSize = (sizeof(*iBuff)*BITS_PER_BYTE), i, j, byte_cnt = width/iSize, zero_cnt, end_bit;
    for (i = 0; i < byte_cnt; i++) {
        if (iBuff[i]) break;
    }
    zero_cnt = i*iSize;
    if (i==byte_cnt) { 
        if ((width % iSize)==0) { *count = zero_cnt; return TcE_OK; }
        end_bit = width % iSize; // scan the rest of bits
    } else {
        end_bit = iSize;  // scan entire byte
    }
    for (j = 0; j < end_bit; j++) {
        if (iBuff[i] & (1<<j)) break;
        zero_cnt++;
    }
    *count = zero_cnt;
    return TcE_OK;
}

TcError BitBufferShiftRight(const void* in, s32 width, s32 shift) {
    TcError rc = TcE_OK;
    u8 bit;
    int i;
    assertLog(shift <= width);
    for (i = 0; i < (width - shift); i++) {
        CHECK_ERROR(commonLibLog, BitBufferGetBit(in, BYTE_COUNT(width), i+shift, &bit));
        CHECK_ERROR(commonLibLog, BitBufferSetBits(in, BYTE_COUNT(width), i, 1, &bit, sizeof(bit)));
    }
    for (; i < width; i++) {
        CHECK_ERROR(commonLibLog, BitBufferClearBit(in, BYTE_COUNT(width), i));
    }
done:
    return rc;
}

TcError BitBufferShiftLeft(const void* in, s32 width, s32 shift) {
    TcError rc = TcE_OK;
    u8 bit;
    int i;
    assertLog(shift <= width);
    for (i = width - 1; i >= shift; i--) {
        CHECK_ERROR(commonLibLog, BitBufferGetBit(in, BYTE_COUNT(width), i - shift, &bit));
        CHECK_ERROR(commonLibLog, BitBufferSetBits(in, BYTE_COUNT(width), i, 1, &bit, sizeof(bit)));
    }
    for (; i >= 0; i--) {
        CHECK_ERROR(commonLibLog, BitBufferClearBit(in, BYTE_COUNT(width), i));
    }
done:
    return rc;
}

TcError BitBufferCountZero(const void* in, s32 inSize, s32 width, s32* count) {
    TcError rc = TcE_OK;
    int oneCnt;
    CHECK_ERROR(commonLibLog, BitBufferCountOne(in, inSize, width, &oneCnt));
    *count = width - oneCnt;
done:
    return rc;
}


char __Ch2Num(char ch)
{
    char i = 0;
    if ((ch >= '0') && (ch <= '9'))
    {
        i = ch - '0';
    }
    else if ((ch >= 'A') && (ch <= 'F'))
    { 
        i = ch - 'A' + 10;
    }
    else if ((ch >= 'a') && (ch <= 'f'))
    {
        i = ch - 'a' + 10;
    }
    return i;
}

int __Chs2Num(char* chs, int width)
{
    int i;
    int num = 0;
    for ( i=0; i<width; i++ )
    {
        num = (num<<4) + __Ch2Num(*(chs+i));
    }
    return num;
}

TcError BitBufferStrToBinary(char* key, s32* count)
{
    TcError rc = TcE_OK;
    s32  len = strlen(key);
    s32  i;
    s32  offset = 0;
    s32 nonZeroByte;
    s32 remainingDigit;
    char tmp[MAX_BYTES];

    if (NULL == key) 
    { 
        printError(commonLibLog, "null output pointer key\n",__FUNCTION__);
        rc = TcE_Invalid_Argument; goto done; 
    }
    if (NULL == count) 
    { 
        printError(commonLibLog, "null output pointer count\n",__FUNCTION__);
        rc = TcE_Invalid_Argument; goto done; 
    }
    if (len == 0)
    {
        printError(commonLibLog, "empty string not supported\n",__FUNCTION__);
        rc = TcE_Invalid_Argument; goto done; 
    }
    if (len > (s32)sizeof(tmp))
    {
        printError(commonLibLog, "len too large. %X max=%\n",__FUNCTION__,
            len, sizeof(tmp));
        rc = TcE_Invalid_Argument; goto done; 
    }

    CHECK_ERROR(commonLibLog,
        BitBufferValidateHexaDecimalStr(key));

    memcpy(tmp, key, len + 1);

    offset = 0;

    nonZeroByte  = (len - offset) / 2;
    remainingDigit = (len - offset )% 2;
    for (i=0; i<nonZeroByte; i++)
    {
        s8 byte =   __Ch2Num(tmp[(len - 1) - i*2]) 
                  | __Ch2Num(tmp[(len - 1) - i*2 - 1]) << 4;
        key[i] = byte;
    }
    if (remainingDigit)
    {
        key[i] = __Ch2Num(tmp[len - 1 - i*2]);
    }

    *count = nonZeroByte + remainingDigit; 

done:
    return rc;
}

TcError BitBufferToBinStr(const char* buffer, int bufferSize, int width, char* out, int* outSize)
{
    int i;
    
    if (*outSize < (width + 1))
    {
        printError(commonLibLog, "out put buffer size too small. %X need >= %X\n", outSize, width + 1);
        *outSize = width + 1;
        return TcE_Buffer_Too_Small;
    }

    *outSize = width + 1;

    for (i=0; i<width; i++)
    {
        s8 bit;
        BitBufferGetBit(buffer, bufferSize, width - 1 -i, &bit);
        out[i] = (bit) ? '1' : '0';
    }
    out[width]=0;

    return TcE_OK;
}

TcError BitDigitToBinStr(const char* buffer, int bufferSize, int width, char* out, int* outSize)
{
    int i;
 
    if ((outSize) &&
        (BYTE_COUNT(width) > *outSize))
    {
        printError(commonLibLog, "output buffer too small. expect >= %X   actual=%X\n",
                    BYTE_COUNT(width), *outSize);
        return TcE_Buffer_Too_Small;
    } 
    for (i=0; i<width; i++)
    {
        s8 bit;
        BitBufferGetBit(buffer, bufferSize, width - 1 -i, &bit);
        out[i] = (bit) ? 1 : 0;
    }
    out[width]=0;

    return TcE_OK;
}

TcError BitBufferValidateHexaDecimalStr(const char* buffer)
{
    TcError rc = TcE_OK;
    const char* p = buffer;

    while (*p)
    { 
        if (((*p >= '0') && (*p <= '9'))
            ||
            ((*p >= 'A') && (*p <= 'F'))
            ||
            ((*p >= 'a') && (*p <= 'f')))
        {
            p++;
        }
        else { break; }
    }
    if (0 != *p) 
    {
        printError(commonLibLog, "Invalid byte: %c (%X)\n",*p, *p & 0xff);
        rc = TcE_Tpf_Parser_Invalid_LC; 
        goto done; 
    }

done: 
    return rc;
}

char *strRev(char *str)
{
	char *p1, *p2;
	
	if (!str || ! *str)
		return str;

	for (p1 = str, p2=str + strlen(str) - 1; p2 > p1; ++p1, --p2)
	{
		*p1 ^= *p2;
		*p2 ^= *p1;
		*p1 ^= *p2;
	}

	return str;
}

void digitRev(char* src, char* des, int length)
{
    int i;
    for (i=0; i<length; i++)
    {
        des[i] = src[length-i-1];
    }
}

static int hex2val(char p)
{
    if ((p >= '0') && (p <= '9'))   
        return p - '0';

    if ( (p >= 'A') && (p <= 'F'))
        return 10 + p - 'A';

    if ( (p >= 'a') && (p <= 'f'))
        return 10 + p - 'a';

    return -1;

}

// converts HEX char string to BIN char string
TcError BitBufferStrToBinStr(const char *buff, char *out, s32 outputSize)
{
	char buffer[MAX_CHARS] = {0};
    int bufferSize = MAX_CHARS, buffLen;
    int rc = TcE_OK;
    const char* p;
    int i, j;
    int offset;

	buffLen = strlen(buff);
	buffer[bufferSize-1] = 0;
    p = &(buff[buffLen - 1]);
	j = buffLen - 1;
    offset = 0;

    while (j >= 0)
    {
        int val = hex2val(*p);
        if (val < 0)
        {
            printError(commonLibLog, "invalid hex digit %c", *p);
            rc = TcE_Failed;
            goto done;
        }
        for (i = (BITS_PER_HEXCHAR - 1); i >= 0 && offset < outputSize; i--, offset++)
        {
            if ((1<<(BITS_PER_HEXCHAR - 1 - i))&val) 
				buffer[offset] = 1;
            else
                buffer[offset] = 0;
        }
        p--;
		j--;
    }

	memset(out, 0, outputSize);
	memcpy(out, buffer, outputSize);

done:
    return rc;
}

TcError BitBufferBinStrToHex(char *buffBitStr, u32 bitStrLen, char *buffHexStr, u32 buffHexStrSize)
{
	TcError rc = TcE_OK;
	char buffHex[MAX_CHARS] = {0};
	u32 totalBits = 0;
	u32 i, j;
	int k;
	u8 hexVal;

	if (buffHexStrSize == 0)
		buffHexStrSize = (bitStrLen+3)>>2;

	if (buffBitStr == NULL || buffHexStr == NULL)
	{
		printError(commonLibLog, "null pointer\n");
		rc = TcE_Null_Pointer;
		goto done;
	}

	if ((buffHexStrSize * BITS_PER_HEXCHAR) < bitStrLen)
	{
		printError(commonLibLog, "output buff size %d is smaller than input buff size %d\n", (buffHexStrSize * BITS_PER_HEXCHAR), bitStrLen);
		rc = TcE_Out_Of_Range;
		goto done;
	}

	memset(buffHex, 0, sizeof(buffHex));
	totalBits = (buffHexStrSize * BITS_PER_HEXCHAR);

	for (j = 0, k = (totalBits - 1); 
         (k >= 0) && (j < buffHexStrSize); 
         j++)
	{
		for (i = 0, hexVal = 0; i < BITS_PER_HEXCHAR && k >= 0; i++)
		{
			if (k < (int)bitStrLen)
				hexVal = hexVal << 1 | buffBitStr[k];
			else
				hexVal = hexVal << 1 | 0;
			k--;
		}
		sprintf(buffHex + j, "%1X", hexVal);
	}

    buffHex[j] = '\0';

	if (strlen(buffHex) > buffHexStrSize)
	{
		printError(commonLibLog, "output buff len %d is not matching expected buff size %d\n", strlen(buffHex), buffHexStrSize);
		rc = TcE_Failed;
		goto done;
	}
	else
		buffHexStrSize = strlen(buffHex);

	strcpy(buffHexStr, buffHex);

done:
	if (rc<0) printError(commonLibLog, "converting bit string to hex failed. rc : %X\n", rc);

	return rc;
}

TcError BitBufferAND(void* dst, const void* src, s32 width) {
    TcError rc = TcE_OK;
    s64* dst64 = (s64*)dst;
    const s64* src64 = (const s64*)src;
    char buf[2*BYTE_COUNT(640)+1];
    s32 eSize = sizeof(dst64[0]);
    s32 count = BYTE_COUNT(width)/eSize;
    s32 remain = BYTE_COUNT(width)%eSize;
    s32 i;
    if ((NULL==dst)||(NULL==src)) {
        if (NULL==dst) printError(commonLibLog, "dst is null\n");
        if (NULL==src) printError(commonLibLog, "src is null\n");
        rc = TcE_Invalid_Argument; goto done;
    }
    //BigEndianBitBufferStr(dst, width, buf, sizeof(buf));
    //printLog(commonLibLog, "     dst=%s\n", buf);
    //BigEndianBitBufferStr(src, width, buf, sizeof(buf));
    //printLog(commonLibLog, " AND src=%s\n", buf);
    for (i = 0; i<count; i++) { dst64[i] &= src64[i]; }
    if (remain) {
        s8* dstRemain = (s8*)&dst64[count];
        const s8* srcRemain = (const s8*)&src64[count];
        for (i = 0; i<remain; i++) { dstRemain[i] &= srcRemain[i]; }
    }
    //BigEndianBitBufferStr(dst, width, buf, sizeof(buf));
    //printLog(commonLibLog, "     dst=%s\n", buf);
done:
    return rc;
}

const char* TcErrorName(TcError rc)
{
    if      (rc == (TcError)TcE_Alloc_Fail_Conflict)                     return "ERROR Alloc Fail Conflict";
    else if (rc == (TcError)TcE_Alloc_Fail_Space)                        return "ERROR Alloc Fail Space";
    else if (rc == (TcError)TcE_Alloc_Fail_Too_Many_Module)              return "ERROR Alloc Fail Too Many Module";
    else if (rc == (TcError)TcE_Buffer_Too_Small)                        return "ERROR Buffer Too Small";
    else if (rc == (TcError)TcE_Capacity_Reached)                        return "ERROR Capacity Reached";
    else if (rc == (TcError)TcE_EccReg_Invalid_Value)                    return "ERROR EccReg Invalid Value";
    else if (rc == (TcError)TcE_End_Of_Str_Reach)                        return "ERROR End Of Str Reach";
    else if (rc == (TcError)TcE_Failed)                                  return "ERROR Failed";
    else if (rc == (TcError)TcE_HRam_Undefined)                          return "ERROR HRam Undefined";
    else if (rc == (TcError)TcE_Ilk_Protocol)                            return "ERROR Ilk Protocol";
    else if (rc == (TcError)TcE_Invalid_Argument)                        return "ERROR Invalid Argument";
    else if (rc == (TcError)TcE_Invalid_File_Format)                     return "ERROR Invalid File Format";
    else if (rc == (TcError)TcE_Invalid_HexaDecimal_String)              return "ERROR Invalid HexaDecimal String";
    else if (rc == (TcError)TcE_Invalid_Pointer)                         return "ERROR Invalid Pointer";
    else if (rc == (TcError)TcE_Invalid_Xml_Str)                         return "ERROR Invalid Xml Str";
    else if (rc == (TcError)TcE_Logic_Error)                             return "ERROR Logic Error";
    else if (rc == (TcError)TcE_Module_Invalid_MemType)                  return "ERROR Module Invalid MemType";
    else if (rc == (TcError)TcE_Not_Found)                               return "ERROR Not Found";
    else if (rc == (TcError)TcE_Not_Implement)                           return "ERROR Not Implement";
    else if (rc == (TcError)TcE_Null_Pointer)                            return "ERROR Null Pointer";
    else if (rc == (TcError)TcE_OK)                                      return "OK";
    else if (rc == (TcError)TcE_Out_Of_Memory)                           return "ERROR Out Of Memory";
    else if (rc == (TcError)TcE_Out_Of_Range)                            return "ERROR Out Of Range";
    else if (rc == (TcError)TcE_Priority_Field_Invalid)                  return "ERROR Priority Field Invalid";
    else if (rc == (TcError)TcE_Priority_Line_Format_Inconsistent)       return "ERROR Priority Line Format Inconsistent";
    else if (rc == (TcError)TcE_Register_Undefine)                       return "ERROR Register Undefine";
    else if (rc == (TcError)TcE_TcRam_Address_Out_Of_Range)              return "ERROR TcRam Address Out Of Range";
    else if (rc == (TcError)TcE_TimeOut)                                 return "ERROR Time out";
    else if (rc == (TcError)TcE_Busy)                                    return "ERROR Busy";
    else if (rc == (TcError)TcE_Tpf_Parser_Invalid_Expected_Priority)    return "ERROR Tpf Parser Invalid Expected Priority";
    else if (rc == (TcError)TcE_Tpf_Parser_Invalid_LC)                   return "ERROR Tpf Parser Invalid LC";
    else if (rc == (TcError)TcE_Tpf_Parser_Invalid_LD)                   return "ERROR Tpf Parser Invalid LD";
    else if (rc == (TcError)TcE_Tpf_Parser_Invalid_Wr)                   return "ERROR Tpf Parser Invalid Wr";
    else if (rc == (TcError)TcE_Unsupport_Opcode)                        return "ERROR Unsupport Opcode";
    else if (rc == (TcError)TcE_XData_Depth_Mismatch)                    return "ERROR XData Depth Mismatch";
    else if (rc == (TcError)TcE_XData_Init_BUFFER_TOO_SMALL)             return "ERROR XData Init BUFFER TOO SMALL";
    else if (rc == (TcError)TcE_XData_Invalid_Hi)                        return "ERROR XData Invalid Hi";
    else if (rc == (TcError)TcE_Xdata_Invalid_Lo)                        return "ERROR Xdata Invalid Lo";
    else if (rc == (TcError)TcE_XData_Undefined)                         return "ERROR XData Undefined";
    else if (rc == (TcError)TcE_XData_Width_Mismatch)                    return "ERROR XData Width Mismatch";
    else if (rc == (TcError)TcW_Comment)                                 return "Warning Comment";
    else if (rc == (TcError)TcW_Tpf_Invalid)                             return "Warning Skip Invalid Tpf String";
    else if (rc == (TcError)TcW_Ecc_Ded)                                 return "Warning Ecc DED";
    else if (rc == (TcError)TcW_Ecc_Reg_Updated)                         return "Warning Ecc Reg Updated";
    else if (rc == (TcError)TcW_Ecc_Sec)                                 return "Warning Ecc SEC";
    else if (rc == (TcError)TcW_Empty_Line)                              return "Warning Empty Line";
    else if (rc == (TcError)TcW_Eof)                                     return "Warning Eof";
    else if (rc == (TcError)TcW_Ilk_Invalid)                             return "Warning Ilk Invalid String";
    else if (rc == (TcError)TcW_Ilk_Partial_Data)                        return "Warning Ilk Partial Data";
    else if (rc == (TcError)TcW_IoCmd_Opcode_Ignored)                    return "Warning IoCmd Opcode Ignored";
    else                                                                 return "Unknown error";
}

const char* MemType_name(u8 type)
{
    static char ramName[512];

    if ((type >= MemTypeDram1) && (type <= MemTypeDram40))
    {
        sprintf(ramName, "MemTypeDataRam_%X", type);
        return ramName;
    }
    else if (type == MemTypeHashRam)                return "MemTypeHashRam";
    else if (type == MemTypeBsRegister)             return "MemTypeBsRegister";
    else if (type == MemTypeHRamOffRegister)        return "MemTypeHRamOffRegister";
    else if (type == MemTypePriorityRam)            return "MemTypePriorityRam";
    else if (type == MemTypeXcRam)                  return "MemTypeXcRam";
    else if (type == MemTypeProfile)                return "MemTypeProfile";
    else if (type == MemTypeXcCtrlRegister)         return "MemTypeXcCtrlRegister";
    else if (type == MemTypeSearchBuffer0)          return "MemTypeSearchBuffer0";
    else if (type == MemTypeSearchBuffer1)          return "MemTypeSearchBuffer1";
    else if (type == MemTypeKguProfile0)            return "MemTypeKguProfile0";
    else if (type == MemTypeKguProfile1)            return "MemTypeKguProfile1";
    else if (type == MemTypeRangeEncoderRegister0)  return "MemTypeRangeEncoderRegister0";
    else if (type == MemTypeRangeEncoderRegister1)  return "MemTypeRangeEncoderRegister1";
    else if (type == MemTypeRamReadRegister)        return "MemTypeRamReadRegister";
    else if (type == MemTypeNone)                   return "MemTypeNone";
    else
    {
        sprintf(ramName, "MemTypeUNKNOWN %X", type);
        return ramName;
    }
}

const char* QuartetMemType_name(u8 type)
{
    static char ramName[512];

         if (type == QuartetMemTypeModuleConfigReg)        return "QuartetMemTypeModuleConfigReg";
    else if (type == QuartetMemTypeTcamReg)                return "QuartetMemTypeTcamReg";
    else if (type == QuartetMemTypeEccStatus)              return "QuartetMemTypeEccStatus";
    else if (type == QuartetMemTypeProfileRam)             return "QuartetMemTypeProfileRam";
    else if (type == QuartetMemTypeModuleRam)              return "QuartetMemTypeModuleRam";
    else if (type == QuartetMemTypeProfileReg)             return "QuartetMemTypeProfileReg";
    else if (type == QuartetMemTypeBPCntrReg)              return "QuartetMemTypeBPCntrReg";
    else if (type == QuartetMemTypeLRUCntrReg)             return "QuartetMemTypeLRUCntrReg";
    else
    {
        sprintf(ramName, "QuartetMemTypeUNKNOWN %X", type);
    }
    return ramName;
}



static int EccBitCount(int width)
{
    int eccBitCnt;
    int coveredDataBitCnt;

    for (eccBitCnt = 1, coveredDataBitCnt = 0; 
        coveredDataBitCnt < width; 
        eccBitCnt++)
    {
        coveredDataBitCnt = ( (1 << eccBitCnt) - 1 - eccBitCnt);
    }
    return eccBitCnt -1; // adjust for loop increment
}

TcError EccAppend(void* buffer, int bufferSize , int* pWidth)
{
    TcError rc = TcE_OK;
    static s16 indexes[MAX_CHARS];
    static int indexesCnt = 0;
    int dataBitCnt;
    int eccBitCnt;
    int hummingBit, dataBit, eccBit;
    s8 xorBitValue;
    //char dataBitsStr[MAX_CHARS] = {0};
    //char dataBitValuesStr[MAX_CHARS] = {0};

    assertLog(pWidth != NULL);

    dataBitCnt = *pWidth;
    eccBitCnt = EccBitCount(dataBitCnt);
    *pWidth = 0;

    if (bufferSize < (BYTE_COUNT(dataBitCnt + eccBitCnt + 1)))    // + 1 for xor bit
    {
        printError(commonLibLog, "out put buffer too small, %X expect >= %X\n", bufferSize, BYTE_COUNT(dataBitCnt + eccBitCnt + 1));
        *pWidth = dataBitCnt + eccBitCnt + 1;
        rc = TcE_Buffer_Too_Small; goto done;
    }

    // set up indexes buffer between working index and data index 
    // humming index:              0   1   2   3   4   5   6   7   8   9   10   11  12  13  14  15  16  17 ....
    // coresponding data index:   -1  -1  -1   0  -1   1   2   3  -1   4    5    6   7   8   9  10  -1  11 ....
    //
    if (indexesCnt < dataBitCnt)
    {
        for (hummingBit = 0, dataBit = 0; 
            hummingBit < ((1<<eccBitCnt) - 1); 
            hummingBit++)
        {
            if (IS_2POWER(hummingBit))
            {
                indexes[hummingBit] = (s16)-1;
            }
            else
            {
                indexes[hummingBit] = (s16)dataBit;
                dataBit++;
            }
        }
        indexesCnt = dataBit;
    }

    assertLog(indexesCnt >= dataBitCnt);

    // calculate ecc bits value and copy them both to
    //    + output data buffer
    //    + working buffer
    for (eccBit = 0; eccBit < eccBitCnt; eccBit++)
    {
        s8 eccBitValue = 0;
        //int bitsCnt = 0;
        //int valuesCnt = 0;

        //bitsCnt = sprintf(dataBitsStr + bitsCnt, "ecc[%X] : ", eccBit);
        //valuesCnt = sprintf(dataBitValuesStr + valuesCnt, "ecc[%X] : ", eccBit);

        for (hummingBit = 1; (hummingBit < (1<<eccBitCnt)); hummingBit++)
        {
            if ((IS_2POWER(hummingBit) == 0)  &&
                (((hummingBit) >> eccBit) & 1))
            {
                s8 bitValue;

                // stop when cover enough data bits
                if (indexes[hummingBit] >= dataBitCnt)
                    break;

                CHECK_ERROR(tcamLog,
                    BitBufferGetBit(buffer, bufferSize, indexes[hummingBit], &bitValue));

                eccBitValue ^= bitValue;

                //bitsCnt += sprintf(dataBitsStr + bitsCnt, "%3d ", indexes[hummingBit]);
                //valuesCnt += sprintf(dataBitValuesStr + valuesCnt, "%3d ", bitValue);
            }
        }

        //printLog(tcamLog, "%s\n",  dataBitsStr);
        //printLog(tcamLog, "%s\n", dataBitValuesStr);

        // set ecc bit value in the output buffer
        CHECK_ERROR(tcamLog,
            BitBufferSetBits(buffer, bufferSize, dataBitCnt + eccBit, 1, &eccBitValue, sizeof(eccBitValue)));
    }

    // xor bit = xor of all data bits and all ecc bits
    xorBitValue = 0;
    for (dataBit = 0; (dataBit < (dataBitCnt + eccBitCnt)); dataBit++)
    {
        s8 bitValue;
        CHECK_ERROR(tcamLog,
            BitBufferGetBit(buffer, bufferSize, dataBit, &bitValue));

        xorBitValue ^= bitValue;
    }

    // xor bit is attached after ecc bits
    CHECK_ERROR(tcamLog,
        BitBufferSetBits(buffer, bufferSize, dataBitCnt + eccBitCnt, 1, &xorBitValue, sizeof(xorBitValue)));

    *pWidth = dataBitCnt + eccBitCnt + 1;

done:
    return rc;
}

int EccWidth(int dataWidth)
{
    return EccBitCount(dataWidth) + dataWidth + 1;   // +1 for xor bit
}

//Checked back to the tests, the priority is
//In Octopuses, 
//   profile ram 
//   hash ram 
//   Module 7 highest priority, module 0 lowest
//   {
//      data ram (40>1) 
//   }
//   x-compress ram 
//   priority ram
//In SBKGU
//   kgu profile ram 
//   sb ram. 

TcError EccPriority(u8 memType, u8 module, int* priority)
{
    const int sbRamPr           = 1;
    const int kguProfileRamPr   = sbRamPr         + TcSB_COUNT;
    const int priorityRamPr     = kguProfileRamPr + TcKGU_COUNT + 1;                // 4 KGU and 1 Range
    const int xcRamPr           = priorityRamPr   + MOD_PER_OCT;
    const int dataRamPr         = xcRamPr         + MOD_PER_OCT;
    const int dataRamModuleSize = (MemTypeDram40 - MemTypeDram1 + 1);
    const int hashRamPr         = dataRamPr       + MOD_PER_OCT * dataRamModuleSize;
    const int profileRamPr      = hashRamPr       + MOD_PER_OCT;

    if ((memType == MemTypeSearchBuffer0) || (memType == MemTypeSearchBuffer1))
    {
        *priority = sbRamPr + module;
    }
    else if ((memType == MemTypeKguProfile0) || (memType == MemTypeKguProfile1))
    {
        *priority = kguProfileRamPr + module;
    }
    else if (memType == MemTypePriorityRam)
    {
        *priority = priorityRamPr + module;
    }
    else if (memType == MemTypeXcRam)
    {
        *priority = xcRamPr + module;
    }
    else if ((memType >= MemTypeDram1) && (memType <= MemTypeDram40))
    {
        // module is more significant than memType for data ram
        *priority = dataRamPr + module * dataRamModuleSize + (memType - MemTypeDram1);
    }
    else if (memType == MemTypeHashRam)
    {
        *priority = hashRamPr + module;
    }
    else if (memType == MemTypeProfile)
    {
        *priority = profileRamPr + module;
    }
    else
    {
        *priority = 0;
    }
    return TcE_OK;
}

TcError EccRegWrite(void* buffer, int bufferSize, u8 module, u16 address, u8 memType, u8 flag)
{
    TcError rc = TcE_OK;

    CHECK_ERROR(commonLibLog,
        BitBufferSetBits(buffer, bufferSize, ECC_REG_FLAG_START, ECC_REG_FLAG_BIT_CNT, &flag, sizeof(flag)));

    CHECK_ERROR(commonLibLog,
        BitBufferSetBits(buffer, bufferSize, ECC_REG_MEMTYPE_START, ECC_REG_MEMTYPE_BIT_CNT, &memType, sizeof(memType)));

    ON_BE_SWAP16(address);
    CHECK_ERROR(commonLibLog,
        BitBufferSetBits(buffer, bufferSize, ECC_REG_ADDR_START, ECC_REG_ADDR_BIT_CNT, &address, sizeof(address)));

    CHECK_ERROR(commonLibLog,
        BitBufferSetBits(buffer, bufferSize, ECC_REG_MODULE_START, ECC_REG_MODULE_BIT_CNT, &module, sizeof(module)));

done:
    return rc;
}

TcError EccRegRead(const void* buffer, int bufferSize, u8 *module, u16 *address, u8* memType, u8* flag)
{
    TcError rc = TcE_OK;

    if (flag)
    {
        CHECK_ERROR(commonLibLog,
            BitBufferGetBits(buffer, bufferSize, ECC_REG_FLAG_START, ECC_REG_FLAG_BIT_CNT, flag, sizeof(*flag)));
    }
    if (memType)
    {
        CHECK_ERROR(commonLibLog,
            BitBufferGetBits(buffer, bufferSize, ECC_REG_MEMTYPE_START, ECC_REG_MEMTYPE_BIT_CNT, memType, sizeof(*memType)));
    }
    if (address)
    {
        CHECK_ERROR(commonLibLog,
            BitBufferGetBits(buffer, bufferSize, ECC_REG_ADDR_START, ECC_REG_ADDR_BIT_CNT, address, sizeof(*address)));
        ON_BE_SWAP16(*address);
    }
    if (module)
    {
        CHECK_ERROR(commonLibLog,
            BitBufferGetBits(buffer, bufferSize, ECC_REG_MODULE_START, ECC_REG_MODULE_BIT_CNT, module, sizeof(*module)));
    }

done:
    return rc;
}

TcError QuartetEccRegRead(const void* buffer, int bufferSize, u16 *address, u8* memType, u8* flag)
{
    TcError rc = TcE_OK;

    if (flag)
    {
        CHECK_ERROR(commonLibLog,
            BitBufferGetBits(buffer, bufferSize, Q_ECC_ERROR_START, Q_ECC_ERROR_BIT_COUNT , flag, sizeof(*flag)));
    }
    if (memType)
    {
        CHECK_ERROR(commonLibLog,
            BitBufferGetBits(buffer, bufferSize, Q_ECC_ISEL_START, Q_ECC_ISEL_BIT_COUNT, memType, sizeof(*memType)));
    }
    if (address)
    {
        CHECK_ERROR(commonLibLog,
            BitBufferGetBits(buffer, bufferSize, Q_ECC_ADDRESS_START, Q_ECC_ADDRESS_BIT_COUNT, address, sizeof(*address)));
        ON_BE_SWAP16(*address);
    }
done:
    return rc;
}

TcError EccRegReset(void* buffer, int bufferSize)
{
    TcError rc = TcE_OK;

    CHECK_ERROR(commonLibLog,
        EccRegWrite(buffer, bufferSize, 0, 0, 0, 0));

done:
    return rc;
}

TcError EccRegCheckAndUpdate(void* buffer, int bufferSize, u8 newFlag, u8 newModule, u16 newAddr, u8 newMemType, u8 isModuleSignificant, int* isUpDated)
{
    TcError rc = TcE_OK;
    u8 oldModule;
    u16 oldAddr;
    u8 oldMemType;
    u8 oldFlag;
    int oldPriority;
    int newPriority;

    *isUpDated = 0;

    CHECK_ERROR(skrLog,
        EccRegRead(buffer, bufferSize, &oldModule, &oldAddr, &oldMemType, &oldFlag));

    if ((oldMemType == MemTypeSearchBuffer0) || (oldMemType == MemTypeSearchBuffer1))
    {
        oldModule = oldAddr >>TcSB_LD2_MODULE_SHIFT;
    }
    else if ((oldMemType == MemTypeKguProfile0) || (oldMemType == MemTypeKguProfile1))
    {
        oldModule = (u8)(oldAddr >> TcKGU_MODULE_SHIFT);
    }
    else if (oldMemType == MemTypeProfile)
    {
        oldModule = (u8)(oldAddr >> TcOctopus_PROFILE_MODULE_SHIFT);
    }

    CHECK_ERROR(skrLog,
        EccPriority(oldMemType, oldModule, &oldPriority));

    if (isModuleSignificant)
    {
        CHECK_ERROR(skrLog,
            EccPriority(newMemType, newModule, &newPriority));
    }
    else
    {
        if ((newMemType >= MemTypeDram1) && (newMemType <= MemTypeDram40))
        {
            // ignore dram, and module
            CHECK_ERROR(skrLog,
                EccPriority(MemTypeDram1, 0, &newPriority));
        }
        else
        {
            // ignore module only
            CHECK_ERROR(skrLog,
                EccPriority(newMemType, 0, &newPriority));
        }
    }

    if ((newFlag > oldFlag)
        ||
        ((newFlag == oldFlag) && (newPriority > oldPriority)))
    {

        if ((newMemType == MemTypeSearchBuffer0) || (newMemType == MemTypeSearchBuffer1))
        {
            newAddr += newModule <<TcSB_LD2_MODULE_SHIFT;
            newModule = 0;
        }
        else if ((newMemType == MemTypeKguProfile0) || (newMemType == MemTypeKguProfile1))
        {
            newAddr += newModule << TcKGU_MODULE_SHIFT;
            newModule = 0;
        }
        else if (newMemType == MemTypeProfile)
        {
            newAddr += newModule << TcOctopus_PROFILE_MODULE_SHIFT;
            newModule = 0;
        }

        CHECK_ERROR(skrLog,
            EccRegWrite(buffer, bufferSize, newModule, newAddr, newMemType, newFlag));

        *isUpDated = 1;
    }

done:
    return rc;
}

TcError StrAppend(char* outStr, const int outStrSize, int* pUsed, const char* newStr)
{
    TcError rc = TcE_OK;

    int len = strlen(newStr);

    assertLog(pUsed);

    if ((len + *pUsed) < outStrSize)   { strcpy(outStr + *pUsed, newStr); }
    else                               { rc = TcE_Buffer_Too_Small;  }

    *pUsed += len;

done:
    return rc;
}

static TcError BigELittleEToggle(const void* in, int inWidth, void* out, int outWidth)
{
    TcError rc = TcE_OK;
    int inBCount = BYTE_COUNT(inWidth);
    int outBCount = BYTE_COUNT(outWidth);
    int i;
    if (outWidth < inWidth)
    {
        rc = TcE_Invalid_Argument; goto done;
    }
    
    for (i=0; i<inBCount; i++)
    {
        ((char*)out)[outBCount - 1 - i] = ((char*)in)[i];
    }

    if (inBCount < outBCount)
    {
        memset((char*)out + inBCount, 0, outBCount - inBCount);
    }

done:
    return rc;
}

TcError BitBufferToBigEndianBitBuffer(const void* littleBuffer, int width, void* bigBuffer, int bigBufferWidth)
{
    TcError rc = TcE_OK;
    int offset;

    if (bigBufferWidth < width)
    {
        printError(commonLibLog, "out buffer width = %X < input buffer with = %X\n", bigBufferWidth, width);
        rc = TcE_Invalid_Argument; goto done;
    }

    offset = BYTE_COUNT(bigBufferWidth) - BYTE_COUNT(width);
    memset(bigBuffer, 0, offset);

    CHECK_ERROR(commonLibLog,
        BigELittleEToggle(littleBuffer, width, (char*)bigBuffer + offset, width));

done:
    return rc;
}

TcError BigEndianBitBufferToBitBuffer(const void* bigEBuffer, int bigEBufferWidth, void* littleEBuffer, int littleEBufferWidth)
{
    TcError rc = TcE_OK;
    int offset;

    if (littleEBufferWidth < bigEBufferWidth)
    {
        printError(commonLibLog, "out buffer width = %X < input buffer with = %X\n", littleEBufferWidth, bigEBufferWidth);
        rc = TcE_Invalid_Argument; goto done;
    }

    offset = BYTE_COUNT(littleEBufferWidth) - BYTE_COUNT(bigEBufferWidth);
    memset((char *)littleEBuffer+BYTE_COUNT(bigEBufferWidth), 0, offset);

    CHECK_ERROR(commonLibLog,
        BigELittleEToggle(bigEBuffer, bigEBufferWidth, littleEBuffer, bigEBufferWidth));

done:
    return rc;
}

TcError BigEndianBitBufferStr(const void* bigEBuffer, int width, char* outStr, int outStrSize)
{
    TcError rc = TcE_OK;
    int i;
    if (outStrSize < (HEX_COUNT(BYTE_COUNT(width))))
    {
        printError(sqLog, "output buffer too small. %X expected %X\n", outStrSize, (HEX_COUNT(BYTE_COUNT(width))));
        rc = TcE_Out_Of_Memory; goto done;
    }
    for (i=0; i<BYTE_COUNT(width); i++)
    {
        sprintf(&outStr[i*2], "%02X", ((char*)bigEBuffer)[i] & 0xff);
    }
    outStr[i*2] = 0;

done:
    return rc;
}


TcError BigEndianBitBufferStrToBinary(void* bigEBuffer, s32* count)
{
    TcError rc = TcE_OK;
    char* key = (char*)bigEBuffer;
    s32  len = strlen(key);
    s32  i;
    s32  offset = 0;
    s32 nonZeroByte;
    s32 remainingDigit;
    char tmp[MAX_BYTES];

    if (NULL == key) 
    { 
        printError(commonLibLog, "null output pointer key\n",__FUNCTION__);
        rc = TcE_Invalid_Argument; goto done; 
    }
    if (NULL == count) 
    { 
        printError(commonLibLog, "null output pointer count\n",__FUNCTION__);
        rc = TcE_Invalid_Argument; goto done; 
    }
    if (len == 0)
    {
        printError(commonLibLog, "empty string not supported\n",__FUNCTION__);
        rc = TcE_Invalid_Argument; goto done; 
    }
    if (len > (s32)sizeof(tmp))
    {
        printError(commonLibLog, "len too large. %X max=%\n",__FUNCTION__,
            len, sizeof(tmp));
        rc = TcE_Invalid_Argument; goto done; 
    }

    CHECK_ERROR(commonLibLog,
        BitBufferValidateHexaDecimalStr(key));

    memcpy(tmp, key, len + 1);

    offset = 0;

    nonZeroByte  = (len - offset) / 2;
    remainingDigit = (len - offset )% 2;
    for (i=0; i<nonZeroByte; i++)
    {
        s8 byte =   __Ch2Num(tmp[i*2]) << 4
                  | __Ch2Num(tmp[i*2 + 1]);
        key[i] = byte;
    }
    if (remainingDigit)
    {
        key[i] = __Ch2Num(tmp[i*2]);
    }

    *count = nonZeroByte + remainingDigit; 

done:
    return rc;
}

API void fillData(void* buff, int size)
{
    int i;
    int* intBuff = (int*)buff;
    
    for (i=0; i<(int)(size/sizeof(int)); i++)
    {
        intBuff[i] = rand();
    }

    if (size%sizeof(int))
    {
        char* charBuff = (char*)&intBuff[i];
        int count = size % sizeof(int);

        for (i=0; i<count; i++)
        {
            charBuff[i] = (char)rand();
        }
    }
}

TcError BitBufferXOR(void* dst, const void* src, s32 width) {
    TcError rc = TcE_OK;
    s64* dst64 = (s64*)dst;
    const s64* src64 = (const s64*)src;
    char buf[2*BYTE_COUNT(640)+1];
    s32 eSize = sizeof(dst64[0]);
    s32 count = BYTE_COUNT(width)/eSize;
    s32 remain = BYTE_COUNT(width)%eSize;
    s32 i;
    if ((NULL==dst)||(NULL==src)) {
        if (NULL==dst) printError(commonLibLog, "dst is null\n");
        if (NULL==src) printError(commonLibLog, "src is null\n");
        rc = TcE_Invalid_Argument; goto done;
    }
    //BigEndianBitBufferStr(dst, width, buf, sizeof(buf));
    //printLog(commonLibLog, "     dst=%s\n", buf);
    //BigEndianBitBufferStr(src, width, buf, sizeof(buf));
    //printLog(commonLibLog, " XOR src=%s\n", buf);
    for (i = 0; i<count; i++) { dst64[i] ^= src64[i]; }
    if (remain) {
        s8* dstRemain = (s8*)&dst64[count];
        const s8* srcRemain = (const s8*)&src64[count];
        for (i = 0; i<remain; i++) { dstRemain[i] ^= srcRemain[i]; }
    }
    //BigEndianBitBufferStr(dst, width, buf, sizeof(buf));
    //printLog(commonLibLog, "     dst=%s\n", buf);
done:
    return rc;
}

TcError BitBufferOR(void* dst, const void* src, s32 width) {
    TcError rc = TcE_OK;
    s64* dst64 = (s64*)dst;
    const s64* src64 = (const s64*)src;
    char buf[2*BYTE_COUNT(640)+1];
    s32 eSize = sizeof(dst64[0]);
    s32 count = BYTE_COUNT(width)/eSize;
    s32 remain = BYTE_COUNT(width)%eSize;
    s32 i;
    if ((NULL==dst)||(NULL==src)) {
        if (NULL==dst) printError(commonLibLog, "dst is null\n");
        if (NULL==src) printError(commonLibLog, "src is null\n");
        rc = TcE_Invalid_Argument; goto done;
    }
    BigEndianBitBufferStr(dst, width, buf, sizeof(buf));
    printLog(commonLibLog, "     dst=%s\n", buf);
    BigEndianBitBufferStr(src, width, buf, sizeof(buf));
    printLog(commonLibLog, " OR src=%s\n", buf);
    for (i = 0; i<count; i++) { dst64[i] |= src64[i]; }
    if (remain) {
        s8* dstRemain = (s8*)&dst64[count];
        const s8* srcRemain = (const s8*)&src64[count];
        for (i = 0; i<remain; i++) { dstRemain[i] |= srcRemain[i]; }
    }
    BigEndianBitBufferStr(dst, width, buf, sizeof(buf));
    printLog(commonLibLog, "     dst=%s\n", buf);
done:
    return rc;
}

int BitBufferCmp(const void *in1, const void *in2, s32 start, s32 bitCnt)
{
    s8 *buf1 = (s8 *)in1;
    s8 *buf2 = (s8 *)in2;
    s32 eSize = sizeof(buf1[0])*BITS_PER_BYTE;
    s32 startByte = start / eSize, endByte = (start + bitCnt) / eSize;
    s32 pre = start % eSize, post = (start + bitCnt) % eSize;
    s32 i = startByte;
    s8 mask;

    if (startByte == endByte) {
        mask = 0xff & (~((1<<pre)-1));
        mask &= (1<<post)-1;
        return (buf1[i] ^ buf2[i]) & mask;
    }
    if (pre) {
        mask = 0xff & (~((1<<pre)-1));
        if ((buf1[i] ^ buf2[i]) & mask) return 1;
        i++;
    }
    if ((endByte>i) && memcmp(&buf1[i], &buf2[i], endByte-i)) return 1;
    if (post) {
        mask = 0xff & ((1<<post)-1);
        if ((buf1[endByte] ^ buf2[endByte]) & mask) return 1;
    }
    return 0;
}

TcError ByteBitsReversal(u8* buff) {
    TcError rc = TcE_OK;
    u8 c1 = (*buff) & 0x0f;
    u8 c2 = (*buff) & 0xf0;
    switch (c1) {
        case 0x00: c1 = 0x00; break;
        case 0x01: c1 = 0x80; break;
        case 0x02: c1 = 0x40; break;
        case 0x03: c1 = 0xc0; break;
        case 0x04: c1 = 0x20; break;
        case 0x05: c1 = 0xa0; break;
        case 0x06: c1 = 0x60; break;
        case 0x07: c1 = 0xe0; break;
        case 0x08: c1 = 0x10; break;
        case 0x09: c1 = 0x90; break;
        case 0x0a: c1 = 0x50; break;
        case 0x0b: c1 = 0xd0; break;
        case 0x0c: c1 = 0x30; break;
        case 0x0d: c1 = 0xb0; break;
        case 0x0e: c1 = 0x70; break;
        case 0x0f: c1 = 0xf0; break;
        default:
        printError(commonLibLog, "bits reversal error!\n");
        rc = TcE_Failed; goto done;
        break;
    }
    switch (c2) {
        case 0x00: c2 = 0x00; break;
        case 0x10: c2 = 0x08; break;
        case 0x20: c2 = 0x04; break;
        case 0x30: c2 = 0x0c; break;
        case 0x40: c2 = 0x02; break;
        case 0x50: c2 = 0x0a; break;
        case 0x60: c2 = 0x06; break;
        case 0x70: c2 = 0x0e; break;
        case 0x80: c2 = 0x01; break;
        case 0x90: c2 = 0x09; break;
        case 0xa0: c2 = 0x05; break;
        case 0xb0: c2 = 0x0d; break;
        case 0xc0: c2 = 0x03; break;
        case 0xd0: c2 = 0x0b; break;
        case 0xe0: c2 = 0x07; break;
        case 0xf0: c2 = 0x0f; break;
        default:
        printError(commonLibLog, "bits reversal error!\n");
        rc = TcE_Failed; goto done;
        break;
    }
    *buff = c1 | c2;
done:
    return rc;
}

TcError BEBitBufferSetBit(void* buff, s32 buff_width, s32 bit) {
    TcError rc = TcE_OK;
    u8 *buff8 = (u8*)buff;
    if (bit >= buff_width) {
        printError(commonLibLog, "bit is too big: max=%d, bit=%d\n", buff_width - 1, bit); rc = TcE_Invalid_Argument; goto done;
    }
    buff8[U8_COUNT_FLOOR(bit)] |= 1 << (U8_BIT_COUNT - 1 - U8_MOD(bit));
done:
    return rc;
}

static void __BEBitBufferSetBits8(void* buff, s32 buff_width, s32 bitStart, s32 bitCount, const void* in) {
    u8 *data, *data_in, left_mask, right_mask, data_mask, in_mask, data_in_right_bits;
    int i, left_bits, right_bits, bitCount_left, bitCount_right, bitStart_u8_count, data_width;
    bitStart_u8_count = U8_COUNT_FLOOR(bitStart);
    data = (u8*)buff + bitStart_u8_count;
    data_width = buff_width - bitStart_u8_count<<U8_SHIFT;
    data_in = (u8*)in;
    bitStart = U8_MOD(bitStart);
    if (!bitStart) { // bitStart is byte-aligned
        for (i = 0; i<U8_COUNT_FLOOR(bitCount); i++) { data[i] = data_in[i]; }
        if (U8_MOD(bitCount)) {
            right_mask = (1<<(U8_BIT_COUNT - U8_MOD(bitCount))) - 1;
            data[i] = (data_in[i] & (~right_mask)) | (data[i] & right_mask);
        }
        return;
    }
    //
    //      <----data(i)------><----data(i+1)---->
    //data  [ 0 1 2 3 4 5 6 7 ][ 0 1 2 3 4 5 6 7 ][ 0 1 2 3 4 5 6 7 ]....
    //        <--->               left_bits
    //              <------->     right_bits
    //        0 0 0 1 1 1 1 1     right_mask
    //        1 1 1 0 0 0 0 0     left_mask
    //in            <----in(i)-------> <------in(i+1)--->.....
    //
    left_bits  = bitStart;
    right_bits = U8_BIT_COUNT - left_bits;
    right_mask = (1<<right_bits) - 1;
    left_mask = ~right_mask;
    for (i = 0; i < U8_COUNT_FLOOR(bitCount); i++) {
        data[i]   = (data[i] & left_mask)        | (data_in[i] >> left_bits);
        data[i+1] = (data_in[i] << right_bits)   | (data[i+1] & right_mask);
    }
    if (U8_MOD(bitCount)) {
        bitCount = U8_MOD(bitCount);
        if (bitCount <= right_bits) {
            // buff [ 0 1 2 3 4 5 6 7 ][ 0 1 2 3 4 5 6 7 ]
            //        <---> left
            //              <-------> right_bits
            //              <---> bitCount
            //              1 1 1 0 0    0 0 0   :  in_mask
            //              <---in----------->
            //        0 0 0 0 0 0 1 1            : data_mask
            //        1 1 1 0 0 0 0 0            : left_mask
            in_mask = ((1 << bitCount) - 1) << (U8_BIT_COUNT - bitCount); // bitCount of MSB
            data_mask = (1 << (U8_BIT_COUNT - left_bits - bitCount)) - 1;
            data[i] = (data[i] & left_mask) | ((data_in[i] & in_mask) >> left_bits) | (data[i] & data_mask);
            return;
        }
        // buff [ 0 1 2 3 4 5 6 7 ][ 0 1 2 3 4 5 6 7 ]
        //        <---> left
        //              <-------> right_bits
        //              <------------> bitCount
        //              0 0 0 0 0    1 0 0           :  in_mask
        //              <---in----------->
        //                           0 1 1 1 1 1 1 1 : data_mask
        data[i] = (data[i] & left_mask) | (data_in[i] >> left_bits);
        in_mask = (((1<<(bitCount-right_bits)) - 1) << (U8_BIT_COUNT - bitCount));
        data_mask = (1<<(U8_BIT_COUNT - (bitCount-right_bits))) - 1;
        data[i+1] = ((data_in[i] & in_mask)<<right_bits) | (data[i+1] & data_mask);
    }
}

static void __BEBitBufferSetBits64(void* buff, s32 buff_width, s32 bitStart, s32 bitCount, const void* in) {
    u64 *data, *data_in;
    int data_width, bitStart_u64_count, i;
    if (!U64_ALIGN_PTR(buff) || !U64_ALIGN_PTR(in)) {
        __BEBitBufferSetBits8(buff, buff_width, bitStart, bitCount, in);
    }
    data_in     = (u64*)in ;
    bitStart_u64_count = U64_COUNT_FLOOR(bitStart);
    data        = (u64*)buff + bitStart_u64_count;
    data_width  = buff_width - bitStart_u64_count<<U64_SHIFT;
    bitStart    = U64_MOD(bitStart);
    if (bitStart) {
        __BEBitBufferSetBits8(data, data_width, bitStart, bitCount, data_in);
        return;
    }
    for (i = 0; i < U64_COUNT_FLOOR(bitCount); i++) {
        data[i] = data_in[i];
    }
    if (U64_MOD(bitCount)) {
        data = &data[i];
        data_in = &data_in[i];
        data_width -= i<<U64_SHIFT;
        __BEBitBufferSetBits8(data, data_width, 0, U64_MOD(bitCount), data_in);
    }
}

TcError BEBitBufferSetBits(void* buff, s32 buff_width, s32 bitStart, s32 bitCount, const void* in) {
    TcError rc = TcE_OK;
    if (bitStart+bitCount > buff_width) {
        printError(commonLibLog, "bit over run: startBit(%d)+bitCount(%d) > buff_width(%d)\n", bitStart, bitCount, buff_width); rc = TcE_Invalid_Argument; goto done;
    }
    __BEBitBufferSetBits64(buff, buff_width, bitStart, bitCount, in);
done:
    return rc;
}

TcError BEBitBufferClearBit(void* buff, s32 buff_width, s32 bit) {
    TcError rc = TcE_OK;
    u8 *data = (u8*)buff, data_width = U8_BIT_COUNT;
    if (bit >= buff_width) {
        printError(commonLibLog, "bit is too big: max=%d, bit=%d\n", buff_width - 1, bit); rc = TcE_Invalid_Argument; goto done;
    }
    data[U8_COUNT_FLOOR(bit)] &= ~(1 << ((U8_BIT_COUNT - 1) - U8_MOD(bit)));
done:
    return rc;
}

static void __BEBitBufferClearBits8(void *buff, s32 buff_width, s32 bitStart, s32 bitCount) {
    u8 *data = NULL, mask;
    int i, bitStart_u8_count, data_width;
    bitStart_u8_count = U8_COUNT_FLOOR(bitStart); 
    data       = (u8*)buff  + bitStart_u8_count;
    data_width = buff_width - bitStart_u8_count<<U8_SHIFT;
    bitStart   = U8_MOD(bitStart);
    if (bitStart) {
        //int data_aligned_bitStart = (bitStart + U8_BIT_COUNT - 1) & (~(U8_BIT_COUNT-1));
        if ((bitStart + bitCount) <= U8_BIT_COUNT) {
            //   0    1    2    3    4    5    6    7 
            //   <-bitStart>    <-bitCount>
            //   
            mask =  ((1<<bitCount) - 1)<<(U8_BIT_COUNT - bitStart - bitCount);
            data[0] &= ~mask;
            return;
        }
        mask = (1<<(U8_BIT_COUNT - bitStart)) - 1;
        data[0] &= ~mask;
        data++;
        bitCount -= U8_BIT_COUNT - bitStart;
    }
    for (i = 0; i < (U8_COUNT_FLOOR(bitCount)); i++) {
        data[i] = 0;
    }
    if (U8_MOD(bitCount)) {
        bitCount = U8_MOD(bitCount);
        mask = ((1<<bitCount) - 1)<<(U8_BIT_COUNT - bitCount );
        data[i] &= ~mask;
    }
}

static void __BEBitBufferClearBits64(void* buff, s32 buff_width, s32 bitStart, s32 bitCount) {
    u64 *data = NULL;
    int data_width, bitStart_u64_count, i;
    if (!U64_ALIGN_PTR(buff)) {
        __BEBitBufferClearBits8(buff, buff_width, bitStart, bitCount);
    }
    bitStart_u64_count = U64_COUNT_FLOOR(bitStart);
    data       = (u64*)buff + bitStart_u64_count;
    data_width = buff_width - (bitStart_u64_count<<U64_SHIFT);
    bitStart = U64_MOD(bitStart);
    if (bitStart) {
        if ((bitStart + bitCount) <= U64_BIT_COUNT ) {
            __BEBitBufferClearBits8(data, data_width, bitStart, bitCount);
            return;
        }
        __BEBitBufferClearBits8(data, data_width, bitStart, U64_BIT_COUNT - bitStart);
        bitCount -= U64_BIT_COUNT - bitStart;
        data++;
    }
    for (i = 0; i < U64_COUNT_FLOOR(bitCount); i++) { data[i] = 0; }
    if (U64_MOD(bitCount)) {
        data        = &data[i];
        data_width -= (i<<U64_SHIFT);
        __BEBitBufferClearBits8(data, data_width, 0, U64_MOD(bitCount));
    }
}

// bitStart: 0...64, bitCount: 1...128
TcError BEBitBufferClearBits(void* buff, s32 buff_width, s32 bitStart, s32 bitCount) {
    TcError rc = TcE_OK;
    if (bitStart + bitCount > buff_width) {
        printError(commonLibLog, "bit over run: startBit(%d)+bitCount(%d) > buff_width(%d)\n", bitStart, bitCount, buff_width); rc = TcE_Invalid_Argument; goto done;
    }
    __BEBitBufferClearBits64(buff, buff_width, bitStart, bitCount);
done:
    return rc;
}

TcError BEBitBufferGetBit(const void* buff, s32 buff_width, s32 bit, s8* bitValue) {
    TcError rc = TcE_OK;
    u8 *buff8 = (u8*)buff;
    if (bit >= buff_width) {
        printError(commonLibLog, "bit is too big: max=%d, bit=%d\n", buff_width - 1, bit); rc = TcE_Invalid_Argument; goto done;
    }
    if (buff8[U8_COUNT_FLOOR(bit)] & (1 << (U8_BIT_COUNT - 1 - U8_MOD(bit)))) {
        *bitValue = 1;
    } else {
        *bitValue = 0;
    }
done:
    return rc;
}

static void __BEBitBufferGetBits8(const void* buff, s32 buff_width, s32 bitStart, s32 bitCount, void* out) {
    u8 *data = (u8*)buff, *data_out=(u8*)out, left_mask, right_mask, data_mask, out_mask;
    int i, left_bits, right_bits, bitCount_left, bitCount_right, bitStart_u8_count, data_width;
    bitStart_u8_count = U8_COUNT_FLOOR(bitStart);
    data = (u8*)buff + bitStart_u8_count;
    data_width = buff_width - (bitStart_u8_count<<U8_SHIFT);
    data_out = (u8*)out;
    bitStart = U8_MOD(bitStart);
    if (!bitStart) { // bitStart is byte-aligned
        for (i = 0; i<(U8_COUNT_FLOOR(bitCount)); i++) {
            data_out[i] = data[i];
        }
        if (U8_MOD(bitCount)) {
            bitCount = U8_MOD(bitCount);
            right_mask = (1<<(U8_BIT_COUNT - bitCount)) - 1;
            data_out[i] = (data[i]&(~right_mask)) | (data_out[i] & right_mask);
        }
        return;
    }
    //buff  [01234567][01234567]....
    //       <->          left_bits
    //          <--->     right_bits
    //       00011111     right_mask
    //       11100000     left_mask
    //out       <--out0--><--out1-->....
    left_bits  = bitStart;
    right_bits = U8_BIT_COUNT - left_bits;
    right_mask = (1<<right_bits) - 1;
    left_mask  = ~right_mask;
    for (i = 0; i < (U8_COUNT_FLOOR(bitCount)); i++) {
        data_out[i] = (data[i]<<left_bits) | (data[i+1] >> right_bits);
    }
    if (U8_MOD(bitCount)) {
        bitCount = U8_MOD(bitCount);
        if (bitCount <= right_bits) {
            // buff [ 0 1 2 3 4 5 6 7 ][ 0 1 2 3 4 5 6 7 ]
            //        <---> left
            //              <-------> right_bits
            //              <---> bitCount
            //        0 0 0 1 1 1 0 0  data_mask
            data_mask = ((1<<bitCount) - 1) << (right_bits-bitCount);
            out_mask  = (1<<(U8_BIT_COUNT - bitCount)) - 1;
            data_out[i] = ((data[i] & data_mask) << left_bits) | (data_out[i] & out_mask);
            return;
        }
        //      [       i         ][    i+1          ]
        // buff [ 0 1 2 3 4 5 6 7 ][ 0 1 2 3 4 5 6 7 ]
        //        <---> left
        //              <--------> right_bits
        //              <--------------> bitCount
        //                           1 1 0 0 0 0 0 0 : data_mask
        //              0 0 0 0 0    0 0 1           : out_mask
        // out         [      i           ]
        //               3 4 5 6 7 0 1 2
        bitCount -= right_bits;
        data_mask = (((1<<bitCount) - 1)<<(U8_BIT_COUNT-bitCount));
        bitCount += right_bits;  // original bitCount
        out_mask  = (1<<(U8_BIT_COUNT-bitCount)) - 1;
        data_out[i] = ((data[i] & right_mask)<<left_bits) | ((data[i+1] & data_mask) >>right_bits) | (data_out[i] & out_mask);
    }
}

static void __BEBitBufferGetBits64(const void* buff, s32 buff_width, s32 bitStart, s32 bitCount, void* out) {
    u64 *data, *data_out;
    int bitStart_u64_count, data_width, i;
    if (!U64_ALIGN_PTR(buff) || !U64_ALIGN_PTR(out)) {
        __BEBitBufferGetBits8(buff, buff_width, bitStart, bitCount, out);
        return;
    }
    bitStart_u64_count = U64_COUNT_FLOOR(bitStart);
    data = (u64*)buff + bitStart_u64_count;
    data_width = buff_width - (bitStart_u64_count<<U64_SHIFT);
    data_out = (u64*)out;
    bitStart = U64_MOD(bitStart);
    if (bitStart) { 
        __BEBitBufferGetBits8(data, data_width, bitStart, bitCount, data_out);
        return;
    }
    for (i=0; i<U64_COUNT_FLOOR(bitCount); i++) { data_out[i] = data[i]; }
    if (U64_MOD(bitCount)) {
        data        = &data[i];
        data_out    = &data_out[i];
        data_width -= (i<<U64_SHIFT);
        __BEBitBufferGetBits8(data, data_width, 0, U64_MOD(bitCount), data_out);
    }
}

TcError BEBitBufferGetBits(const void* buff, s32 buff_width, s32 bitStart, s32 bitCount, void* out) {
    TcError rc = TcE_OK;
    if (bitStart+bitCount > buff_width) {
        printError(commonLibLog, "bit over run: startBit(%d)+bitCount(%d) > buff_width(%d)\n", bitStart, bitCount, buff_width); rc = TcE_Invalid_Argument; goto done;
    }
    __BEBitBufferGetBits64(buff, buff_width, bitStart, bitCount, out);
done:
    return rc;
}

static TcError BEBitBufferXOR8(void *in_dst, void *in_src, int width) {
    u8 *dst = (u8*)in_dst, *src = (u8*)in_src;
    int i;
    u8 mask;
    for (i=0; i<U8_COUNT_FLOOR(width); i++) {
        dst[i] ^= src[i];
    }
    if (U8_MOD(width)) {
        mask = (1<<(U8_BIT_COUNT - U8_MOD(width))) - 1;
        dst[i] = (dst[i] ^ src[i]) & (~(mask)) | (dst[i] & mask);
    }
    return 0;
}

TcError BEBitBufferXOR(void* dst, const void* src, s32 width) {
    TcError rc = TcE_OK;
    u64* dst64 = (u64*)dst;
    const u64* src64 = (const u64*)src;
    s32 i;
    if ((NULL==dst)||(NULL==src)) {
        if (NULL==dst) printError(commonLibLog, "dst is null\n");
        if (NULL==src) printError(commonLibLog, "src is null\n");
        rc = TcE_Invalid_Argument; goto done;
    }
    if (!U64_ALIGN_PTR(dst) || !U64_ALIGN_PTR(src)) {
        return BEBitBufferXOR8(dst, src, width);
    }
    for (i = 0; i<U64_COUNT_FLOOR(width); i++) { dst64[i] ^= src64[i]; }
    if (U64_MOD(width)) {
        return BEBitBufferXOR8(&dst64[i], &src64[i], U64_MOD(width));
    }
done:
    return rc;
}

static inline int __BEBitBufferCountLeadingZero1(void *in_byte, int bitCnt) {
    u8 *byte = (u8*)in_byte;
    int i;
    for (i = 0; i < bitCnt; i++) {
        if (byte[0] & (1 << (U8_BIT_COUNT - 1 - i))) { return i; }
    }
    return i;
}

static int __BEBitBufferCountLeadingZero8(void *buff, int width) {
    u8 *buff8 = (u8*)buff;
    int count=0, i;
    for (i = 0; i < U8_COUNT_FLOOR(width); i++) {
        if (buff8[i]) {
            count += __BEBitBufferCountLeadingZero1(&buff8[i], U8_BIT_COUNT);
            return count;
        }
        count+=U8_BIT_COUNT;
    }
    if (U8_MOD(width)) {
        count += __BEBitBufferCountLeadingZero1(&buff8[i], U8_MOD(width));
    }
    return count;
}

TcError BEBitBufferCountLeadingZero(const void* in, s32 width, int *pCount) {
    TcError rc = TcE_OK;
    u64* buff64 = (u64*)in;
    int i, count;
    assertLog(in!=NULL);
    if (!U64_ALIGN_PTR(in)) {  //in not point to 64-bit aligned address
        *pCount = __BEBitBufferCountLeadingZero8(in, width);
		return TcE_OK;
    }
    count=0;
    for (i = 0; i<U64_COUNT_FLOOR(width); i++) { 
        if (buff64[i]) {
            *pCount = count + __BEBitBufferCountLeadingZero8(&buff64[i], U64_BIT_COUNT);
            return TcE_OK;
        }
        count+=U64_BIT_COUNT;
    }
    if (U64_MOD(width)) {
        count += __BEBitBufferCountLeadingZero8((u8*)&buff64[i], U64_MOD(width));
    }
    *pCount = count;
done:
    return rc;
}

// start and bitCnt must be confined within a byte
static int BEBitBufferCmp1(void *in1, void *in2, s32 start, s32 bitCnt, s32 *pPrefixLen) {
    u8 mask, i, byte1 = *((u8*)in1), byte2 = *((u8*)in2);
    // 0 1 2 3 4 5 6 7
    // ------>start
    //       <----->bitCount
    // 0 0 0 1 1 1 1 1: 
    // 0 0 0 0 0 0 0 1:
    // 0 0 0 1 1 1 1 0: mask
    // 
    mask  = (1<<(U8_BIT_COUNT - start)) - 1;
    mask ^= (1<<(U8_BIT_COUNT - (start + bitCnt))) - 1;
    if ((byte1&mask) != (byte2&mask)) {
        for (i=start; i<(start+bitCnt); i++) {
            mask = 1<<(U8_BIT_COUNT - 1 - i);
            if ((byte1 & mask) != (byte2 & mask)) {
                if (pPrefixLen) { *pPrefixLen = i - start; }
                return 1; 
            }
        } 
    }
    if (pPrefixLen) { *pPrefixLen = bitCnt; }
    return 0;
}

static int BEBitBufferCmp8(void *void_in1, void *void_in2, s32 start, s32 bitCnt, s32 *pPrefixLen) {
    int i, j, cnt=0, prefix_len=0, start_u8_count;
    u8 mask, *in1, *in2;
    start_u8_count = U8_COUNT_FLOOR(start);
    in1 = (u8*)void_in1 + start_u8_count;
    in2 = (u8*)void_in2 + start_u8_count;
    start = U8_MOD(start);
    if ((start+bitCnt) <= U8_BIT_COUNT) {   // start and end: same byte
        return BEBitBufferCmp1(in1, in2, start, bitCnt, pPrefixLen);
    }
    if (start) {
        if (BEBitBufferCmp1(in1, in2, start, U8_BIT_COUNT - start, &prefix_len)) {
           if (pPrefixLen) { *pPrefixLen = prefix_len; }
           return 1; 
        }
        cnt    = U8_BIT_COUNT - start;
        bitCnt -= U8_BIT_COUNT - start;
        in1++;
        in2++;
    }
    for (i=0; i<U8_COUNT_FLOOR(bitCnt); i++) {
        if (in1[i] != in2[i]) {
            BEBitBufferCmp1(&in1[i], &in2[i], 0, U8_BIT_COUNT, &prefix_len);
            if (pPrefixLen) { *pPrefixLen = cnt + (i<<U8_SHIFT) + prefix_len; }
            return 1;
        }
    }
    if (U8_MOD(bitCnt)) {
        if (BEBitBufferCmp1(&in1[i], &in2[i], 0, U8_MOD(bitCnt), &prefix_len)) {
            if (pPrefixLen) { *pPrefixLen = cnt + (i<<U8_SHIFT) + prefix_len; }
            return 1;
        }
    }
    if (pPrefixLen) { *pPrefixLen = cnt + (i<<U8_SHIFT) + prefix_len; }
    return 0;
}

int BEBitBufferCmp(const void *in1, const void *in2, s32 start, s32 bitCnt, s32 *pPrefixLen) {
    u64 *buff1 = NULL, *buff2 = NULL;
    int cnt=0, start_align_up, i, start_u64_count, prefix_len = 0;
    if (bitCnt==0) {
        if (pPrefixLen) { *pPrefixLen = 0; }
        return 0;
    }
    if (!U64_ALIGN_PTR(in1) || !U64_ALIGN_PTR(in2)) { // either in1 or in2 not point to 64-bit address
        return BEBitBufferCmp8(in1, in2, start, bitCnt, pPrefixLen);
    }
    start_u64_count = U64_COUNT_FLOOR(start);
    buff1 = (u64*)in1 + start_u64_count;
    buff2 = (u64*)in2 + start_u64_count;
    start = U64_MOD(start);
    if ((start+bitCnt) <= U64_BIT_COUNT) {
        return BEBitBufferCmp8(buff1, buff2, start, bitCnt, pPrefixLen);
    }
    if (start) { 
        if (BEBitBufferCmp8(buff1, buff2, start, U64_BIT_COUNT - start, &prefix_len)) {
            if (pPrefixLen) { *pPrefixLen = prefix_len; }
            return 1;
        }
        bitCnt -= U8_BIT_COUNT - start;
        cnt    = U8_BIT_COUNT - start;
        buff1++;
        buff2++;
    }
    for (i=0; i<U64_COUNT_FLOOR(bitCnt); i++) {
        if (buff1[i] != buff2[i]) {
            BEBitBufferCmp8(&buff1[i], &buff2[i], 0, U64_BIT_COUNT, &prefix_len);
            if (pPrefixLen) { *pPrefixLen = cnt + (i<<U64_SHIFT) + prefix_len; }
            return 1;
        }
    }
    if (U64_MOD(bitCnt)) {
        if (BEBitBufferCmp8(&buff1[i], &buff2[i], 0, U64_MOD(bitCnt), &prefix_len)) {
           if (pPrefixLen) { *pPrefixLen = cnt + (i<<U64_SHIFT) + prefix_len; }
            return 1;
        }
    }
    if (pPrefixLen) { *pPrefixLen = cnt + (i<<U64_SHIFT) + prefix_len; }
    return 0;
}

TcError BEBitBufferShiftLeft(const void *buff, s32 width, int bitCnt) {
    TcError rc = TcE_OK;
    u8 bit;
    int i;
    assertLog(bitCnt <= width);
    if (bitCnt == width) {
        BEBitBufferClearBits(buff, width, 0, width);
        return TcE_OK;
    }
    for (i = 0; i <(width-bitCnt); i++) {
        CHECK_ERROR(commonLibLog, BEBitBufferGetBit(buff, width, i + bitCnt, &bit));
        CHECK_ERROR(commonLibLog, BEBitBufferSetBits(buff, width, i, 1, &bit));
    }
    CHECK_ERROR(commonLibLog, BEBitBufferClearBits(buff, width, width-bitCnt, bitCnt));
done:
    return rc;
}
