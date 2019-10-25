#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "Io.h"
#include "IoCmd.h"
#include "log.h"

typedef struct sDataOpCodeQuiet 
{
    u8 opCode;
    u16 delay;
}
DataOpCodeQuiet;

typedef struct sDataOpCodeNop 
{
    u8 opCode;
    u16 delay;
}
DataOpCodeNop;

typedef struct sDataOpCode_LD5_LC
{
    u8 opCode;
    u8 profile;
    u16 sbAddr;
    u8 width;
    u8 key[IO_KEY_MAX_SIZE_BYTE_COUNT];
}
DataOpCode_LD5_LC;

typedef struct sDataOpCodeLD2
{
    u8 opCode;
    u8 dev;
    u8 data[IO_DATA_SIZE_BYTE_COUNT];
}
DataOpCodeLD2;

typedef struct sDataOpCodeLD3
{
    u8 opCode;
    u8 dev;
    u32 address;
}
DataOpCodeLD3;

typedef struct sDataDbgLCLpmAD
{
    u8 opCode;
    u8 profile;
    u16 sbAddr;
    u8 width;
    u8 key[BYTE_COUNT(144)];
    u8 ready1;
    u8 match1;
    u8 prior1[IO_PRIORITY_BYTE_COUNT];
    u8 sec;
    u8 addata[BYTE_COUNT(4*256)];
    u8 adwidth[BYTE_COUNT(4*3)];
}
DataDbgLCLpmAD;

typedef struct sDataDbgLCInput
{
    u8 opCode;
    u8 profile;
    u16 sbAddr;
    u8 width;
    u8 key[IO_KEY_MAX_SIZE_BYTE_COUNT];
    u8 ready1;
    u8 match1;
    u8 prior1[IO_PRIORITY_BYTE_COUNT];
    u8 ready2;
    u8 match2;
    u8 prior2[IO_PRIORITY_BYTE_COUNT];
    u8 isOnline;
}
DataDbgLCInput;

typedef struct sDataDbgLCOutput
{
    u8 opCode;
    u8 profile;
    u16 sbAddr;
    u8 width;
    u8 key[IO_KEY_MAX_SIZE_BYTE_COUNT];
    u8 ready;
    u8 match;
    u8 prior[IO_PRIORITY_BYTE_COUNT];
    u8 sec;
    u8 ad[IO_AD_BYTE_COUNT];
    u8 adwidth[IO_ADWIDTH_BYTE_COUNT];
}
DataDbgLCOutput;

typedef struct sDataDbgNFE
{
    u8 opCode;
    u8 dev;
    u8 octopus;
    u8 module;
    u8 memType;
    u8 type;
    u8 xorMask[IO_NFE_XOR_MASK_BYTE_COUNT];
}
DataDbgNFE;

typedef struct sDataDbgLD3
{
    u8 opCode;
    u8 dev;
    u8 octopus;
    u32 address;
    u8 data[IO_DATA_SIZE_BYTE_COUNT];
}
DataDbgLD3;

typedef struct sDataMdio
{
    u8 opCode;
    u8 operation;
    u8 mpid;
    u8 devAddr;
    u16 regAddr;
    u16 regData;
}
DataMdio;

typedef struct sDataCmtNseLib
{
    u8 opCode;
    u8 data;
}
DataCmtNseLib;

typedef struct sDataCmtOnlineRecord
{
    u8 opCode;
    char tableName[IO_TABLE_NAME_BYTE_COUNT];
    u32 prior;
}
DataCmtOnlineRecord;

typedef struct sDataCmtOnlineTable
{
    u8 opCode;
    char tableName[IO_TABLE_NAME_BYTE_COUNT];
    u32 id;
}
DataCmtOnlineTable;

typedef struct sDataCmtString
{
    u8  opCode;
    u8  len;
    char str[IO_CMT_STRING_MAX];
}
DataCmtString;

struct sIoCmd
{
    union 
    {
        u8                  opCode;
        DataOpCodeNop       nop;
        DataOpCode_LD5_LC   ld5_lc;
        DataOpCodeLD2       ld2;
        DataOpCodeLD3       ld3;
        DataDbgLCInput      lcInput;
        DataDbgLCOutput     lcOutput;
	DataDbgLCLpmAD      lclpmAD;
        DataDbgNFE          nfe;
        DataDbgLD3          dbgLd3;
        DataMdio            mdio;
        DataCmtNseLib       cmtNseLib;
        DataCmtOnlineRecord onlineRecord;
        DataCmtOnlineTable  onlineTable;
        DataOpCodeQuiet     quiet;
        DataCmtString       cmtString;
    } u;
};

// create IoCmd large enough to handle longest cmd
IoCmd* IoCmd_Create(void)
{
    IoCmd* cmd = (IoCmd*)malloc(sizeof(IoCmd));

    if (NULL == cmd)
    {
        printError(commonLibLog, "failed to allocate memory for cmd\n");
        return NULL;
    }
    cmd->u.opCode = IO_OPCODE_INVALID; 
    return cmd;
}

IoCmd* IoCmd_CreateCopy(const IoCmd* cmd)
{
    IoCmd* copyCmd = IoCmd_Create();
    
    if (NULL == cmd)
        return NULL;

    memcpy(copyCmd, cmd, sizeof(cmd[0]));
    return copyCmd;
}

void IoCmd_Destroy(IoCmd* cmd)
{
    if (cmd)
        free(cmd);
}

TcError IoCmd_OpCode_Get(const IoCmd* cmd, u8* pOpcode)
{
    *pOpcode = cmd->u.opCode;
    return TcE_OK;
}

