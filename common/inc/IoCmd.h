#ifndef __IoCmd_h__
#define __IoCmd_h__

#include "Platform.h"
#include "TcError.h"
#include "utils.h"
#include "defines.h"

/*
 *                              Bytes:
 *                              +------+------+------+------+------+------+------+------+------+------+------+------+------------------------------------------------------+------+------+------+------+ 
 *                              |  0   |   1  |   2  |   3  |   4  |   5  |   6  |   7  |   8  |   9  |  10  |  11  | .................................................... |  21  |  22  |  23  |  24  |
 *                              +------+------+------+------+------+------+------+------+------+------+------+------+------------------------------------------------------+------+------+------+------+ 
 *  MAGIC NUMBER                | 0xABCDDCBA                |                                                                                                                                           
 *                              +---------------------------+
 *  VERSION                     | 0x00000000                |                                                                                                                                           
 *                              +---------------------------+
 *                              ...
 *                              +---------------------------+------+-------------+
 *  NOP                         |   seq#                    | 0    |    cnt      |                                                                                                                             
 *                              +---------------------------+------+-------------+
 *                              ...
 *                              +---------------------------+------+------+------+------+------------------------------------+
 *  LC                          |   seq#                    | 1    | pr   | sb   |  w   | key ( (w+1) * (80-bit, 10-byte) )  |
 *                              +---------------------------+------+------+------+------+------------------------------------+
 *                              ... 
 *                              +---------------------------+------+------+----------------------------+ 
 *  LD2                         |   seq#                    | 2    | dev  | data (160-bit, 20-byte)    |                            
 *                              +---------------------------+------+------+----------------------------+ 
 *                              ...
 *                              +---------------------------+------+------+------+------+------+
 *  LD3                         |   seq#                    | 3    | dev  | address (4-byte)   |
 *                              +---------------------------+------+------+------+------+------+
 *                              ...
 *                              +---------------------------+------+------+------+------+------------------------------------+
 *  LD5                         |   seq#                    | 5    | pr   | sb   | w    | key ( (w+1) * (80-bit, 10-byte) )  |
 *                              +---------------------------+------+------+------+------+------------------------------------+
 *                              ...
 *                              +---------------------------+------+------+------+------+------+-------------------------------+------+
 *  DBG_NFE                     |   seq#                    | 7    | dev  |oc/qu | m    | memT | xorMaskHex (160-bit, 20-byte) | type |
 *                              +---------------------------+------+------+------+------+------+-------------------------------+------+
 *                              ... 
 *                              +---------------------------+------+------+------+------+------------------------------------+-----+-----+--------------------------+-----+-----+--------------------------+-----+
 *  DBG_LC_INPUT                |   seq#                    | 6    | pr   | sb   |  w   | key ( (w+1) * (80-bit, 10-byte) )  | r1  | m1  | prior1 (96-bit, 12-byte) | r2  | m2  | prior2 (96-bit, 12-byte) | ?on |
 *                              +---------------------------+------+------+------+------+------------------------------------+-----+-----+--------------------------+-----+-----+--------------------------+-----+
 *                              ... 
 *                              +---------------------------+------+------+------+------+------------------------------------+-----+-----+--------------------------+-----+------------------------------+-------------------------+
 *  DBG_LC_OUTPUT               |   seq#                    | 8    | pr   | sb   |  w   | key ( (w+1) * (80-bit, 10-byte) )  | r   | m   | prior1 (96-bit, 12-byte) | sec | AD data (1024 bit, 128-byte) | adwidth (12bit, 2bytes) |
 *                              +---------------------------+------+------+------+------+------------------------------------+-----+-----+--------------------------+-----+------------------------------+-------------------------+
 *                              ... 
 *                              +---------------------------+------+------+------+------+------+----------------------------+ 
 *  DBG_LD3                     |   seq#                    | 9    | dev  | address (4-byte)   | data (160-bit, 20-byte)    |                            
 *                              +---------------------------+------+------+------+------+------+----------------------------+
 *                              ...
 *                              +---------------------------+------+------+------+------+------+------+------+------+ 
 *  MDIO                        |   seq#                    | 10   | w/r  | mpid | dev  | regaddr     | data        |                            
 *                              +---------------------------+------+------+------+------+------+------+------+------+
 *                              ...
 *                              +---------------------------+------+------+------+------+---------------------------+-----+-----+-------------------------+-----+------------------------------+-------------------------+
 *  DBG_LC_LC_LpmAD             |   seq#                    | 11   | pr   | sb   |  1   | key ( 144bit, 18bytes )   | r   | m   | prior (96-bit, 12-byte) | sec | AD data (1024 bit, 128-byte) | adwidth (12bit, 2bytes) |
 *                              +---------------------------+------+------+------+------+---------------------------+-----+-----+-------------------------+-----+------------------------------+-------------------------+
 *                              ... 
 *                              +---------------------------+------+------+
 *  CMT_NSELIB [START]          |   seq#                    | 0x80 | 0    |                                                                                                                             
 *                              +---------------------------+------+------+
 *                              ... 
 *                              +---------------------------+------+------+
 *  CMT_NSELIB [END]            |   seq#                    | 0x80 | 1    |                                                                                                                             
 *                              +---------------------------+------+------+
 *                              ... 
 *                              +---------------------------+------+
 *  CMT_ONLINE_SPLIT_ADD_REC    |   seq#                    | 0x81 |
 *                              +---------------------------+------+
 *                              ... 
 *                              +---------------------------+------+---------------------------+------------------------+
 *  CMT_ONLINE_DEL_REC          |   seq#                    | 0x82 | table name (16-bytes)     | priority (4-bytes)     |
 *                              +---------------------------+------+---------------------------+------------------------+
 *                              ... 
 *                              +---------------------------+------+---------------------------+------------------------+
 *  CMT_ONLINE_ADD_REC          |   seq#                    | 0x83 | table name (16-bytes)     | priority (4-bytes)     |
 *                              +---------------------------+------+---------------------------+------------------------+
 *                              ... 
 *
 *
 */


