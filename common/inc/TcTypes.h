#ifndef __TcTypes_h__
#define __TcTypes_h__

#include "types.h"
#include "Platform.h"

// when add new entry, also need to
// update MemType_name() accordingly
typedef enum eMemType {
    MemTypeNone                     = 0,
    MemTypeDram1                    = 1,
    MemTypeDram40                   = 40,
    MemTypeHashRam                  = 41,
    MemTypeBsRegister               = 42,
    MemTypeHRamOffRegister          = 43,
    MemTypePriorityRam              = 44,
    MemTypeXcRam                    = 47,
    MemTypeProfile                  = 49,
    MemTypeXcCtrlRegister           = 50,
    MemTypeSearchBuffer0            = 57,
    MemTypeSearchBuffer1            = 58,
    MemTypeKguProfile0              = 59,  
    MemTypeKguProfile1              = 60,  
    MemTypeRangeEncoderRegister0    = 61,  
    MemTypeRangeEncoderRegister1    = 62,  
    MemTypeRamReadRegister          = 63,
    MemTypeCount
} MemType;

// when add new entry, also need to
// update QuartetMemType_name() accordingly
typedef enum eQuartetMemType {
    QuartetMemTypeModuleConfigReg          = 1,
    QuartetMemTypeTcamReg                  = 2,
    QuartetMemTypeEccStatus                = 3,
    QuartetMemTypeProfileRam               = 4,
    QuartetMemTypeModuleRam                = 5,
    QuartetMemTypeProfileReg               = 6,
	QuartetMemTypeBPCntrReg				   = 7,
	QuartetMemTypeLRUCntrReg			   = 8,
    QuartetMemTypeCount_new
} QuartetMemType;

#ifdef __cplusplus
extern "C" {
#endif

API const char* MemType_name(u8 type);
API const char* QuartetMemType_name(u8 type);

#ifdef __cplusplus
}
#endif

#endif /* __TcamTypes_h__ */