TcError IoCmd_Set_Nop(IoCmd* cmd, const u16* pCnt)
{
    TcError rc = TcE_OK;
    DataOpCodeNop* p = &cmd->u.nop;

    assertLog(cmd != NULL);
    assertLog(pCnt != NULL);

    p->opCode = IO_OPCODE_NOP;
    p->delay = *pCnt;

done:
    return rc;
}

TcError IoCmd_Set_Quiet(IoCmd* cmd)
{
    TcError rc = TcE_OK;
    DataOpCodeQuiet* p = &cmd->u.quiet;

    assertLog(cmd != NULL);

    p->opCode = IO_OPCODE_QUIET;

done:
    return rc;
}

static TcError IoCmd_Set_LD5_LC(IoCmd* cmd, u8 opCode, const u8* pProfile, const u16* pSbAddr, const u8* pWidth, const void* key, int keySize)
{
    TcError rc = TcE_OK;
    DataOpCode_LD5_LC* p = &cmd->u.ld5_lc;
    int keyWriteSize;
  
    assertLog(cmd != NULL);

    if ((p->opCode == IO_OPCODE_INVALID)                   // uninitialize cmd
        ||                                                                        
        ((pProfile) && (pSbAddr) && (pWidth) && (key)))    // all parameters given
    {
        p->opCode = opCode;
    }
    else if (p->opCode != opCode)
    {
        printError(commonLibLog, "opCode mismatch: current: %X (%s) change to %X (%s)\n", p->opCode, IoCmd_Opcode_Name(p->opCode), opCode, IoCmd_Opcode_Name(opCode));
        return TcE_Logic_Error;
    }
    

    if (pProfile)   { p->profile    = *pProfile; }
    if (pSbAddr)    { p->sbAddr     = *pSbAddr; }
    if (pWidth)
    {
        p->width = *pWidth;
        if (key)
        {
            keyWriteSize = (*pWidth+1)*IO_KEY_MIN_SIZE_BYTE_COUNT;
            if (keySize < keyWriteSize)
            {
                printError(commonLibLog, "key size too small. %X expected >= %X\n", 
                            keySize, keyWriteSize);
                return TcE_Buffer_Too_Small;
            }
            memcpy(p->key, key, keyWriteSize); 
        }
    }

done:
    return rc;
}

TcError IoCmd_Set_LC(IoCmd* cmd, const u8* pProfile, const u16* pSbAddr, const u8* pWidth, const void* key, int keySize)
{
    return IoCmd_Set_LD5_LC(cmd, IO_OPCODE_LC, pProfile, pSbAddr, pWidth, key, keySize);
}

TcError IoCmd_Set_LD2(IoCmd* cmd, const u8* pDev, const void* data, int dataSize)
{
    TcError rc = TcE_OK;
    DataOpCodeLD2* p = &cmd->u.ld2;

    assertLog(cmd != NULL);

    if ((p->opCode == IO_OPCODE_INVALID)    // uninitialize cmd
        ||
        ((pDev) && (data)))                 // all parameters given
    {
        p->opCode = IO_OPCODE_LD2;
    }
    else if (p->opCode != IO_OPCODE_LD2)
    {
        printError(commonLibLog, "opCode mismatch: current: %X (%s) change to %X (%s)\n", p->opCode, IoCmd_Opcode_Name(p->opCode), IO_OPCODE_LD2, IoCmd_Opcode_Name(IO_OPCODE_LD2));
        return TcE_Logic_Error;
    }

    if (pDev) { p->dev = *pDev; }
    if (data) 
    {
        if (dataSize >= IO_DATA_SIZE_BYTE_COUNT)
        {
            memcpy(p->data, data, IO_DATA_SIZE_BYTE_COUNT);
        }
        else
        {
            memset(p->data, 0, IO_DATA_SIZE_BYTE_COUNT);
            memcpy(p->data, data, dataSize);
        }
    }

done:
    return rc;
}

TcError IoCmd_Set_LD3(IoCmd* cmd, const u8* pDev, const u32* pAddress)
{
    TcError rc = TcE_OK;
    DataOpCodeLD3* p = &cmd->u.ld3;

    assertLog(cmd != NULL);
    assertLog(pAddress != NULL);

    if ((p->opCode == IO_OPCODE_INVALID)    // uninitialize cmd
        ||                                                         
        ( (pDev) && (pAddress)))            // all parameters given
            
    {
        p->opCode = IO_OPCODE_LD3;
    }
    else if (p->opCode != IO_OPCODE_LD3)
    {
        printError(commonLibLog, "opCode mismatch: current: %X (%s) change to %X (%s)\n", p->opCode, IoCmd_Opcode_Name(p->opCode), IO_OPCODE_LD3, IoCmd_Opcode_Name(IO_OPCODE_LD3));
        return TcE_Logic_Error;
    }
    if (pDev)      { p->dev = *pDev; }
    if (pAddress)  { p->address = *pAddress; }

done:
    return rc;
}

TcError IoCmd_Set_LD5(IoCmd* cmd, const u8* pProfile, const u16* pSbAddr, const u8* pWidth, const void* key, int keySize)
{
    return IoCmd_Set_LD5_LC(cmd, IO_OPCODE_LD5, pProfile, pSbAddr, pWidth, key, keySize);
}