#define IO_DATA_SIZE_BIT_COUNT              (160)
#define IO_DATA_SIZE_BYTE_COUNT             (BYTE_COUNT(IO_DATA_SIZE_BIT_COUNT))
#define IO_DATA_SIZE_HEX_COUNT              (HEX_COUNT(IO_DATA_SIZE_BYTE_COUNT))

#define IO_KEY_MIN_SIZE_BIT_COUNT           (80)
#define IO_KEY_MIN_SIZE_BYTE_COUNT          (BYTE_COUNT(IO_KEY_MIN_SIZE_BIT_COUNT))

#define IO_KEY_MAX_SIZE_BIT_COUNT           (8*IO_KEY_MIN_SIZE_BIT_COUNT)
#define IO_KEY_MAX_SIZE_BYTE_COUNT          (BYTE_COUNT(IO_KEY_MAX_SIZE_BIT_COUNT))
#define IO_KEY_MAX_SIZE_HEX_COUNT           (HEX_COUNT(IO_KEY_MAX_SIZE_BYTE_COUNT))

#define IO_TABLE_NAME_BYTE_COUNT            (16)
#define IO_CMT_STRING_MAX                   (64)
#define IO_PRIORITY_CHANNEL_COUNT           (4)
#define IO_PRIORITY_PER_CHANNEL_BIT_COUNT   (24)
#define IO_PRIORITY_BIT_COUNT               (IO_PRIORITY_CHANNEL_COUNT * IO_PRIORITY_PER_CHANNEL_BIT_COUNT)
#define IO_PRIORITY_BYTE_COUNT              (BYTE_COUNT(IO_PRIORITY_BIT_COUNT))
#define IO_PRIORITY_HEX_COUNT               (HEX_COUNT(IO_PRIORITY_BYTE_COUNT))

#define IO_AD_PER_CHANNEL_BIT_COUNT         (256)
#define IO_AD_PER_CHANNEL_BYTE_COUNT        (BYTE_COUNT(IO_AD_PER_CHANNEL_BIT_COUNT))
#define IO_AD_BIT_COUNT                     (MAX_CHANNELS * IO_AD_PER_CHANNEL_BIT_COUNT)
#define IO_AD_BYTE_COUNT                    (BYTE_COUNT(IO_AD_BIT_COUNT))

#define IO_ADWIDTH_PER_CHANNEL_BIT_COUNT    (3)
#define IO_ADWIDTH_BIT_COUNT                (MAX_CHANNELS * IO_ADWIDTH_PER_CHANNEL_BIT_COUNT)
#define IO_ADWIDTH_BYTE_COUNT               (BYTE_COUNT(IO_ADWIDTH_BIT_COUNT))

#define IO_NFE_XOR_MASK_BIT_COUNT           (160)
#define IO_NFE_XOR_MASK_BYTE_COUNT          (BYTE_COUNT(IO_NFE_XOR_MASK_BIT_COUNT))
#define IO_NFE_XOR_MASK_HEX_COUNT           (HEX_COUNT(IO_NFE_XOR_MASK_BYTE_COUNT))

#define IO_NFE_TYPE_ACL                     (0)
#define IO_NFE_TYPE_LPM                     (1)
#define IO_NFE_TYPE_QUIET                   (2)

