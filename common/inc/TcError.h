#ifndef __TcError_h__
#define __TcError_h__

#include "types.h"
#include "Platform.h"

typedef s32 TcError;

// errors
enum TcE_Errors {
    TcE_Start                                      = 0x80000000,
    TcE_Alloc_Fail_Conflict,
    TcE_Alloc_Fail_Space,
    TcE_Alloc_Fail_Too_Many_Module,
    TcE_Buffer_Too_Small,
    TcE_Capacity_Reached,
    TcE_EccReg_Invalid_Value,
    TcE_End_Of_Str_Reach,
    TcE_Failed,
    TcE_HRam_Undefined,
    TcE_Ilk_Protocol,
    TcE_Invalid_Argument,
    TcE_Invalid_File_Format,
    TcE_Invalid_HexaDecimal_String,
    TcE_Invalid_Pointer,
    TcE_Invalid_Xml_Str,
    TcE_Logic_Error,
    TcE_Module_Invalid_MemType,
    TcE_Not_Found,
    TcE_Not_Implement,
    TcE_Null_Pointer,
    TcE_Out_Of_Memory,
    TcE_Out_Of_Range,
    TcE_Priority_Field_Invalid,
    TcE_Priority_Line_Format_Inconsistent,
    TcE_Register_Undefine,
    TcE_TcRam_Address_Out_Of_Range,
    TcE_TimeOut,
    TcE_Busy,
    TcE_Tpf_Parser_Invalid_Expected_Priority,
    TcE_Tpf_Parser_Invalid_LC,
    TcE_Tpf_Parser_Invalid_LD,
    TcE_Tpf_Parser_Invalid_Wr,
    TcE_Unsupport_Opcode,
    TcE_XData_Depth_Mismatch,
    TcE_XData_Init_BUFFER_TOO_SMALL,         
    TcE_XData_Invalid_Hi,
    TcE_Xdata_Invalid_Lo,
    TcE_XData_Undefined,
    TcE_XData_Width_Mismatch,
    TcE_End,
};

// success, and  warnings
enum TcE_Warnings {
    TcE_OK                                     = 0,    // success
    TcW_Tpf_Invalid,
    TcW_Comment,
    TcW_Ecc_Ded,
    TcW_Ecc_Reg_Updated,
    TcW_Ecc_Sec,
    TcW_Interface,
    TcW_Empty_Line,
    TcW_Eof,
    TcW_End,
    TcW_Ilk_Partial_Data,
    TcW_Ilk_Invalid,
    TcW_IoCmd_Opcode_Ignored,
};

#ifdef __cplusplus
extern "C" {
#endif

API const char* TcErrorName(TcError rc);

#ifdef __cplusplus
}
#endif

#endif /* __TcError_h__ */