TcError IoCmd_Set_DBG_LC_LpmAD(IoCmd* cmd, const u8* pProfile, const u16* pSbAddr, const u8* pWidth, const void* key, int keySize, 
                                    const u8* pReady1, const u8* pMatch1, const void* prior1, int prior1Size, 
                                    const u8* psec, const void* addata, int adsize, const void* adwidth, int adwidthsize)
{
    TcError rc = TcE_OK;
    DataDbgLCLpmAD* p = &cmd->u.lclpmAD;
  
    assertLog(cmd != NULL);

    if ((p->opCode == IO_OPCODE_INVALID)                      // uninitialize cmd
        ||                                                                           
        ((pProfile) && (pSbAddr) && (pWidth) && (key) &&      // all parameters given
         (pReady1)  && (pMatch1) && (prior1) &&
         (psec)))
    {
        p->opCode = IO_DBG_LC_LPMAD;
    }
    else if (p->opCode != IO_DBG_LC_INPUT)
    {
        printError(commonLibLog, "opCode mismatch: current: %X (%s) change to %X (%s)\n", p->opCode, IoCmd_Opcode_Name(p->opCode), IO_DBG_LC_INPUT, IoCmd_Opcode_Name(IO_DBG_LC_INPUT));
        return TcE_Logic_Error;
    }

    if (pProfile)   { p->profile = *pProfile; }
    if (pSbAddr)    { p->sbAddr = *pSbAddr; }
    if (pWidth)     
    { 
        p->width = *pWidth; 
        if (key)
        {
            memset(p->key, 0, sizeof(p->key));
            if (keySize >= BYTE_COUNT(144))
            {
                memcpy(p->key, key, BYTE_COUNT(144));
            }
            else
            {
                memcpy(p->key,  key, keySize); 
            }
        }
    }

    if (pReady1)    { p->ready1 = *pReady1; }
    if (pMatch1)    { p->match1 = *pMatch1; }
    if (prior1)     
    {
        if (prior1Size >= IO_PRIORITY_BYTE_COUNT)
        {
            memcpy(p->prior1, prior1, IO_PRIORITY_BYTE_COUNT);
        }
        else
        {
            memset(p->prior1, 0, sizeof(p->prior1));
            memcpy(p->prior1,  prior1,  prior1Size);
        }
    }
	if (psec)    { p->sec = *psec; }
    

	if(addata)
		memcpy(p->addata,addata,adsize);
	if(adwidth)
		memcpy(p->adwidth,adwidth,adwidthsize);

done:
    return rc;
}

TcError IoCmd_Set_DBG_LC_Input(IoCmd* cmd, const u8* pProfile, const u16* pSbAddr, const u8* pWidth, const void* key, int keySize, 
                                    const u8* pReady1, const u8* pMatch1, const void* prior1, int prior1Size, 
                                    const u8* pReady2, const u8* pMatch2, const void* prior2, int prior2Size, 
                                    const u8* pIsOnline )
{
    TcError rc = TcE_OK;
    DataDbgLCInput* p = &cmd->u.lcInput;
    int keyWriteSize;
  
    assertLog(cmd != NULL);

    if ((p->opCode == IO_OPCODE_INVALID)                      // uninitialize cmd
        ||                                                                           
        ((pProfile) && (pSbAddr) && (pWidth) && (key) &&      // all parameters given
         (pReady1)  && (pMatch1) && (prior1) &&
         (pReady2)  && (pMatch2) && (prior2) &&
         (pIsOnline)))
    {
        p->opCode = IO_DBG_LC_INPUT;
    }
    else if (p->opCode != IO_DBG_LC_INPUT)
    {
        printError(commonLibLog, "opCode mismatch: current:  %X (%s) change to %X (%s)\n", p->opCode, IoCmd_Opcode_Name(p->opCode), IO_DBG_LC_INPUT, IoCmd_Opcode_Name(IO_DBG_LC_INPUT));
        return TcE_Logic_Error;
    }

    if (pProfile)   { p->profile = *pProfile; }
    if (pSbAddr)    { p->sbAddr = *pSbAddr; }
    if (pWidth)     
    { 
        p->width = *pWidth; 
        if (key)
        {
            keyWriteSize = (p->width+1)*IO_KEY_MIN_SIZE_BYTE_COUNT;
            if (keySize >= keyWriteSize)
            {
                memcpy(p->key,  key,  keyWriteSize);
            }
            else
            {
                memset(p->key, 0, sizeof(p->key));
                memcpy(p->key,  key,  keySize); 
            }
        }
    }

    if (pReady1)    { p->ready1 = *pReady1; }
    if (pMatch1)    { p->match1 = *pMatch1; }
    if (prior1)     
    {
        if (prior1Size >= IO_PRIORITY_BYTE_COUNT)
        {
            memcpy(p->prior1, prior1, IO_PRIORITY_BYTE_COUNT);
        }
        else
        {
            memset(p->prior1, 0, sizeof(p->prior1));
            memcpy(p->prior1,  prior1,  prior1Size);
        }
    }

    if (pReady2)    { p->ready2 = *pReady2; }
    if (pMatch2)    { p->match2 = *pMatch2; }
    if (prior2)     
    { 
        if (prior2Size >= IO_PRIORITY_BYTE_COUNT)
        {
            memcpy(p->prior2, prior2, IO_PRIORITY_BYTE_COUNT);
        }
        else
        {
            memset(p->prior2, 0, sizeof(p->prior2));
            memcpy(p->prior2,  prior2,  prior2Size);
        }
    }
    if (pIsOnline)  { p->isOnline = *pIsOnline; }

done:
    return rc;
}