#define IO_MDIO_WRITE_OP                    (0)
#define IO_MDIO_WAIT_OP                     (1)
#define IO_MDIO_READ_OP                     (IO_MDIO_WAIT_OP)

#define IO_OPCODE_NOP                       ((u8)0)
#define IO_OPCODE_LC                        ((u8)1)
#define IO_OPCODE_LD2                       ((u8)2)
#define IO_OPCODE_LD3                       ((u8)3)
#define IO_OPCODE_LD5                       ((u8)5)
#define IO_OPCODE_QUIET                     ((u8)6)
#define IO_DBG_NFE                          ((u8)7)
#define IO_DBG_LC_INPUT                     ((u8)8)
#define IO_DBG_LC_OUTPUT                    ((u8)9)
#define IO_DBG_LD3                          ((u8)10)
#define IO_MDIO                             ((u8)11)
#define IO_DBG_LC_LPMAD                     ((u8)12)
#define IO_CMT_NSELIB                       ((u8)0x80)
#define IO_CMT_ONLINE_SPLIT_ADD_REC         ((u8)0x81)
#define IO_CMT_ONLINE_DEL_REC               ((u8)0x82)
#define IO_CMT_ONLINE_ADD_REC               ((u8)0x83)
#define IO_CMT_ONLINE_DEL_TABLE             ((u8)0x84)
#define IO_CMT_ONLINE_ADD_TABLE             ((u8)0x85)
#define IO_CMT_STRING                       ((u8)0x86)
#define IO_OPCODE_INVALID                   ((u8)-1)

#define IO_CMT_NSELIB_DATA_START            ((u8)0)
#define IO_CMT_NSELIB_DATA_END              ((u8)1)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sIoCmd IoCmd;

API const char* IoCmd_Opcode_Name(u8 opcode);

API const char* IoCmd_Name(const IoCmd* cmd);

// create IoCmd large enough to handle longest cmd
API IoCmd*      IoCmd_Create(void);
API void        IoCmd_Destroy(IoCmd* cmd);

API IoCmd*      IoCmd_CreateCopy(const IoCmd* cmd);
API void        IoCmd_Copy(IoCmd* dst, const IoCmd* src);

API TcError     IoCmd_Get_OpCode(const IoCmd* cmd, u8* opcode);

API TcError     IoCmd_Set_Nop(IoCmd* cmd, const u16* pCnt);
API TcError     IoCmd_Set_Quiet(IoCmd* cmd);
API TcError     IoCmd_Set_LC(IoCmd* cmd, const u8* pProfile, const u16* pSbAddr, const u8* pWidth, const void* key, int keySize);
API TcError     IoCmd_Set_LD2(IoCmd* cmd, const u8* pDev, const void* data, int dataSize);
API TcError     IoCmd_Set_LD3(IoCmd* cmd, const u8* pDev, const u32* pAddress);
API TcError     IoCmd_Set_LD5(IoCmd* cmd, const u8* pProfile, const u16* pSbAddr, const u8* pWidth, const void* key, int keySize);
API TcError	IoCmd_Set_DBG_LC_LpmAD(IoCmd* cmd, const u8* pProfile, const u16* pSbAddr, const u8* pWidth, const void* key, int keySize, 
					                     const u8* pReady1, const u8* pMatch1, const void* prior1, int prior1Size, 
						                 const u8* psec, const void* addata, int adsize, const void* adwidth, int adwidthsize);
API TcError     IoCmd_Set_DBG_LC_Input(IoCmd* cmd, const u8* pProfile, const u16* sbAddr, const u8* pWidth, const void* key, int keySize, 
                                         const u8* pReady1, const u8* pMatch1, const void* prior1, int prior1Size, 
                                         const u8* pReady2, const u8* pMatch2, const void* prior2, int prior2Size, 
                                         const u8* pIsOnline);
API TcError     IoCmd_Set_DBG_LC_Output(IoCmd* cmd, const u8* pProfile, const u16* sbAddr, const u8* pWidth, const void* key, int keySize, 
                                         const u8* pReady, const u8* pMatch, const void* prior, int priorSize, 
                                         const u8* pSec, const void* ad, int adSize, const void* adWidth, int adWidthSize);
