#include <string.h>
#include <stdlib.h>
#include "IoCmd.h"
#include "log.h"

#define CHECK_FOR_EOF(io)   { if (feof(io->fp))  { rc = TcW_Eof; goto done; } }
#define IO_MAGIC_NUMBER    (0xABCDDCBA)
#define IO_VERSION_NUMBER  (0)
#define IO_TYPE_INPUT      (1)
#define IO_TYPE_OUPUT      (2)

#define FWRITE_FIELD(io, a)                                                  \
      {                                                                      \
          int count = sizeof(a);                                             \
          int i;                                                             \
          for (i=0; i<count; i++)                                            \
          {                                                                  \
               unsigned char byte = (unsigned char)(a>>(i*BITS_PER_BYTE));   \
               if (fwrite(&byte, 1, 1, io->fp) != 1)                         \
               {                                                             \
                   printError(commonLibLog, "failed to write %s\n", #a);     \
                   rc = TcE_Failed; goto done;                               \
               }                                                             \
          }                                                                  \
      }

#define FWRITE_BUFFER(io, buff, buffSize)                           \
      if (fwrite(buff, buffSize, 1, io->fp) != 1)                   \
      {                                                             \
          printError(commonLibLog, "failed to write %s\n", #buff);  \
          rc = TcE_Failed; goto done;                               \
      }                                                             \

#define FREAD_FIELD(io, a)                                                   \
      {                                                                      \
          int count = sizeof(*(a));                                          \
          int i;                                                             \
          *(a) = 0;                                                          \
          for (i=0; i<count; i++)                                            \
          {                                                                  \
               unsigned char byte;                                           \
               if (fread(&byte, 1, 1, io->fp) != 1)                          \
               {                                                             \
                   CHECK_FOR_EOF(io);                                        \
                   printError(commonLibLog, "failed to read byte: %d of %d \n", i, count); \
                   rc = TcE_Failed; goto done;                               \
               }                                                             \
               *(a) |= byte << (i*BITS_PER_BYTE);                            \
          }                                                                  \
      }


#define FREAD_BUFFER(io, buff, buffSize)                            \
      if (fread(buff, buffSize, 1, io->fp) != 1)                    \
      {                                                             \
          printError(commonLibLog, "failed to read %s\n", #buff);   \
          rc = TcE_Failed; goto done;                               \
      }                                                             \

#define SEQUENCE_NUMBER_INVALID                                 ((u32)-1)

struct sIo
{
    char fname[512];
    u32 seqNumber;
    FILE* fp;
    int  type;
    u32 readMagic;
    u32 readVersion;
    u8 readOpcode;
    u32 readExpectSeqNumber;
};

TcError Io_Output_Open(const char* fn, Io** pIo)
{
   TcError rc = TcE_OK;
//   int cnt;
   Io* io = (Io*)malloc(sizeof(*io));
   u32 magic = IO_MAGIC_NUMBER;
   u32 version = IO_VERSION_NUMBER;

   if (!io)
   {
       printError(commonLibLog, "out-of-memory\n");
       rc = TcE_Out_Of_Memory; goto done;
   }
   if (strlen(fn) >= sizeof(io->fname))
   {
       printError(commonLibLog, "file name too long. %s (%X) max = %X\n", 
           fn, strlen(fn), sizeof(io->fname)-1);
       return TcE_Out_Of_Range;
   }

   memset(io, 0, sizeof(*io));
   io->type = IO_TYPE_OUPUT;
   io->fp  = fopen(fn, "wb");
   if (NULL == io->fp)
   {
        printError(commonLibLog, "cannot open file %s for writing. \n", fn);
        return TcE_Failed;
   }

   FWRITE_FIELD(io, magic);

//   if ((cnt=fwrite(&magic, sizeof(magic), 1, io->fp)) != 1)
//   {
//       printError(commonLibLog, "cannot write magic number to file %s\n", fn);
//       return TcE_Failed;
//   }

   FWRITE_FIELD(io, version);

//   if ((cnt=fwrite(&version, sizeof(version), 1, io->fp)) != 1)
//   {
//       printError(commonLibLog, "cannot write version number to file %s\n", fn);
//       return TcE_Invalid_Argument;
//   }

   io->seqNumber = 0 | 0xffff0000;
   *pIo = io;

done:
   if ((rc) && (io))
   {
       Io_Close(io);
   }
   return rc;
}

void Io_Close(Io* io)
{
    if (io)
    {
        if (io->fp) fclose(io->fp);
        free(io);
    }
}

static TcError Io_Write_SeqNumber(Io* io)
{
    TcError rc = TcE_OK;

    FWRITE_FIELD(io, io->seqNumber);

//    if (fwrite(&io->seqNumber, sizeof(io->seqNumber), 1, io->fp) != 1)
//    {
//        printError(commonLibLog, "failed to write seq number\n");
//        return TcE_Failed; 
//    }

    io->seqNumber = ++io->seqNumber | 0xffff0000;

done:
    return rc;
}

TcError Io_Write_LD2(Io* io, u8 dev, const void* data, int dataSize)
{
    TcError rc = TcE_OK;
    u8 opCode = IO_OPCODE_LD2;  

    assertLog(io->type == IO_TYPE_OUPUT);

    CHECK_ERROR(commonLibLog, Io_Write_SeqNumber(io));
    FWRITE_FIELD(io, opCode);
    FWRITE_FIELD(io, dev);

    if (dataSize < IO_DATA_SIZE_BYTE_COUNT)
    {
        char buff[IO_DATA_SIZE_BYTE_COUNT];
        memset(buff, 0, sizeof(buff));   // unnecessary but useful when debugging
        memcpy(buff, data, dataSize);

        FWRITE_BUFFER(io, buff, IO_DATA_SIZE_BYTE_COUNT);
    }
    else
    {
        FWRITE_BUFFER(io, data, IO_DATA_SIZE_BYTE_COUNT);
    }

done:
    return rc;
}

TcError Io_Write_LD3(Io* io, u8 dev, u32 address)
{
    TcError rc = TcE_OK;
    u8 opCode = IO_OPCODE_LD3;  

    CHECK_ERROR(commonLibLog, Io_Write_SeqNumber(io));
    FWRITE_FIELD(io, opCode);
    FWRITE_FIELD(io, dev);
    FWRITE_FIELD(io, address);

done:
    return rc;
}

TcError Io_Write_LD5_LC(Io* io, u8 opCode, u8 profile, u16 sbAddr, u8 width, const void* key, int keySize)
{
    TcError rc = TcE_OK;
    int writeKeySize;

    writeKeySize = (width+1) * IO_KEY_MIN_SIZE_BYTE_COUNT;
    if (keySize < writeKeySize)
    {
        printError(commonLibLog, "key size too small. %X < %X\n",
                keySize, writeKeySize);
        rc = TcE_Invalid_Argument; goto done;
    }

    CHECK_ERROR(commonLibLog, Io_Write_SeqNumber(io));
    FWRITE_FIELD(io, opCode);
    FWRITE_FIELD(io, profile);
    FWRITE_FIELD(io, sbAddr);
    FWRITE_FIELD(io, width);
    FWRITE_BUFFER(io, key, writeKeySize);

done:
    return rc;
}

TcError Io_Write_LC(Io* io, u8 profile, u16 sbAddr, u8 width, const void* key, int keySize)
{
    return Io_Write_LD5_LC(io, IO_OPCODE_LC, profile, sbAddr, width, key, keySize);
}

TcError Io_Write_LD5(Io* io, u8 profile, u16 sbAddr, u8 width, const void* key, int keySize)
{
    return Io_Write_LD5_LC(io, IO_OPCODE_LD5, profile, sbAddr, width, key, keySize);
}

TcError Io_Write_DBG_LC_Input(Io* io, u8 profile, u16 sbAddr, u8 width, const void* key, int keySize, 
                                u8 ready1, u8 match1, const void* prior1, int prior1Size, 
                                u8 ready2, u8 match2, const void* prior2, int prior2Size, 
                                u8 isOnline )
{
    TcError rc = TcE_OK;
    int writeKeySize;
    u8 opCode = IO_DBG_LC_INPUT;

    writeKeySize = (width+1) * IO_KEY_MIN_SIZE_BYTE_COUNT;
    if (keySize < writeKeySize)
    {
        printError(commonLibLog, "key size too small. %X < %X\n",
                keySize, writeKeySize);
        rc = TcE_Invalid_Argument; goto done;
    }

    CHECK_ERROR(commonLibLog, Io_Write_SeqNumber(io));

    FWRITE_FIELD(io, opCode);
    FWRITE_FIELD(io, profile);
    FWRITE_FIELD(io, sbAddr);
    FWRITE_FIELD(io, width);
    FWRITE_BUFFER(io, key, writeKeySize);
    FWRITE_FIELD(io, ready1);
    FWRITE_FIELD(io, match1);
    FWRITE_BUFFER(io, prior1, prior1Size);
    FWRITE_FIELD(io, ready2);
    FWRITE_FIELD(io, match2);
    FWRITE_BUFFER(io, prior2, prior2Size);
    FWRITE_FIELD(io, isOnline);

done:
    return rc;
}

TcError Io_Write_DBG_LC_Output(Io* io, u8 profile, u16 sbAddr, u8 width, const void* key, int keySize, 
                                u8 ready, u8 match, const void* prior, int priorSize, 
                                u8 sed, const void* ad, int adSize, const void* adWidth, int adWidthSize)
{
    TcError rc = TcE_OK;
    int writeKeySize;
    u8 opCode = IO_DBG_LC_OUTPUT;

    writeKeySize = (width+1) * IO_KEY_MIN_SIZE_BYTE_COUNT;
    if (keySize < writeKeySize)
    {
        printError(commonLibLog, "key size too small. %X < %X\n",
                keySize, writeKeySize);
        rc = TcE_Invalid_Argument; goto done;
    }

    CHECK_ERROR(commonLibLog, Io_Write_SeqNumber(io));

    FWRITE_FIELD(io, opCode);
    FWRITE_FIELD(io, profile);
    FWRITE_FIELD(io, sbAddr);
    FWRITE_FIELD(io, width);
    FWRITE_BUFFER(io, key, writeKeySize);
    FWRITE_FIELD(io, ready);
    FWRITE_FIELD(io, match);
    FWRITE_BUFFER(io, prior, priorSize);
    FWRITE_FIELD(io, sed);
    FWRITE_BUFFER(io, ad, adSize);
    FWRITE_BUFFER(io, adWidth, adWidthSize);

done:
    return rc;
}

TcError Io_Write_DBG_NFE(Io* io, u8 dev, u8 octopus, u8 module, u8 memType, u8 type, const void* xorMask, int xorMaskSize)
{
    TcError rc = TcE_OK;
    u8 opCode = IO_DBG_NFE;

    CHECK_ERROR(commonLibLog, Io_Write_SeqNumber(io));
    FWRITE_FIELD(io, opCode);
    FWRITE_FIELD(io, dev);
    FWRITE_FIELD(io, octopus);
    FWRITE_FIELD(io, module);
    FWRITE_FIELD(io, memType);
    FWRITE_FIELD(io, type);
    FWRITE_BUFFER(io, xorMask, xorMaskSize);

done:
    return rc;
}

TcError Io_Write_DBG_LD3(Io* io, u8 dev, u32 address, const void* data, int dataSize )
{
    TcError rc = TcE_OK;
    u8 opCode = IO_DBG_LD3;

    CHECK_ERROR(commonLibLog, Io_Write_SeqNumber(io));
    FWRITE_FIELD(io, opCode);
    FWRITE_FIELD(io, dev);
    FWRITE_FIELD(io, address);
    if (dataSize < IO_DATA_SIZE_BYTE_COUNT)
    {
        char buff[IO_DATA_SIZE_BYTE_COUNT];
        memset(buff, 0, sizeof(buff));
        memcpy(buff, data, dataSize);
        FWRITE_BUFFER(io, buff, sizeof(buff));
    }
    else
    {
        FWRITE_BUFFER(io, data, IO_DATA_SIZE_BYTE_COUNT);
    }

done:
    return rc;
}

TcError Io_Write_MDIO(Io* io, u8 operation, u8 mpid, u8 devAddr, u16 regAddr, u16 regData)
{
    TcError rc = TcE_OK;
    u8 opCode = IO_MDIO;

    CHECK_ERROR(commonLibLog, Io_Write_SeqNumber(io));
    FWRITE_FIELD(io, opCode);
    FWRITE_FIELD(io, operation);
    FWRITE_FIELD(io, mpid);
    FWRITE_FIELD(io, devAddr);
    FWRITE_FIELD(io, regAddr);
    FWRITE_FIELD(io, regData);

done:
    return rc;
}

TcError Io_Write_Cmt_NseLib(Io* io, u8 cmnData)
{
    TcError rc = TcE_OK;
    u8 opCode = IO_CMT_NSELIB;

    CHECK_ERROR(commonLibLog, Io_Write_SeqNumber(io));
    
    FWRITE_FIELD(io, opCode);
    FWRITE_FIELD(io, cmnData);

done:
    return rc;
}

TcError Io_Write_Cmt_OnlineRecord(Io* io, u8 opCode, const char* tableName, u32 priority)
{
    TcError rc = TcE_OK;
    char buff[IO_TABLE_NAME_BYTE_COUNT];

    strncpy(buff, tableName, sizeof(buff));
    buff[IO_TABLE_NAME_BYTE_COUNT - 1] = 0;

    CHECK_ERROR(commonLibLog, Io_Write_SeqNumber(io));
    FWRITE_FIELD(io, opCode);
    FWRITE_BUFFER(io, buff, sizeof(buff));
    FWRITE_FIELD(io, priority);

done:
    return rc;
}

TcError Io_Write_Cmt_OnlineSplitAddRecord(Io* io, const char* tableName, u32 priority)
{
    return Io_Write_Cmt_OnlineRecord(io, IO_CMT_ONLINE_SPLIT_ADD_REC, tableName, priority);
}

TcError Io_Write_Cmt_OnlineDeleteRecord(Io* io, const char* tableName, u32 priority)
{
    return Io_Write_Cmt_OnlineRecord(io, IO_CMT_ONLINE_DEL_REC, tableName, priority);
}

TcError Io_Write_Cmt_OnlineAddRecord(Io* io, const char* tableName, u32 priority)
{
    return Io_Write_Cmt_OnlineRecord(io, IO_CMT_ONLINE_ADD_REC, tableName, priority);
}

TcError Io_Write_Cmt_OnlineTable(Io* io, u8 opCode, const char* tableName, u32 id)
{
    TcError rc = TcE_OK;
    char buff[IO_TABLE_NAME_BYTE_COUNT];

    strncpy(buff, tableName, sizeof(buff));
    buff[IO_TABLE_NAME_BYTE_COUNT - 1] = 0;

    CHECK_ERROR(commonLibLog, Io_Write_SeqNumber(io));
    FWRITE_FIELD(io, opCode);
    FWRITE_BUFFER(io, buff, sizeof(buff));
    FWRITE_FIELD(io, id);

done:
    return rc;
}

TcError Io_Write_Cmt_OnlineAddTable(Io* io, const char* tableName, u32 id)
{
    return Io_Write_Cmt_OnlineTable(io, IO_CMT_ONLINE_ADD_TABLE, tableName, id);
}

TcError Io_Write_Cmt_OnlineDeleteTable(Io* io, const char* tableName, u32 id)
{
    return Io_Write_Cmt_OnlineTable(io, IO_CMT_ONLINE_DEL_TABLE, tableName, id);
}

TcError Io_Write_Cmt_String(Io* io, const char* str, u8 len)
{
    TcError rc = TcE_OK;
    u8 opCode = IO_CMT_STRING;

    CHECK_ERROR(commonLibLog, Io_Write_SeqNumber(io));
    FWRITE_FIELD(io, opCode);
    FWRITE_FIELD(io, len);
    FWRITE_BUFFER(io, str, len);

done:
    return rc;
}

TcError Io_Write_Nop(Io* io, u16 cnt)
{
    TcError rc = TcE_OK;
    u8 opCode = IO_OPCODE_NOP;

    CHECK_ERROR(commonLibLog, Io_Write_SeqNumber(io));
    FWRITE_FIELD(io, opCode);
    FWRITE_FIELD(io, cnt);

done:
    return rc;
}

API int Io_Read_Magic(Io* io, u32* pMagic)
{
    TcError rc = TcE_OK;
    assertLog(pMagic != NULL);
    *pMagic = io->readMagic;
done:
    return rc;
}

API int Io_Read_Version(Io* io, u32* pVersion)
{
    TcError rc = TcE_OK;
    assertLog(pVersion != NULL);
    *pVersion = io->readVersion;
done:
    return rc;
}

TcError Io_Input_Open(const char* fn, Io** pIo)
{
   TcError rc = TcE_OK;
   Io* io = (Io*)malloc(sizeof(*io));

   assertLog(fn != NULL);

   if (!io)
   {
       printError(commonLibLog, "Out-of-memory error\n");
       rc = TcE_Out_Of_Memory; goto done;
   }
   if (strlen(fn) >= (sizeof(io->fname) - 1))
   {
       printError(commonLibLog, "file name too long. %s (%X) max = %X\n", 
                                fn, strlen(fn), sizeof(io->fname)-1);
       rc = TcE_Out_Of_Range; goto done;
   }

   io->fp = fopen(fn, "rb");
   if (NULL == io->fp)
   {
        printError(commonLibLog, "cannot open file %s for reading. \n", fn);
        return TcE_Failed;
   }

   CHECK_ERROR(commonLibLog,
           Io_Input_Reset(io));

   *pIo = io;

done:
   if ((rc) && (io))
   {
       Io_Close(io);
   }
   return rc;
}

static TcError Io_Private_Read_OpcodeAndSeqNumber(Io* io)
{
    TcError rc = TcE_OK;
    
    FREAD_FIELD(io, &io->seqNumber);

//    if (fread(&io->seqNumber, sizeof(io->seqNumber), 1, io->fp) != 1)
//    {
//        CHECK_FOR_EOF(io);
//        printError(commonLibLog, "failed to read seq number\n");
//        rc = TcE_Failed; goto done;
//    }     
    
    if (io->readExpectSeqNumber != io->seqNumber)
    {
        printError(commonLibLog, "sequence number mismatch. expected: %X actual: %X\n",
            io->readExpectSeqNumber, io->seqNumber);
        rc = TcE_Failed; goto done;
    }

    io->readExpectSeqNumber =  ++io->readExpectSeqNumber | 0xffff0000;

    FREAD_FIELD(io, &io->readOpcode);

//    if (fread(&io->readOpcode, sizeof(io->readOpcode), 1, io->fp) != 1)
//    {
//        printError(commonLibLog, "failed to read opcode\n");
//        rc = TcE_Failed; goto done;
//    }     

done:
    return rc;
}

TcError Io_Read_OpCode(Io* io, u8* pOpCode)
{
    TcError rc = TcE_OK; 
    if (io->readOpcode == IO_OPCODE_INVALID)
    {
        CHECK_ERROR_WARN(commonLibLog, Io_Private_Read_OpcodeAndSeqNumber(io));
    }

    *pOpCode = io->readOpcode;

done:
    return rc;
}

API TcError Io_Read_SeqNumber(Io* io, u32* pSeqNumber)
{
    TcError rc = TcE_OK; 
    if (io->readOpcode == IO_OPCODE_INVALID)
    {
        CHECK_ERROR_WARN(commonLibLog, Io_Private_Read_OpcodeAndSeqNumber(io));
    }

    *pSeqNumber = io->seqNumber;

done:
    return rc;
}

TcError Io_Read_LD2(Io* io, u8* pDev, void* data, int dataSize)
{
    TcError rc = TcE_OK;

    assertLog(io->readOpcode == IO_OPCODE_LD2);
    FREAD_FIELD(io, pDev);
    if (dataSize < IO_DATA_SIZE_BYTE_COUNT)
    {
        printError(commonLibLog, "output data size too small. %X expect >= %X\n",
                dataSize, IO_DATA_SIZE_BYTE_COUNT );
        rc = TcE_Failed; goto done;
    }
    FREAD_BUFFER(io, data, IO_DATA_SIZE_BYTE_COUNT);

    // signaling a complete command read
    io->readOpcode = IO_OPCODE_INVALID;

done:
    return rc;
}

TcError Io_Read_LD3(Io* io, u8* pDev, u32* pAddress)
{
    TcError rc = TcE_OK;

    assertLog(io->readOpcode == IO_OPCODE_LD3);
    FREAD_FIELD(io, pDev);
    FREAD_FIELD(io, pAddress);

    // signaling a complete command read
    io->readOpcode = IO_OPCODE_INVALID;

done:
    return rc;

}

static TcError Io_Read_LD5_LC(Io* io, u8* pProfile, u16* pSbAddr, u8* pWidth, void* key, int keySize)
{
    TcError rc = TcE_OK;
    int readKeySize;

    FREAD_FIELD(io, pProfile);
    FREAD_FIELD(io, pSbAddr);
    FREAD_FIELD(io, pWidth);

    readKeySize = (*pWidth+1) * IO_KEY_MIN_SIZE_BYTE_COUNT;

    if (keySize < readKeySize)
    {
        printError(commonLibLog, "output key size too small. %X expect >= %X\n",
                keySize, readKeySize);
        rc = TcE_Failed; goto done;
    }

    FREAD_BUFFER(io, key, readKeySize);

    // signaling a complete command read
    io->readOpcode = IO_OPCODE_INVALID;

done:
    return rc;
}

TcError Io_Read_LD5(Io* io, u8* pProfile, u16* pSbAddr, u8* pKeySize, void* data, int dataSize)
{
    TcError rc = TcE_OK;
    assertLog(io->readOpcode == IO_OPCODE_LD5);
    rc = Io_Read_LD5_LC(io, pProfile, pSbAddr, pKeySize, data, dataSize);
done:
    return rc;
}

TcError Io_Read_LC(Io* io, u8* pProfile, u16* pSbAddr, u8* pWidth, void* data, int dataSize)
{
    TcError rc = TcE_OK;
    assertLog(io->readOpcode == IO_OPCODE_LC);
    rc = Io_Read_LD5_LC(io, pProfile, pSbAddr, pWidth, data, dataSize);
done:
    return rc;
}

TcError Io_Read_DBG_LC_Input(Io* io, u8* pProfile, u16* pSbAddr, u8* pWidth, void* key, int keySize, 
                                u8* pReady1, u8* pMatch1, void* prior1, int prior1Size, 
                                u8* pReady2, u8* pMatch2, void* prior2, int prior2Size, 
                                u8* pIsOnline)
{
    TcError rc = TcE_OK;
    int readKeySize;

    assertLog(io->readOpcode == IO_DBG_LC_INPUT);

    FREAD_FIELD(io, pProfile);
    FREAD_FIELD(io, pSbAddr);
    FREAD_FIELD(io, pWidth);

    readKeySize = (*pWidth+1) * IO_KEY_MIN_SIZE_BYTE_COUNT;

    if (keySize < readKeySize)
    {
        printError(commonLibLog, "output key size too small. %X expect >= %X\n",
                keySize, readKeySize);
        rc = TcE_Failed; goto done;
    }

    FREAD_BUFFER(io, key, readKeySize);
    FREAD_FIELD(io, pReady1);
    FREAD_FIELD(io, pMatch1);
    if (prior1Size < IO_PRIORITY_BYTE_COUNT)
    {
        printError(commonLibLog, "priority1 size too small. %X expect >= %X\n",
                prior1Size, IO_PRIORITY_BYTE_COUNT);
        rc = TcE_Failed; goto done;
    }
    FREAD_BUFFER(io, prior1, IO_PRIORITY_BYTE_COUNT);
    FREAD_FIELD(io, pReady2);
    FREAD_FIELD(io, pMatch2);
    if (prior2Size < IO_PRIORITY_BYTE_COUNT)
    {
        printError(commonLibLog, "priority1 size too small. %X expect >= %X\n",
                prior1Size, IO_PRIORITY_BYTE_COUNT);
        rc = TcE_Failed; goto done;
    }
    FREAD_BUFFER(io, prior2, IO_PRIORITY_BYTE_COUNT);
    FREAD_FIELD(io, pIsOnline);

    // signaling a complete command read
    io->readOpcode = IO_OPCODE_INVALID;

done:
    return rc;
}

TcError Io_Read_DBG_LC_Output(Io* io, u8* pProfile, u16* pSbAddr, u8* pWidth, void* key, int keySize, 
                                u8* pReady, u8* pMatch, void* prior, int priorSize, 
                                u8* pSec, void* ad, int adSize, void* adWidth, int adWidthSize)
{
    TcError rc = TcE_OK;
    int readKeySize;

    assertLog(io->readOpcode == IO_DBG_LC_OUTPUT);

    FREAD_FIELD(io, pProfile);
    FREAD_FIELD(io, pSbAddr);
    FREAD_FIELD(io, pWidth);

    readKeySize = (*pWidth+1) * IO_KEY_MIN_SIZE_BYTE_COUNT;

    if (keySize < readKeySize)
    {
        printError(commonLibLog, "output key size too small. %X expect >= %X\n",
                keySize, readKeySize);
        rc = TcE_Failed; goto done;
    }

    FREAD_BUFFER(io, key, readKeySize);
    FREAD_FIELD(io, pReady);
    FREAD_FIELD(io, pMatch);
    if (priorSize < IO_PRIORITY_BYTE_COUNT)
    {
        printError(commonLibLog, "priority1 size too small. %X expect >= %X\n",
                priorSize, IO_PRIORITY_BYTE_COUNT);
        rc = TcE_Failed; goto done;
    }
    FREAD_BUFFER(io, prior, IO_PRIORITY_BYTE_COUNT);
    FREAD_FIELD(io, pSec);
    FREAD_BUFFER(io, ad, IO_AD_BYTE_COUNT);
    if (adSize < IO_AD_BYTE_COUNT)
    {
        printError(commonLibLog, "ad size too small. %X expect >= %X\n",
                adSize, IO_AD_BYTE_COUNT);
        rc = TcE_Failed; goto done;
    }
    FREAD_BUFFER(io, adWidth, IO_ADWIDTH_BYTE_COUNT);
    if (adWidthSize < IO_ADWIDTH_BYTE_COUNT)
    {
        printError(commonLibLog, "adWidth size too small. %X expect >= %X\n",
                adWidthSize, IO_ADWIDTH_BYTE_COUNT);
        rc = TcE_Failed; goto done;
    }

    // signaling a complete command read
    io->readOpcode = IO_OPCODE_INVALID;

done:
    return rc;
}

TcError Io_Read_DBG_NFE(Io* io, u8* pDev, u8* pOctopus, u8* pModule, u8* pMemType, u8* pType, void* xorMask, int xorMaskSize)
{
    TcError rc = TcE_OK;
    
    assertLog(io->readOpcode == IO_DBG_NFE);

    FREAD_FIELD(io, pDev);
    FREAD_FIELD(io, pOctopus);
    FREAD_FIELD(io, pModule);
    FREAD_FIELD(io, pMemType);
    FREAD_FIELD(io, pType);

    if (xorMaskSize < IO_NFE_XOR_MASK_BYTE_COUNT )
    {
        printError(commonLibLog, "xorMaskSize too small. %X expect >= %X\n",
                xorMaskSize, IO_NFE_XOR_MASK_BYTE_COUNT);
        rc = TcE_Failed; goto done;
    }

    FREAD_BUFFER(io, xorMask, IO_NFE_XOR_MASK_BYTE_COUNT);

    // signaling a complete command read
    io->readOpcode = IO_OPCODE_INVALID;

done:
    return rc;
}

TcError Io_Read_DBG_LD3(Io* io, u8* pDev, u32* pAddress, void* data, int dataSize )
{
    TcError rc = TcE_OK;

    assertLog(io->readOpcode == IO_DBG_LD3);

    FREAD_FIELD(io, pDev);
    FREAD_FIELD(io, pAddress);
    if (dataSize < IO_DATA_SIZE_BYTE_COUNT )
    {
        printError(commonLibLog, "data size too small. %X expect >= %X\n",
                dataSize, IO_DATA_SIZE_BYTE_COUNT);
        rc = TcE_Failed; goto done;
    }
    FREAD_BUFFER(io, data, IO_DATA_SIZE_BYTE_COUNT);

    // signaling a complete command read
    io->readOpcode = IO_OPCODE_INVALID;

done:
    return rc;
}

TcError Io_Read_MDIO(Io* io, u8* operation, u8* mpid, u8* devAddr, u16* regAddr, u16* regData)
{
    TcError rc = TcE_OK;

    assertLog(io->readOpcode == IO_MDIO);

    FREAD_FIELD(io, operation);
    FREAD_FIELD(io, mpid);
    FREAD_FIELD(io, devAddr);
    FREAD_FIELD(io, regAddr);
    FREAD_FIELD(io, regData);
    
    // signaling a complete command read
    io->readOpcode = IO_OPCODE_INVALID;

done:
    return rc;
}

TcError Io_Read_Cmt_NseLib(Io* io, u8* pCmnData)
{
    TcError rc = TcE_OK;
    assertLog(io->readOpcode == IO_CMT_NSELIB);
    FREAD_FIELD(io, pCmnData);

    // signaling a complete command read
    io->readOpcode = IO_OPCODE_INVALID;

done:
    return rc;
}

static TcError Io_Read_Cmt_OnlineRecord(Io* io, char* tableName, int tableNameSize, u32* pPriority)
{
    TcError rc = TcE_OK;

    if (tableNameSize < IO_TABLE_NAME_BYTE_COUNT)
    {
        char dummy[IO_TABLE_NAME_BYTE_COUNT];
        FREAD_BUFFER(io, tableName, tableNameSize);
        tableName[tableNameSize-1] = 0;
        FREAD_BUFFER(io, dummy, IO_TABLE_NAME_BYTE_COUNT - tableNameSize);
    }
    else
    {
        FREAD_BUFFER(io, tableName, IO_TABLE_NAME_BYTE_COUNT);
    }
    FREAD_FIELD(io, pPriority);

    // signaling a complete command read
    io->readOpcode = IO_OPCODE_INVALID;

done:
    return rc;
}

TcError Io_Read_Cmt_OnlineSplitAddRecord(Io* io, char* tableName, int tableNameSize,  u32* pPriority)
{
    TcError rc = TcE_OK;
    assertLog(io->readOpcode == IO_CMT_ONLINE_SPLIT_ADD_REC);
    rc = Io_Read_Cmt_OnlineRecord(io, tableName, tableNameSize, pPriority);
done:
    return rc;
}

TcError Io_Read_Cmt_OnlineDeleteRecord(Io* io, char* tableName, int tableNameSize,  u32* pPriority)
{
    TcError rc = TcE_OK;
    assertLog(io->readOpcode == IO_CMT_ONLINE_DEL_REC);
    rc = Io_Read_Cmt_OnlineRecord(io, tableName, tableNameSize, pPriority);
done:
    return rc;
}

TcError Io_Read_Cmt_OnlineAddRecord(Io* io, char* tableName, int tableNameSize,  u32* pPriority)
{
    TcError rc = TcE_OK;
    assertLog(io->readOpcode == IO_CMT_ONLINE_ADD_REC);
    rc = Io_Read_Cmt_OnlineRecord(io, tableName, tableNameSize, pPriority);
done:
    return rc;
}

TcError Io_Read_Cmt_OnlineTable(Io* io, char* tableName, int tableNameSize,  u32* pId)
{
    TcError rc = TcE_OK;

    if (tableNameSize < IO_TABLE_NAME_BYTE_COUNT)
    {
        printError(commonLibLog, "tableNameSize is too small %d expexted >= %d\n",
                        tableNameSize, IO_TABLE_NAME_BYTE_COUNT);
        rc = TcE_Buffer_Too_Small; goto done;
    }
    FREAD_BUFFER(io, tableName, IO_TABLE_NAME_BYTE_COUNT);
    FREAD_FIELD(io, pId);

    // signaling a complete command read
    io->readOpcode = IO_OPCODE_INVALID;

done:
    return rc;
}

TcError Io_Read_Cmt_OnlineDeleteTable(Io* io, char* tableName, int tableNameSize,  u32* pId)
{
    TcError rc = TcE_OK;
    assertLog(io->readOpcode == IO_CMT_ONLINE_DEL_TABLE);
    rc = Io_Read_Cmt_OnlineTable(io, tableName, tableNameSize, pId);
done:
    return rc;
}

TcError Io_Read_Cmt_OnlineAddTable(Io* io, char* tableName, int tableNameSize,  u32* pId)
{
    TcError rc = TcE_OK;
    assertLog(io->readOpcode == IO_CMT_ONLINE_ADD_TABLE);
    rc = Io_Read_Cmt_OnlineTable(io, tableName, tableNameSize, pId);
done:
    return rc;
}

TcError Io_Read_Cmt_String(Io* io, char* str, u8* pLen)
{
    TcError rc = TcE_OK;
    u8 maxLen = 0;
    u8 len = 0;

    assertLog(io->readOpcode == IO_CMT_STRING);

    if (pLen) maxLen=*pLen;

    FREAD_FIELD(io, &len);
    
    if (maxLen >= len)
    {
       FREAD_BUFFER(io, str, len); 
    }
    else
    {
       char byte;
       int i;
       FREAD_BUFFER(io, str, maxLen-1);
       str[maxLen-1]=0;
       for (i=maxLen-1; i<len; i++)
            FREAD_FIELD(io, &byte); 
    }

    // signaling a complete command read
    io->readOpcode = IO_OPCODE_INVALID;

done:
    return rc;
}

TcError Io_Read_Nop(Io* io, u16* pCnt)
{
    TcError rc = TcE_OK;

    assertLog(io->readOpcode == IO_OPCODE_NOP);
    FREAD_FIELD(io, pCnt);

    // signaling a complete command read
    io->readOpcode = IO_OPCODE_INVALID;

done:
    return rc;
}

TcError Io_Read_Quiet(Io* io)
{
    TcError rc = TcE_OK;
    assertLog(io->readOpcode == IO_OPCODE_QUIET);

    // signaling a complete command read
    io->readOpcode = IO_OPCODE_INVALID;

done:
    return rc;
}

// write IoCmd to file 
TcError Io_IoCmd_Write(Io* io, const IoCmd* cmd)
{
    TcError rc = TcE_OK;
    u8 opCode;

    CHECK_ERROR(commonLibLog,
        IoCmd_Get_OpCode(cmd, &opCode));

    switch (opCode)
    {
        case IO_OPCODE_NOP:
        {
            u16 cnt;
            CHECK_ERROR(commonLibLog,
                IoCmd_Get_Nop(cmd, &cnt));

            CHECK_ERROR(commonLibLog, Io_Write_Nop(io, cnt));
        }
        break;
        case IO_OPCODE_QUIET:
        {
            CHECK_ERROR(commonLibLog, Io_Write_Quiet(io));
        }
        break;
        case IO_OPCODE_LC:
        {
            u8 profile;
            u16 sbAddr;
            u8 width;
            const void* key;
            int keySize;

            CHECK_ERROR(commonLibLog, 
                IoCmd_Get_LC(cmd, &profile, &sbAddr, &width, &key, &keySize));

            CHECK_ERROR(commonLibLog, 
                Io_Write_LC(io, profile, sbAddr, width, key, keySize));
        }
        break;
        case IO_OPCODE_LD5:
        {
            u8 profile;
            u16 sbAddr;
            u8 width;
            const void* key;
            int keySize;

            CHECK_ERROR(commonLibLog, 
                IoCmd_Get_LD5(cmd, &profile, &sbAddr, &width, &key, &keySize));

            CHECK_ERROR(commonLibLog, 
                Io_Write_LD5(io, profile, sbAddr, width, key, keySize));
        }
        break;
        case IO_OPCODE_LD2:
        {
            u8 devId;
            const void* data;
            int dataSize;

            CHECK_ERROR(commonLibLog, 
                IoCmd_Get_LD2(cmd, &devId, &data, &dataSize));

            CHECK_ERROR(commonLibLog, 
                Io_Write_LD2(io, devId, data, dataSize));
        }
        break;
        case IO_OPCODE_LD3:
        {
            u8 devId;
            u32 address;

            CHECK_ERROR(commonLibLog, 
                IoCmd_Get_LD3(cmd, &devId, &address));

            CHECK_ERROR(commonLibLog, 
                Io_Write_LD3(io, devId, address));
        }
        break;
        case IO_DBG_LC_INPUT:
        {
            u8 profile;
            u16 sbAddr;
            u8 width;
            const void* key;
            int keySize;
            u8 ready1;
            u8 match1;
            const void* prior1;
            int prior1Size;
            u8 ready2;
            u8 match2;
            const void* prior2;
            int prior2Size;
            u8 isOnline;

            CHECK_ERROR(commonLibLog, 
                IoCmd_Get_DBG_LC_Input(cmd, &profile, &sbAddr, &width, &key, &keySize,
                                                                &ready1, &match1, &prior1, &prior1Size,
                                                                &ready2, &match2, &prior2, &prior2Size,
                                                                &isOnline));
            CHECK_ERROR(commonLibLog, 
                Io_Write_DBG_LC_Input(io, profile, sbAddr, width, key, keySize,
                                                                ready1, match1, prior1, prior1Size,
                                                                ready2, match2, prior2, prior2Size,
                                                                isOnline));
        }
        break;
        case IO_DBG_LC_OUTPUT:
        {
            u8 profile;
            u16 sbAddr;
            u8 width;
            const void* key;
            int keySize;
            u8 ready;
            u8 match;
            const void* prior;
            int priorSize;
            u8 sec;
            const void* ad;
            int adSize;
            const void* adWidth;
            int adWidthSize;

            CHECK_ERROR(commonLibLog, 
                IoCmd_Get_DBG_LC_Output(cmd, &profile, &sbAddr, &width, &key, &keySize,
                                                                &ready, &match, &prior, &priorSize,
                                                                &sec, &ad, &adSize, &adWidth, &adWidthSize));
            CHECK_ERROR(commonLibLog, 
                Io_Write_DBG_LC_Output(io, profile, sbAddr, width, key, keySize,
                                                                ready, match, prior, priorSize,
                                                                sec, ad, adSize, adWidth, adWidthSize));
        }
        break;
        case IO_DBG_NFE:
        {
            u8 dev;
            u8 octopus;
            u8 module;
            u8 memType;
            u8 type;
            const void* xorMask;
            int xorMaskSize;

            CHECK_ERROR(commonLibLog, 
                IoCmd_Get_DBG_NFE(cmd, &dev, &octopus, &module, &memType, &type, &xorMask, &xorMaskSize));

            CHECK_ERROR(commonLibLog, 
                Io_Write_DBG_NFE(io, dev, octopus, module, memType, type, xorMask, xorMaskSize));
        }
        break;
        case IO_DBG_LD3:
        {
            u8 dev;
            u32 address;
            const void* data;
            int dataSize;

            CHECK_ERROR(commonLibLog, 
                IoCmd_Get_DBG_LD3(cmd, &dev, &address, &data, &dataSize));

            CHECK_ERROR(commonLibLog, 
                Io_Write_DBG_LD3(io, dev, address, data, dataSize));
        }
        break;
        case IO_MDIO:
        {
            u8 operation;
            u8 mpid;
            u8 devAddr;
            u16 regAddr;
            u16 regData;

            CHECK_ERROR(commonLibLog, 
                IoCmd_Get_MDIO(cmd, &operation, &mpid, &devAddr, &regAddr, &regData));

            CHECK_ERROR(commonLibLog, 
                Io_Write_MDIO(io, operation, mpid, devAddr, regAddr, regData));
        }
        break;
        case IO_CMT_NSELIB:
        {
            u8 data;
            CHECK_ERROR(commonLibLog, 
                IoCmd_Get_Cmt_NseLib(cmd, &data));

            CHECK_ERROR(commonLibLog, 
                Io_Write_Cmt_NseLib(io, data));
        }
        break;
        case IO_CMT_ONLINE_SPLIT_ADD_REC:
        {
            const char* tableName;
            u32 prior;

            CHECK_ERROR(commonLibLog, 
                IoCmd_Get_Cmt_OnlineSplitAddRecord(cmd, &tableName, &prior));

            CHECK_ERROR(commonLibLog, 
                Io_Write_Cmt_OnlineSplitAddRecord(io, tableName, prior));
        }
        break;
        case IO_CMT_ONLINE_ADD_REC:
        {
            const char* tableName;
            u32 prior;

            CHECK_ERROR(commonLibLog, 
                IoCmd_Get_Cmt_OnlineAddRecord(cmd, &tableName, &prior));

            CHECK_ERROR(commonLibLog, 
                Io_Write_Cmt_OnlineAddRecord(io, tableName, prior));
        }
        break;
        case IO_CMT_ONLINE_DEL_REC:
        {
            const char* tableName;
            u32 prior;

            CHECK_ERROR(commonLibLog, 
                IoCmd_Get_Cmt_OnlineDeleteRecord(cmd, &tableName, &prior));

            CHECK_ERROR(commonLibLog, 
                Io_Write_Cmt_OnlineDeleteRecord(io, tableName, prior));
        }
        break;
        case IO_CMT_ONLINE_ADD_TABLE:
        {
            const char* tableName;
            u32 id;

            CHECK_ERROR(commonLibLog, 
                IoCmd_Get_Cmt_OnlineAddTable(cmd, &tableName, &id));

            CHECK_ERROR(commonLibLog, 
                Io_Write_Cmt_OnlineAddTable(io, tableName, id));
        }
        break;
        case IO_CMT_ONLINE_DEL_TABLE:
        {
            const char* tableName;
            u32 id;

            CHECK_ERROR(commonLibLog, 
                IoCmd_Get_Cmt_OnlineDeleteTable(cmd, &tableName, &id));

            CHECK_ERROR(commonLibLog, 
                Io_Write_Cmt_OnlineDeleteTable(io, tableName, id));
        }
        break;
        case IO_CMT_STRING:
        {
           const char* str = NULL;
           u8 len=0;
            CHECK_ERROR(commonLibLog, 
                IoCmd_Get_Cmt_String (cmd, &str, &len));

            CHECK_ERROR(commonLibLog, 
                Io_Write_Cmt_String(io, str, len));
        }
        break;
        default:
        {
            printError(commonLibLog, "unknown opCode. %X\n", opCode);
            rc = TcE_Invalid_Argument; goto done;
        }
        break;
    }

done:
    return rc;
}

// read IoCmd from file
TcError Io_IoCmd_Read(Io* io, IoCmd* cmd)
{
    TcError rc = TcE_OK;
    u8 opCode;

    CHECK_ERROR_WARN(commonLibLog, 
            Io_Read_OpCode(io, &opCode));

    switch (opCode)
    {
        case IO_OPCODE_NOP:
        {
            u16 cnt;

            CHECK_ERROR(commonLibLog, 
                Io_Read_Nop(io, &cnt));

            CHECK_ERROR(commonLibLog,
                IoCmd_Set_Nop(cmd, &cnt));

        }
        break;
        case IO_OPCODE_QUIET:
        {
            CHECK_ERROR(commonLibLog, 
                Io_Read_Quiet(io));

            CHECK_ERROR(commonLibLog,
                IoCmd_Set_Quiet(cmd));
        }
        break;
        case IO_OPCODE_LC:
        {
            u8 profile;
            u16 sbAddr;
            u8 width;
            char key[IO_KEY_MAX_SIZE_BYTE_COUNT];

            CHECK_ERROR(commonLibLog, 
                Io_Read_LC(io, &profile, &sbAddr, &width, &key, sizeof(key)));

            CHECK_ERROR(commonLibLog, 
                IoCmd_Set_LC(cmd, &profile, &sbAddr, &width, key, sizeof(key)));
        }
        break;
        case IO_OPCODE_LD5:
        {
            u8 profile;
            u16 sbAddr;
            u8 width;
            char key[IO_KEY_MAX_SIZE_BYTE_COUNT];
            int keySize = sizeof(key);

            CHECK_ERROR(commonLibLog, 
                Io_Read_LD5(io, &profile, &sbAddr, &width, &key, keySize));

            CHECK_ERROR(commonLibLog, 
                IoCmd_Set_LD5(cmd, &profile, &sbAddr, &width, key, keySize));
        }
        break;
        case IO_OPCODE_LD2:
        {
            u8 devId;
            char data[IO_DATA_SIZE_BYTE_COUNT];
            int dataSize = sizeof(data);

            CHECK_ERROR(commonLibLog, 
                Io_Read_LD2(io, &devId, &data, dataSize));

            CHECK_ERROR(commonLibLog, 
                IoCmd_Set_LD2(cmd, &devId, data, dataSize));
        }
        break;
        case IO_OPCODE_LD3:
        {
            u8 devId;
            u32 address;

            CHECK_ERROR(commonLibLog, 
                Io_Read_LD3(io, &devId, &address));

            CHECK_ERROR(commonLibLog, 
                IoCmd_Set_LD3(cmd, &devId, &address));
        }
        break;
        case IO_DBG_LC_INPUT:
        {
            u8 profile;
            u16 sbAddr;
            u8 width;
            char key[IO_KEY_MAX_SIZE_BYTE_COUNT];
            int keySize = sizeof(key);
            u8 ready1;
            u8 match1;
            char prior1[IO_PRIORITY_BYTE_COUNT];
            int prior1Size = sizeof(prior1);
            u8 ready2;
            u8 match2;
            char prior2[IO_PRIORITY_BYTE_COUNT];
            int prior2Size = sizeof(prior2);
            u8 isOnline;

            CHECK_ERROR(commonLibLog, 
                Io_Read_DBG_LC_Input(io, &profile, &sbAddr, &width, key, keySize,
                                                                &ready1, &match1, prior1, prior1Size,
                                                                &ready2, &match2, prior2, prior2Size,
                                                                &isOnline));
            CHECK_ERROR(commonLibLog, 
                IoCmd_Set_DBG_LC_Input(cmd, &profile, &sbAddr, &width, key, keySize,
                                                                &ready1, &match1, prior1, prior1Size,
                                                                &ready2, &match2, prior2, prior2Size,
                                                                &isOnline));
        }
        break;
        case IO_DBG_LC_OUTPUT:
        {
            u8 profile;
            u16 sbAddr;
            u8 width;
            char key[IO_KEY_MAX_SIZE_BYTE_COUNT];
            int keySize = sizeof(key);
            u8 ready;
            u8 match;
            char prior[IO_PRIORITY_BYTE_COUNT];
            int priorSize = sizeof(prior);
            u8 sec;
            char ad[IO_AD_BYTE_COUNT];
            char adWidth[IO_ADWIDTH_BYTE_COUNT];

            CHECK_ERROR(commonLibLog, 
                Io_Read_DBG_LC_Output(io, &profile, &sbAddr, &width, key, keySize,
                                                                &ready, &match, prior, priorSize,
                                                                &sec, ad, sizeof(ad), adWidth, sizeof(adWidth)));
            CHECK_ERROR(commonLibLog, 
                IoCmd_Set_DBG_LC_Output(cmd, &profile, &sbAddr, &width, key, keySize,
                                                                &ready, &match, prior, priorSize,
                                                                &sec, ad, sizeof(ad), adWidth, sizeof(adWidth)));
        }
        break;
        case IO_DBG_NFE:
        {
            u8 dev;
            u8 octopus;
            u8 module;
            u8 memType;
            u8 type;
            char xorMask[IO_NFE_XOR_MASK_BYTE_COUNT];
            int xorMaskSize = sizeof(xorMask);

            CHECK_ERROR(commonLibLog, 
                Io_Read_DBG_NFE(io, &dev, &octopus, &module, &memType, &type, xorMask, xorMaskSize));

            CHECK_ERROR(commonLibLog, 
                IoCmd_Set_DBG_NFE(cmd, &dev, &octopus, &module, &memType, &type, xorMask, xorMaskSize));
        }
        break;
        case IO_DBG_LD3:
        {
            u8 dev;
            u32 address;
            char data[IO_DATA_SIZE_BYTE_COUNT];
            int dataSize = sizeof(data);

            CHECK_ERROR(commonLibLog, 
                Io_Read_DBG_LD3(io, &dev, &address, data, dataSize));

            CHECK_ERROR(commonLibLog, 
                IoCmd_Set_DBG_LD3(cmd, &dev, &address, data, dataSize));
        }
        break;
        case IO_CMT_NSELIB:
        {
            u8 data;

            CHECK_ERROR(commonLibLog, 
                Io_Read_Cmt_NseLib(io, &data));

            CHECK_ERROR(commonLibLog, 
                IoCmd_Set_Cmt_NseLib(cmd, &data));
        }
        break;
        case IO_CMT_ONLINE_SPLIT_ADD_REC:
        {
            char tableName[IO_TABLE_NAME_BYTE_COUNT];
            u32 prior;

            CHECK_ERROR(commonLibLog, 
                Io_Read_Cmt_OnlineSplitAddRecord(io, tableName, sizeof(tableName), &prior));

            CHECK_ERROR(commonLibLog, 
                IoCmd_Set_Cmt_OnlineSplitAddRecord(cmd, tableName, &prior));
        }
        break;
        case IO_CMT_ONLINE_ADD_REC:
        {
            char tableName[IO_TABLE_NAME_BYTE_COUNT];
            u32 prior;

            CHECK_ERROR(commonLibLog, 
                Io_Read_Cmt_OnlineAddRecord(io, tableName, sizeof(tableName), &prior));

            CHECK_ERROR(commonLibLog, 
                IoCmd_Set_Cmt_OnlineAddRecord(cmd, tableName, &prior));
        }
        break;
        case IO_CMT_ONLINE_DEL_REC:
        {
            char tableName[IO_TABLE_NAME_BYTE_COUNT];
            u32 prior;

            CHECK_ERROR(commonLibLog, 
                Io_Read_Cmt_OnlineDeleteRecord(io, tableName, sizeof(tableName), &prior));

            CHECK_ERROR(commonLibLog, 
                IoCmd_Set_Cmt_OnlineDeleteRecord(cmd, tableName, &prior));
        }
        break;
        case IO_CMT_ONLINE_ADD_TABLE:
        {
            char tableName[IO_TABLE_NAME_BYTE_COUNT];
            u32 id;

            CHECK_ERROR(commonLibLog, 
                Io_Read_Cmt_OnlineAddTable(io, tableName, sizeof(tableName), &id));

            CHECK_ERROR(commonLibLog, 
                IoCmd_Set_Cmt_OnlineAddTable(cmd, tableName, &id));
        }
        break;
        case IO_CMT_ONLINE_DEL_TABLE:
        {
            char tableName[IO_TABLE_NAME_BYTE_COUNT];
            u32 id;

            CHECK_ERROR(commonLibLog, 
                Io_Read_Cmt_OnlineDeleteTable(io, tableName, sizeof(tableName), &id));

            CHECK_ERROR(commonLibLog, 
                IoCmd_Set_Cmt_OnlineDeleteTable(cmd, tableName, &id));
        }
        break;
        case IO_CMT_STRING:
        {
           char buff[128];
           u8 len = sizeof(buff);

            CHECK_ERROR(commonLibLog, 
                Io_Read_Cmt_String(io, buff, &len));

            CHECK_ERROR(commonLibLog, 
                IoCmd_Set_Cmt_String(cmd, buff));

        }
        break;
        case IO_MDIO:
        {
            u8 oper;
            u8 mpid;
            u8 devAddr;
            u16 regAddr;
            u16 regData;

            CHECK_ERROR(commonLibLog,
                Io_Read_MDIO(io, &oper, &mpid, &devAddr, &regAddr, &regData));

            CHECK_ERROR(commonLibLog, 
                IoCmd_Set_MDIO(cmd, &oper, &mpid, &devAddr, &regAddr, &regData));

        }
        break;
        default:
        {
            printError(commonLibLog, "unknown opCode. %X\n", opCode);
            rc = TcE_Invalid_Argument; goto done;
        }
        break;
    }

done:
    return rc;
}

TcError Io_Input_Reset(Io* io)
{
    TcError rc = TcE_OK;
//    int cnt;

    fseek(io->fp, 0, SEEK_SET);

    FREAD_FIELD(io, &io->readMagic);
//    if ((cnt=fread(&io->readMagic, sizeof(io->readMagic), 1, io->fp)) != 1)
//    {
//        printError(commonLibLog, "cannot read magic number. file: %s \n", io->fp);
//        return TcE_Invalid_Argument;
//    }

    if (IO_MAGIC_NUMBER != io->readMagic )
    {
        printError(commonLibLog, "magic number mismatch: expected: %X actual: %X \n", 
                         IO_MAGIC_NUMBER, io->readMagic);
        return TcE_Invalid_Argument;
    }

    FREAD_FIELD(io, &io->readVersion);
//    if ((cnt=fread(&io->readVersion, sizeof(io->readVersion), 1, io->fp)) != 1)
//    {
//        printError(commonLibLog, "cannot read version number. file %s\n", io->fp);
//        return TcE_Invalid_Argument;
//    }
//

    if (IO_VERSION_NUMBER != io->readVersion)
    {
        printError(commonLibLog, "version mismatch expected: %X actual: %X\n", IO_VERSION_NUMBER, io->readVersion);
        return TcE_Invalid_Argument;
    }

    io->readOpcode = IO_OPCODE_INVALID;
    io->seqNumber = SEQUENCE_NUMBER_INVALID;
    io->readExpectSeqNumber = 0 | 0xffff0000;
   
done: 
    return rc;
}

API TcError Io_Write_Quiet(Io* io)
{
    TcError rc = TcE_OK;
    u8 opCode = IO_OPCODE_QUIET;

    CHECK_ERROR(commonLibLog, Io_Write_SeqNumber(io));
    FWRITE_FIELD(io, opCode);

done:
    return rc;
}