TcError IoCmd_Set_DBG_LC_Output(IoCmd* cmd, const u8* pProfile, const u16* pSbAddr, const u8* pWidth, const void* key, int keySize, 
                                    const u8* pReady, const u8* pMatch, const void* prior, int priorSize, 
                                    const u8* pSec, const void* pAd, int adSize, const void* pAdWidth, int adWidthSize)
{
    TcError rc = TcE_OK;
    DataDbgLCOutput* p = &cmd->u.lcOutput;
    int keyWriteSize;
  
    assertLog(cmd != NULL);

    if ((p->opCode == IO_OPCODE_INVALID)                      // uninitialize cmd
        ||                                                                           
        ((pProfile) && (pSbAddr) && (pWidth) && (key) &&      // all parameters given
         (pReady)   && (pMatch)  && (prior)  &&
         (pSec)     && (pAd)     && (pAdWidth)))
    {
        p->opCode = IO_DBG_LC_OUTPUT;
    }
    else if (p->opCode != IO_DBG_LC_OUTPUT)
    {
        printError(commonLibLog, "opCode mismatch: current:  %X (%s) change to %X (%s)\n", p->opCode, IoCmd_Opcode_Name(p->opCode), IO_DBG_LC_OUTPUT, IoCmd_Opcode_Name(IO_DBG_LC_OUTPUT));
        return TcE_Logic_Error;
    }

    if (pProfile)   { p->profile = *pProfile; }
    if (pSbAddr)    { p->sbAddr = *pSbAddr; }
    if (pWidth)     
    { 
        p->width = *pWidth; 
        if (key)        
        { 
            keyWriteSize = (p->width + 1) * IO_KEY_MIN_SIZE_BYTE_COUNT;
            if (keySize >= keyWriteSize)
            {
                memcpy(p->key,  key,  keyWriteSize);
            }
            else
            {
                memset(p->key, 0, sizeof(p->key));
                memcpy(p->key,  key,  keySize); 
            }
        }
    }
    if (pReady)     { p->ready = *pReady; }
    if (pMatch)     { p->match = *pMatch; }
    if (prior)      
    { 
        if (priorSize >= IO_PRIORITY_BYTE_COUNT)
        {
            memcpy(p->prior, prior, IO_PRIORITY_BYTE_COUNT);
        }
        else
        {
            memset(p->prior, 0, sizeof(p->prior));
            memcpy(p->prior,  prior,  priorSize);
        }
    }
    if (pSec)       { p->sec = *pSec; }
    if (pAd)
    { 
        if (adSize >= IO_AD_BYTE_COUNT)
        {
            memcpy(p->ad, pAd,  IO_AD_BYTE_COUNT); 
        }
        else
        {
            memset(p->ad, 0, sizeof(p->ad));
            memcpy(p->ad, pAd,  adSize); 
        }
    }
    if (pAdWidth)
    { 
        if (adWidthSize >= IO_ADWIDTH_BYTE_COUNT)
        {
            memcpy(p->adwidth, pAdWidth,  IO_ADWIDTH_BYTE_COUNT); 
        }
        else
        {
            memset(p->adwidth, 0, sizeof(p->adwidth));
            memcpy(p->adwidth, pAdWidth,  adWidthSize); 
        }
    }

done:
    return rc;
}

TcError IoCmd_Set_DBG_NFE(IoCmd* cmd, const u8* pDev, const u8* pOctopus, const u8* pModule, const u8* pMemType, const u8* pType, const void* xorMask, int xorMaskSize)
{
    TcError rc = TcE_OK;
    DataDbgNFE* p = &cmd->u.nfe;

    assertLog(cmd != NULL);

    if ((p->opCode == IO_OPCODE_INVALID)                         // uninitialize cmd
        ||                                                                           
        ((pDev) && (pOctopus) && (pModule) && (pMemType) &&      // all parameters given
         (xorMask)))
    {
        p->opCode = IO_DBG_NFE;
    }
    else if (p->opCode != IO_DBG_NFE)
    {
        printError(commonLibLog, "opCode mismatch: current: %X (%s) change to %X (%s)\n", p->opCode, IoCmd_Opcode_Name(p->opCode), IO_DBG_NFE, IoCmd_Opcode_Name(IO_DBG_NFE));
        return TcE_Logic_Error;
    }

    if (pDev)      { p->dev = *pDev; }
    if (pOctopus)  { p->octopus = *pOctopus; }
    if (pModule)   { p->module = *pModule; }
    if (pMemType)  { p->memType = *pMemType; }
    if (pType)     { p->type = *pType; }

    if (xorMask)
    {
        if (xorMaskSize < IO_NFE_XOR_MASK_BYTE_COUNT)
        {
            memset(p->xorMask, 0, sizeof(p->xorMask));
            memcpy(p->xorMask,  xorMask, xorMaskSize); 
        }
        else
        {
            memcpy(p->xorMask, xorMask,  IO_NFE_XOR_MASK_BYTE_COUNT);
        }
    }
done:
    return rc;
}

TcError IoCmd_Set_DBG_LD3(IoCmd* cmd, const u8* pDev, const u32* pAddress, const void* data, int dataSize )
{
    TcError rc = TcE_OK;
    DataDbgLD3* p = &cmd->u.dbgLd3;
  
    assertLog(cmd != NULL);

    if ((p->opCode == IO_OPCODE_INVALID)        // uninitialize cmd
        ||                                                                           
        ((pDev) && (pAddress) && (data)))       // all parameters given
    {
        p->opCode = IO_DBG_LD3;
    }
    else if (p->opCode != IO_DBG_LD3)
    {
        printError(commonLibLog, "opCode mismatch: current: %X (%s) change to %X (%s)\n", p->opCode, IoCmd_Opcode_Name(p->opCode), IO_OPCODE_LD3, IoCmd_Opcode_Name(IO_OPCODE_LD3));
        return TcE_Logic_Error;
    }

    if (pDev)      { p->dev = *pDev; }
    if (pAddress)  { p->address = *pAddress; }

    if (data)
    {
        if (dataSize < IO_DATA_SIZE_BYTE_COUNT)
        {
            memset(p->data, 0, sizeof(p->data));
            memcpy(p->data, data, dataSize);
        }
        else
        {
            memcpy(p->data, data, IO_DATA_SIZE_BYTE_COUNT);
        }
    }
done:
    return rc;
}