API TcError     IoCmd_Set_DBG_NFE(IoCmd* cmd, const u8* pDev, const u8* pOctopus, const u8* pModule, const u8* pMemType, const u8* pType, const void* xorMask, int xorMaskSize);
API TcError     IoCmd_Set_DBG_LD3(IoCmd* cmd, const u8* pDev, const u32* pAddress, const void* data, int dataSize );
API TcError     IoCmd_Set_MDIO(IoCmd* cmd, const u8* operation, const u8* pMpid, const u8* pDevAddr, const u16* pRegAddr, const u16* pRegData);
API TcError     IoCmd_Set_Cmt_NseLib(IoCmd* cmd, const u8* cmData);
API TcError     IoCmd_Set_Cmt_OnlineSplitAddRecord(IoCmd* cmd, const char* tableName, const u32* priority);
API TcError     IoCmd_Set_Cmt_OnlineDeleteRecord(IoCmd* cmd, const char* tableName, const u32* priority);
API TcError     IoCmd_Set_Cmt_OnlineAddRecord(IoCmd* cmd, const char* tableName, const u32* priority);
API TcError     IoCmd_Set_Cmt_OnlineDeleteTable(IoCmd* cmd, const char* tableName, const u32* id);
API TcError     IoCmd_Set_Cmt_OnlineAddTable(IoCmd* cmd, const char* tableName, const u32* id);
API TcError     IoCmd_Set_Cmt_String(IoCmd* cmd, const char* fmt, ...);

API u8          IoCmd_isInitialized(const IoCmd* cmd);
API TcError     IoCmd_Get_Nop(const IoCmd* cmd, u16* pCnt);
API TcError     IoCmd_Get_LC(const IoCmd* cmd, u8* pProfile, u16* pSbAddr, u8* pWidth, const void** ppKey, int* pKeySize);
API TcError     IoCmd_Get_LD2(const IoCmd* cmd, u8* pDev, const void** data, int* pDataSize);
API TcError     IoCmd_Get_LD3(const IoCmd* cmd, u8* pDev, u32* pAddress);
API TcError     IoCmd_Get_LD5(const IoCmd* cmd, u8* pProfile, u16* pSbAddr, u8* pWidth, const void** ppKey, int* pKeySize);
API TcError	IoCmd_Get_DBG_LC_LpmAD(const IoCmd* cmd, u8* pProfile, u16* pSbAddr, u8* pWidth, const void** ppKey, int* pKeySize, 
                                    u8* pReady1, u8* pMatch1, const  void** ppPrior1, int* pPrior1Size, 
                                    u8* psec, const void** ppaddata, int* padsz , const void** ppadwidth, int* padwdsz);
API TcError     IoCmd_Get_DBG_LC_Input(const IoCmd* cmd, u8* pProfile, u16* pSbAddr, u8* pWidth, const void** ppKey, int* pKeySize, 
                                     u8* pReady1, u8* pMatch1, const void** ppPrior1, int* pPrior1Size, 
                                     u8* pReady2, u8* pMatch2, const void** ppPrior2, int* pPrior2Size, 
                                     u8* pIsOnline);
API TcError     IoCmd_Get_DBG_LC_Output(const IoCmd* cmd, u8* pProfile, u16* pSbAddr, u8* pWidth, const void** ppKey, int* pKeySize, 
                                 u8* pReady, u8* pMatch, const void** ppPrior, int* pPriorSize, 
                                 u8* pSec, const void** ad, int* adSize, const void** adWidth, int* adWidthSize);
API TcError     IoCmd_Get_DBG_NFE(const IoCmd* cmd, u8* pDev, u8* octopus, u8* module, u8* memType, u8* type, const void** xorMask, int* pXorMaskSize);
API TcError     IoCmd_Get_DBG_LD3(const IoCmd* cmd, u8* pDev, u32* pAddress, const void** ppData, int* pDataSize);
API TcError     IoCmd_Get_MDIO(const IoCmd* cmd, u8* operation, u8* pMpid, u8* pDevAddr, u16* pRegAddr, u16* pRegData);
API TcError     IoCmd_Get_Cmt_NseLib(const IoCmd* cmd, u8* pData);
API TcError     IoCmd_Get_Cmt_OnlineSplitAddRecord(const IoCmd* cmd, const char** ppTableName, u32* pPriority);
API TcError     IoCmd_Get_Cmt_OnlineDeleteRecord(const IoCmd* cmd, const char** ppTableName, u32* pPriority);
API TcError     IoCmd_Get_Cmt_OnlineDeleteTable(const IoCmd* cmd, const char** tableName, u32* pId);
API TcError     IoCmd_Get_Cmt_OnlineAddTable(const IoCmd* cmd, const char** tableName, u32* pId);
API TcError     IoCmd_Get_Cmt_OnlineAddRecord(const IoCmd* cmd, const char** tableName, u32* pPriority);
API TcError     IoCmd_Get_Cmt_String(const IoCmd* cmd, const char** str, u8* pLen);
API int         IoCmd_Size(void);

#ifdef __cplusplus
}
#endif

#endif /* __IoCmd_h__ */

