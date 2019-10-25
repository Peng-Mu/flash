#ifndef __Io_h__
#define __Io_h__

#include "Platform.h"
#include "TcError.h"
#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sIo Io;

API TcError Io_Input_Open(const char* fn, Io** pIo);
API TcError Io_Output_Open(const char* fn, Io** pIo);
API void Io_Close(Io* io);
API TcError Io_Input_Reset(Io* io);

// Write direct to file
API TcError Io_Write_Nop(Io* io, u16 cnt);
API TcError Io_Write_Quiet(Io* io);
API TcError Io_Write_LC(Io* io, u8 profile, u16 sbAddr, u8 width, const void* key, int keySize);
API TcError Io_Write_LD2(Io* io, u8 dev, const void* data, int dataSize);
API TcError Io_Write_LD3(Io* io, u8 dev, u32 address);
API TcError Io_Write_LD5(Io* io, u8 profile, u16 sbAddr, u8 width, const void* key, int keySize);
API TcError Io_Write_DBG_LC_Input(Io* io, u8 profile, u16 sbAddr, u8 width, const void* key, int keySize, 
                                    u8 ready1, u8 match1, const void* prior1, int prior1Size, 
                                    u8 ready2, u8 match2, const void* prior2, int prior2Size, 
                                    u8 isOnline);
API TcError Io_Write_DBG_LC_Output(Io* io, u8 profile, u16 sbAddr, u8 width, const void* key, int keySize, 
                                u8 ready, u8 match, const void* prior, int priorSize, 
                                u8 sec, const void* ad, int adSize, const void* adWidth, int adWidthSize);
API TcError Io_Write_DBG_NFE(Io* io, u8 dev, u8 octopus, u8 module, u8 memType, u8 type, const void* xorMask, int xorMaskSize);
API TcError Io_Write_DBG_LD3(Io* io, u8 dev, u32 address, const void* data, int dataSize );
API TcError Io_Write_MDIO(Io* io, u8 operation, u8 mpid, u8 devAddr, u16 regAddr, u16 regData);
API TcError Io_Write_Cmt_NseLib(Io* io, u8 cmData);
API TcError Io_Write_Cmt_OnlineSplitAddRecord(Io* io, const char* tableName, u32 priority);
API TcError Io_Write_Cmt_OnlineDeleteRecord(Io* io, const char* tableName, u32 priority);
API TcError Io_Write_Cmt_OnlineAddRecord(Io* io, const char* tableName, u32 priority);
API TcError Io_Write_Cmt_OnlineDeleteTable(Io* io, const char* tableName, u32 id);
API TcError Io_Write_Cmt_OnlineAddTable(Io* io, const char* tableName, u32 id);
API TcError Io_Write_Cmt_String(Io* io, const char* str, u8 len);

// Read direct from file
API TcError Io_Read_OpCode(Io* io, u8* pOpCode);
API TcError Io_Read_Nop(Io* io, u16* pCnt);
API TcError Io_Read_Quiet(Io* io);
API TcError Io_Read_LC(Io* io, u8* pProfile, u16* pSbAddr, u8* pWidth, void* key, int keySize);
API TcError Io_Read_LD2(Io* io, u8* pDev, void* data, int dataSize);
API TcError Io_Read_LD3(Io* io, u8* pDev, u32* pAddress);
API TcError Io_Read_LD5(Io* io, u8* pProfile, u16* pSbAddr, u8* pWidth, void* key, int keySize);
API TcError Io_Read_DBG_LC_Input(Io* io, u8* pProfile, u16* pSbAddr, u8* pWidth, void* key, int keySize, 
                                u8* pReady1, u8* pMatch1, void* prior1, int prior1Size, 
                                u8* pReady2, u8* pMatch2, void* prior2, int prior2Size, 
                                u8* pIsOnline);
API TcError Io_Read_DBG_LC_Output(Io* io, u8* pProfile, u16* pSbAddr, u8* pWidth, void* key, int keySize, 
                                u8* pReady, u8* pMatch, void* prior, int priorSize, 
                                u8* pSec, void* ad, int adSize, void* adWidth, int adWidthSize);
API TcError Io_Read_DBG_NFE(Io* io, u8* pDev, u8* pOctopus, u8* pModule, u8* pMemType, u8* pType, void* xorMask, int xorMaskSize);
API TcError Io_Read_DBG_LD3(Io* io, u8* pDev, u32* pAddress, void* data, int dataSize);
API TcError Io_Read_MDIO(Io* io, u8* operation, u8* mpid, u8* devAddr, u16* regAddr, u16* regData);
API TcError Io_Read_Cmt_NseLib(Io* io, u8* pData);
API TcError Io_Read_Cmt_OnlineSplitAddRecord(Io* io, char* tableName, int tableNameSize, u32* pPriority);
API TcError Io_Read_Cmt_OnlineDeleteRecord(Io* io, char* tableName, int tableNameSize, u32* pPriority);
API TcError Io_Read_Cmt_OnlineAddRecord(Io* io, char* tableName, int tableNameSize, u32* pPriority);
API TcError Io_Read_Cmt_OnlineDeleteTable(Io* io, char* tableName, int tableNameSize, u32* pId);
API TcError Io_Read_Cmt_OnlineAddTable(Io* io, char* tableName, int tableNameSize, u32* pId);
API TcError Io_Read_Cmt_String(Io* io, char* str, u8* pLen);

struct sIoCmd;

API TcError Io_IoCmd_Write(Io* io, const struct sIoCmd* cmd);

API TcError Io_IoCmd_Read(Io* io, struct sIoCmd* cmd);

#ifdef __cplusplus
}
#endif

#endif /* __Io_h__ */