TcError IoCmd_Set_MDIO(IoCmd* cmd, const u8* operation, const u8* pMpid, const u8* pDevAddr, const u16* pRegAddr, const u16* pRegData)
{
    TcError rc = TcE_OK;
    DataMdio* p = &cmd->u.mdio;
  
    assertLog(cmd != NULL);

    if ((p->opCode == IO_OPCODE_INVALID)        // uninitialize cmd
        ||                                                                           
        ((operation) && (pMpid) && (pDevAddr) && (pRegAddr) && (pRegData)))       // all parameters given
    {
        p->opCode = IO_MDIO;
    }
    else if (p->opCode != IO_MDIO)
    {
        printError(commonLibLog, "opCode mismatch: current: %X (%s) change to %X (%s)\n", p->opCode, IoCmd_Opcode_Name(p->opCode), IO_MDIO, IoCmd_Opcode_Name(IO_MDIO));
        return TcE_Logic_Error;
    }

    if (operation)  { p->operation = *operation; }
    if (pMpid)      { p->mpid = *pMpid; }
    if (pDevAddr)   { p->devAddr = *pDevAddr; }
    if (pRegAddr)   { p->regAddr = *pRegAddr; }
    if (pRegData)   { p->regData = *pRegData; }

done:
    return rc;
}

TcError IoCmd_Set_Cmt_NseLib(IoCmd* cmd, const u8* pData)
{
    TcError rc = TcE_OK;
    DataCmtNseLib* p = &cmd->u.cmtNseLib;

    assertLog(cmd != NULL);
    assertLog(pData != NULL);

    p->opCode = IO_CMT_NSELIB;
    p->data = *pData;

done:
    return rc;
}

TcError IoCmd_Set_Cmt_OnlineRecord(IoCmd* cmd, const u8* pOpCode, const char* tableName, const u32* pPriority)
{
    TcError rc = TcE_OK;
    DataCmtOnlineRecord* p = &cmd->u.onlineRecord;

    assertLog(cmd != NULL);
    assertLog(pOpCode != NULL);

    p->opCode = *pOpCode;

    if (tableName)   
    { 
        strncpy(p->tableName, tableName, IO_TABLE_NAME_BYTE_COUNT); 
        p->tableName[IO_TABLE_NAME_BYTE_COUNT-1] = 0;
    }
    if (pPriority) {  p->prior = *pPriority;  }

done:
    return rc;
}

TcError IoCmd_Set_Cmt_OnlineSplitAddRecord(IoCmd* cmd, const char* tableName, const u32* pPriority)
{
    u8 opCode = IO_CMT_ONLINE_SPLIT_ADD_REC;
    return IoCmd_Set_Cmt_OnlineRecord(cmd, &opCode, tableName, pPriority);
}

TcError IoCmd_Set_Cmt_OnlineDeleteRecord(IoCmd* cmd, const char* tableName, const u32* pPriority)
{
    u8 opCode = IO_CMT_ONLINE_DEL_REC;
    return IoCmd_Set_Cmt_OnlineRecord(cmd, &opCode, tableName, pPriority);
}

TcError IoCmd_Set_Cmt_OnlineAddRecord(IoCmd* cmd, const char* tableName, const u32* pPriority)
{
    u8 opCode = IO_CMT_ONLINE_ADD_REC;
    return IoCmd_Set_Cmt_OnlineRecord(cmd, &opCode, tableName, pPriority);
}

TcError IoCmd_Set_Cmt_OnlineTable(IoCmd* cmd, const u8* pOpCode, const char* tableName, const u32* pId)
{
    TcError rc = TcE_OK;
    DataCmtOnlineTable* p = &cmd->u.onlineTable; 

    assertLog(cmd != NULL);
    assertLog(pOpCode != NULL);
    assertLog(tableName != NULL);
    assertLog(pId != NULL);

    p->opCode = *pOpCode;

    strncpy(p->tableName, tableName, IO_TABLE_NAME_BYTE_COUNT);
    p->tableName[IO_TABLE_NAME_BYTE_COUNT-1] = 0;

    p->id = *pId;

done:
    return rc;
}

TcError IoCmd_Set_Cmt_OnlineDeleteTable(IoCmd* cmd, const char* tableName, const u32* pId)
{
    u8 opCode = IO_CMT_ONLINE_DEL_TABLE;
    return IoCmd_Set_Cmt_OnlineTable(cmd, &opCode, tableName, pId);
}

TcError IoCmd_Set_Cmt_OnlineAddTable(IoCmd* cmd, const char* tableName, const u32* pId)
{
    u8 opCode = IO_CMT_ONLINE_ADD_TABLE;
    return IoCmd_Set_Cmt_OnlineTable(cmd, &opCode, tableName, pId);
}

TcError IoCmd_Set_Cmt_String(IoCmd* cmd, const char* fmt, ...)
{
    DataCmtString* p = &cmd->u.cmtString;
    int len;
    va_list ap;

    va_start(ap, fmt);
   
    p->opCode = IO_CMT_STRING;
    len = vsnprintf(p->str, sizeof(p->str), fmt, ap);
    p->len = len+1;
    return TcE_OK;
}

u8 IoCmd_isInitialized(const IoCmd* cmd)
{
    TcError rc = TcE_OK;
    assertLog(cmd != NULL);
    rc = (cmd->u.opCode != IO_OPCODE_INVALID);
done:
    return rc;
}

TcError IoCmd_Get_OpCode(const IoCmd* cmd, u8* pOpCode)
{
    TcError rc = TcE_OK;
    assertLog(cmd != NULL);
    *pOpCode = cmd->u.opCode;
done:
    return rc;
}

static TcError IoCmd_ValidateOpcode(const IoCmd* cmd, u8 opcode)
{
    if (cmd->u.opCode == IO_OPCODE_INVALID)
    {
        printError(commonLibLog, "cmd : uninitialized\n");
        return TcE_Invalid_Argument;
    }
    else if (cmd->u.opCode != opcode)
    {
        printError(commonLibLog, "opcode mismatch: %X (%s) expect: %X (%s)\n",
                      cmd->u.opCode, IoCmd_Opcode_Name(cmd->u.opCode), opcode, IoCmd_Opcode_Name(opcode));
        return TcE_Invalid_Argument;
    }
    return TcE_OK;
}

TcError IoCmd_Get_Nop(const IoCmd* cmd, u16* pCnt)
{
    TcError rc = TcE_OK;

    assertLog(cmd != NULL);
    assertLog(pCnt != NULL);

    CHECK_ERROR(commonLibLog, IoCmd_ValidateOpcode(cmd, IO_OPCODE_NOP));

    *pCnt = cmd->u.nop.delay;

done:
    return rc;
}

TcError IoCmd_Get_LD5_LC(const IoCmd* cmd, u8* pProfile, u16* pSbAddr, u8* pWidth, const void** ppKey, int* pKeySize)
{
    const DataOpCode_LD5_LC* p = &cmd->u.ld5_lc;

    if (pProfile)  { *pProfile = p->profile; }
    if (pSbAddr)   { *pSbAddr = p->sbAddr; }
    if (pWidth)    { *pWidth = p->width; }
    if (ppKey)     { *ppKey = p->key; }
    if (pKeySize)  { *pKeySize = (p->width + 1)*IO_KEY_MIN_SIZE_BYTE_COUNT; }

    return TcE_OK;
}

TcError IoCmd_Get_LC(const IoCmd* cmd, u8* pProfile, u16* pSbAddr, u8* pWidth, const void** ppKey, int* pKeySize)
{
    TcError rc = TcE_OK;

    assertLog(cmd != NULL);

    CHECK_ERROR(commonLibLog, IoCmd_ValidateOpcode(cmd, IO_OPCODE_LC));
    CHECK_ERROR(commonLibLog, IoCmd_Get_LD5_LC(cmd, pProfile, pSbAddr, pWidth, ppKey, pKeySize));

done:
    return rc;
}

TcError IoCmd_Get_LD2(const IoCmd* cmd, u8* pDev, const void** ppData, int* pDataSize)
{
    TcError rc = TcE_OK;
    const DataOpCodeLD2* p = &cmd->u.ld2;

    assertLog(cmd != NULL);
    CHECK_ERROR(commonLibLog, IoCmd_ValidateOpcode(cmd, IO_OPCODE_LD2));

    if (pDev)       { *pDev = p->dev; }
    if (ppData)     { *ppData = p->data; }
    if (pDataSize)  { *pDataSize = sizeof(p->data); }

done:
    return rc;
}

TcError IoCmd_Get_LD3(const IoCmd* cmd, u8* pDev, u32* pAddress)
{
    TcError rc = TcE_OK;
    const DataOpCodeLD3* p = &cmd->u.ld3;

    assertLog(cmd != NULL);
    CHECK_ERROR(commonLibLog, IoCmd_ValidateOpcode(cmd, IO_OPCODE_LD3));

    if (pDev)     { *pDev     = p->dev; }
    if (pAddress) { *pAddress = p->address; }

done:
    return rc;
}

TcError IoCmd_Get_LD5(const IoCmd* cmd, u8* pProfile, u16* pSbAddr, u8* pWidth, const void** ppKey, int* pKeySize)
{
    TcError rc = TcE_OK;

    assertLog(cmd != NULL);
    CHECK_ERROR(commonLibLog, IoCmd_ValidateOpcode(cmd, IO_OPCODE_LD5));
    CHECK_ERROR(commonLibLog, IoCmd_Get_LD5_LC(cmd, pProfile, pSbAddr, pWidth, ppKey, pKeySize));

done:
    return rc;
}

TcError IoCmd_Get_DBG_LC_LpmAD(const IoCmd* cmd, u8* pProfile, u16* pSbAddr, u8* pWidth, const void** ppKey, int* pKeySize, 
                                    u8* pReady1, u8* pMatch1, const  void** ppPrior1, int* pPrior1Size, 
                                    u8* psec, const void** ppaddata, int* padsz , const void** ppadwidth, int* padwdsz)
{
    TcError rc = TcE_OK;
	const DataDbgLCLpmAD* p = &cmd->u.lclpmAD;

    assertLog(cmd != NULL);
    CHECK_ERROR(commonLibLog, IoCmd_ValidateOpcode(cmd, IO_DBG_LC_LPMAD));

    if (pProfile)       { *pProfile = p->profile; }
    if (pSbAddr)        { *pSbAddr = p->sbAddr; }
    if (pWidth)         { *pWidth = p->width; }
    if (ppKey)          { *ppKey = p->key; }
    if (pKeySize)       { *pKeySize = 18; }
    if (pReady1)        { *pReady1 = p->ready1; }
    if (pMatch1)        { *pMatch1 = p->match1; }
    if (ppPrior1)       { *ppPrior1 = p->prior1;  }
    if (pPrior1Size)    { *pPrior1Size = sizeof(p->prior1); }
	if (psec)           { *psec = p->sec; }
	if (ppaddata)       { *ppaddata = p->addata; }
	if (ppadwidth)      { *ppadwidth = p->adwidth; }
	if (padsz)		    { *padsz = sizeof(p->addata); }
	if (padwdsz)        { *padwdsz = sizeof(p->adwidth); }

done:
    return rc;
}

TcError IoCmd_Get_DBG_LC_Input(const IoCmd* cmd, u8* pProfile, u16* pSbAddr, u8* pWidth, const void** ppKey, int* pKeySize, 
                                    u8* pReady1, u8* pMatch1, const  void** ppPrior1, int* pPrior1Size, 
                                    u8* pReady2, u8* pMatch2, const void** ppPrior2, int* pPrior2Size, 
                                    u8* pIsOnline)
{
    TcError rc = TcE_OK;
    const DataDbgLCInput* p = &cmd->u.lcInput;

    assertLog(cmd != NULL);
    CHECK_ERROR(commonLibLog, IoCmd_ValidateOpcode(cmd, IO_DBG_LC_INPUT));

    if (pProfile)       { *pProfile = p->profile; }
    if (pSbAddr)        { *pSbAddr = p->sbAddr; }
    if (pWidth)         { *pWidth = p->width; }
    if (ppKey)          { *ppKey = p->key; }
    if (pKeySize)       { *pKeySize = (p->width+1)*IO_KEY_MIN_SIZE_BYTE_COUNT; }
    if (pReady1)        { *pReady1 = p->ready1; }
    if (pMatch1)        { *pMatch1 = p->match1; }
    if (ppPrior1)       { *ppPrior1 = p->prior1;  }
    if (pPrior1Size)    { *pPrior1Size = sizeof(p->prior1); }
    if (pReady2)        { *pReady2 = p->ready2; }
    if (pMatch2)        { *pMatch2 = p->match2; }
    if (ppPrior2)       { *ppPrior2 = p->prior2; }
    if (pPrior2Size)    { *pPrior2Size = sizeof(p->prior2); }
    if (pIsOnline)      { *pIsOnline = p->isOnline; }

done:
    return rc;
}

TcError IoCmd_Get_DBG_LC_Output(const IoCmd* cmd, u8* pProfile, u16* pSbAddr, u8* pWidth, const void** ppKey, int* pKeySize, 
                                    u8* pReady, u8* pMatch, const void** ppPrior, int* pPriorSize, 
                                    u8* pSec, const void** pAd, int* pAdSize, const void** pAdWidth, int* pAdWidthSize)
{
    TcError rc = TcE_OK;
    const DataDbgLCOutput* p = &cmd->u.lcOutput;

    assertLog(cmd != NULL);
    CHECK_ERROR(commonLibLog, IoCmd_ValidateOpcode(cmd, IO_DBG_LC_OUTPUT));

    if (pProfile)       { *pProfile = p->profile; }
    if (pSbAddr)        { *pSbAddr = p->sbAddr; }
    if (pWidth)         { *pWidth = p->width; }
    if (ppKey)          { *ppKey = p->key; }
    if (pKeySize)       { *pKeySize = (p->width+1)*IO_KEY_MIN_SIZE_BYTE_COUNT; }
    if (pReady)         { *pReady = p->ready; }
    if (pMatch)         { *pMatch = p->match; }
    if (ppPrior)        { *ppPrior = p->prior;  }
    if (pPriorSize)     { *pPriorSize = sizeof(p->prior); }
    if (pSec)           { *pSec = p->sec; }
    if (pAd)            { *pAd = p->ad; };
    if (pAdSize)        { *pAdSize = sizeof(p->ad); }
    if (pAdWidth)       { *pAdWidth = p->adwidth; }
    if (pAdWidthSize)   { *pAdWidthSize = sizeof(p->adwidth); }

done:
    return rc;
}

TcError IoCmd_Get_DBG_NFE(const IoCmd* cmd, u8* pDev, u8* pOctopus, u8* pModule, u8* pMemType, u8* pType, const void** ppXorMask, int* pXorMaskSize)
{
    TcError rc = TcE_OK;
    const DataDbgNFE* p = &cmd->u.nfe;

    assertLog(cmd != NULL);
    CHECK_ERROR(commonLibLog, IoCmd_ValidateOpcode(cmd, IO_DBG_NFE));

    if (pDev)           { *pDev = p->dev; }
    if (pOctopus)       { *pOctopus = p->octopus; }
    if (pModule)        { *pModule = p->module; }
    if (pMemType)       { *pMemType = p->memType; }
    if (pType)          { *pType = p->type; }
    if (ppXorMask)      { *ppXorMask = p->xorMask; }
    if (pXorMaskSize)   { *pXorMaskSize = sizeof(p->xorMask); }

done:
    return rc;
}

TcError IoCmd_Get_DBG_LD3(const IoCmd* cmd, u8* pDev, u32* pAddress, const void** ppData, int* pDataSize )
{
    TcError rc = TcE_OK;
    const DataDbgLD3 * p = &cmd->u.dbgLd3;

    assertLog(cmd != NULL);
    CHECK_ERROR(commonLibLog, IoCmd_ValidateOpcode(cmd, IO_DBG_LD3));

    if (pDev)       { *pDev  = p->dev; }
    if (pAddress)   { *pAddress = p->address; }
    if (ppData)     { *ppData = p->data; }
    if (pDataSize)  { *pDataSize = sizeof(p->data); }

done:
    return rc;
}

TcError IoCmd_Get_MDIO(const IoCmd* cmd, u8* operation, u8* pMpid, u8* pDevAddr, u16* pRegAddr, u16* pRegData)
{
    TcError rc = TcE_OK;
    const DataMdio * p = &cmd->u.mdio;

    assertLog(cmd != NULL);
    CHECK_ERROR(commonLibLog, IoCmd_ValidateOpcode(cmd, IO_MDIO));

    if (operation)       { *operation = p->operation; }
    if (pMpid)           { *pMpid = p->mpid; }
    if (pDevAddr)        { *pDevAddr = p->devAddr; }
    if (pRegAddr)        { *pRegAddr = p->regAddr; }
    if (pRegData)        { *pRegData = p->regData; }

done:
    return rc;
}

TcError IoCmd_Get_Cmt_NseLib(const IoCmd* cmd, u8* pData)
{
    TcError rc = TcE_OK;
    const DataCmtNseLib* p = &cmd->u.cmtNseLib;

    assertLog(cmd != NULL);
    assertLog(pData != NULL);

    *pData = p->data;

done:
    return rc;
}

TcError IoCmd_Get_Cmt_OnlineRecord(const IoCmd* cmd, const char** ppTableName, u32* pPriority)
{
    TcError rc = TcE_OK;
    const DataCmtOnlineRecord* p = &cmd->u.onlineRecord;
    assertLog(cmd != NULL);

    if (ppTableName)    { *ppTableName = p->tableName; }
    if (pPriority)      { *pPriority = p->prior; }

done:
    return rc;
}

TcError IoCmd_Get_Cmt_OnlineSplitAddRecord(const IoCmd* cmd, const char** ppTableName, u32* pPriority)
{
    return IoCmd_Get_Cmt_OnlineRecord(cmd, ppTableName, pPriority);
}

TcError IoCmd_Get_Cmt_OnlineAddRecord(const IoCmd* cmd, const char** ppTableName, u32* pPriority)
{
    return IoCmd_Get_Cmt_OnlineRecord(cmd, ppTableName, pPriority);
}

TcError IoCmd_Get_Cmt_OnlineDeleteRecord(const IoCmd* cmd, const char** ppTableName, u32* pPriority)
{
    return IoCmd_Get_Cmt_OnlineRecord(cmd, ppTableName, pPriority);
}

TcError IoCmd_Get_Cmt_OnlineTable(const IoCmd* cmd, const char** ppTableName, u32* pId)
{
    TcError rc = TcE_OK;
    const DataCmtOnlineTable* p = &cmd->u.onlineTable;
    assertLog(cmd != NULL);

    if (ppTableName)        { *ppTableName = p->tableName; }
    if (pId)                { *pId = p->id; }

done:
    return rc;
}

TcError IoCmd_Get_Cmt_OnlineDeleteTable(const IoCmd* cmd, const char** tableName, u32* pId)
{
    TcError rc = TcE_OK;
    assertLog(cmd != NULL);
    rc = IoCmd_Get_Cmt_OnlineTable(cmd, tableName, pId);
done:
    return rc;
}

TcError IoCmd_Get_Cmt_OnlineAddTable(const IoCmd* cmd, const char** tableName, u32* pId)
{
    TcError rc = TcE_OK;
    assertLog(cmd != NULL);
    rc = IoCmd_Get_Cmt_OnlineTable(cmd, tableName, pId);
done:
    return rc;
}

TcError IoCmd_Get_Cmt_String(const IoCmd* cmd, const char** str, u8* pLen)
{
    DataCmtString* p = &cmd->u.cmtString;

    *str = p->str;
    if (pLen)   *pLen = p->len;

    return TcE_OK;
}

void IoCmd_Copy(IoCmd* dst, const IoCmd* src)
{
    memcpy(dst, src, sizeof(*src));
}


const char* IoCmd_Opcode_Name(u8 opcode)
{
    if (opcode == IO_OPCODE_NOP)                    return "NOP";
    else if (opcode == IO_OPCODE_LC)                return "LC";
    else if (opcode == IO_OPCODE_LD2)               return "LD2";
    else if (opcode == IO_OPCODE_LD3)               return "LD3";
    else if (opcode == IO_OPCODE_LD5)               return "LD5";
    else if (opcode == IO_OPCODE_QUIET)             return "QUIET";
    else if (opcode == IO_DBG_NFE)                  return "Dbg NFE";
    else if (opcode == IO_DBG_LC_INPUT)             return "Dbg LC Input";
    else if (opcode == IO_DBG_LC_OUTPUT)            return "Dbg LC Output";
    else if (opcode == IO_DBG_LD3)                  return "Dbg LD3";
    else if (opcode == IO_CMT_NSELIB)               return "Comment NseLib";
    else if (opcode == IO_CMT_ONLINE_SPLIT_ADD_REC) return "Comment Online Split Add Record";
    else if (opcode == IO_CMT_ONLINE_DEL_REC)       return "Comment Online Delete Record";     
    else if (opcode == IO_CMT_ONLINE_ADD_REC)       return "Comment Online Addd Record";
    else if (opcode == IO_CMT_ONLINE_DEL_TABLE)     return "Comment Online Delete Table";
    else if (opcode == IO_CMT_ONLINE_ADD_TABLE)     return "Comment Online Add Table";
    else if (opcode == IO_OPCODE_INVALID)           return "Invalid opcode";
    else                                            return "Unknown opcode";
}

int IoCmd_Size()
{
    return sizeof(IoCmd);
}

const char* IoCmd_Name(const IoCmd* cmd)
{
    u8 opcode;

    IoCmd_Get_OpCode(cmd, &opcode);

    return IoCmd_Opcode_Name(opcode);
}
