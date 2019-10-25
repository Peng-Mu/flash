#ifndef __defines_h__
#define __defines_h__

//#define ILK_CODE
//#define PALLADIUM_CODE
//#define COMPARE_TPFS
//#define VENDOR1
//#define VENDOR2
//#define VENDOR3
//#define VENDOR4
//#define SHOW_COMMENTS
//#define GET_DEVICE_ID_FROM_OCTOPUS
//#define RAMREADREGISTERBIT

#define MAX_PROFILES_IN_BIT                 (7)
#define MAX_PROFILES                        (1<<MAX_PROFILES_IN_BIT)
#define MAX_SBADDR                          (0x7FFF)
#define MAX_CHANNELS                        (4)
#define MAX_CASCADE_DEVICES                 (4)

#define FILE_NAME_LENGTH				   512
#define	BITS_PER_HEXCHAR				   (4)
#define BYTE_SHIFT                         (3)
#define BITS_PER_BYTE                      (1<<BYTE_SHIFT)
#define BITS_PER_10_BYTES                  (8 * 10)
#define BITS_PER_S32                       (4*BITS_PER_BYTE)
#define BITS_PER_S64                       (2*BITS_PER_S32)
#define MOD_PER_OCT                        (8)
#define VMM_PER_MOD                        (4)
#define DATA_DRAM_WIDTH_IN_BITS            (7)                     /* by spec: 128 */
#define DATA_DRAM_WIDTH                    (1<<DATA_DRAM_WIDTH_IN_BITS)
#define MAX_CHARS						   (2048)
#define	KGU_KEYS						   (4)
#define TABLE_IN_FIELDS                    (9)
#define AD_IN_FIELDS                       (8)

#define MAX_BYTES						   (1024)
#define MAX_READ_BITS					   (140)
#define LD_CMD_RD_BIT_COUNT										32
#define LD_CMD_LD_BIT_COUNT										16
#define LD_CMD_ECC_BIT_COUNT									26
#define LD_CMD_RD_MODULE_NUMBER_START							0
#define LD_CMD_RD_MODULE_NUMBER_WIDTH							4
#define LD_CMD_RD_OCTOPUS_NUMBER_START							(LD_CMD_RD_MODULE_NUMBER_START + LD_CMD_RD_MODULE_NUMBER_WIDTH)
#define LD_CMD_RD_OCTOPUS_NUMBER_WIDTH							3
#define LD_CMD_RD_OPCODE_START									(LD_CMD_RD_OCTOPUS_NUMBER_START + LD_CMD_RD_OCTOPUS_NUMBER_WIDTH)
#define LD_CMD_RD_OPCODE_WIDTH									2
#define LD_CMD_RD_INSTANCE_SELECT_START							(LD_CMD_RD_OPCODE_START + LD_CMD_RD_OPCODE_WIDTH)
#define LD_CMD_RD_INSTANCE_SELECT_WIDTH							6
#define LD_CMD_RD_ADDRESS_START									(LD_CMD_RD_INSTANCE_SELECT_START + LD_CMD_RD_INSTANCE_SELECT_WIDTH)
#define LD_CMD_RD_ADDRESS_WIDTH									15

#define TcKGU_KEY_DATA_BIT_START			(30)

#define TcKGU_KEY_OPCODE										(0x2)	//2
#define LD_CMD_RD_OPCODE										(0x1)	//1



// Profile  RAM configuration

#define Q_ProfileMem_QuartEn_Start		(0)
#define Q_ProfileMem_QuartEn_Bitcnt		(1)
#define Q_ProfileMem_ChanSel_Start		(Q_ProfileMem_QuartEn_Start+Q_ProfileMem_QuartEn_Bitcnt)
#define Q_ProfileMem_ChanSel_Bitcnt		(2)
//#define Q_ProfileMem_KeySel_Start		(Q_ProfileMem_ChanSel_Start+Q_ProfileMem_ChanSel_Bitcnt)
//#define Q_ProfileMem_KeySel_Bitcnt		(1)


// Module Config Reg
#define Q_MODCONFIG_TCAM_NX_REG_Start			(0)
#define Q_MODCONFIG_TCAM_NX_REG_Bitcnt			(8)
#define Q_MODCONFIG_TCAM_ValidBit_B_START		(Q_MODCONFIG_TCAM_NX_REG_Start + Q_MODCONFIG_TCAM_NX_REG_Bitcnt)
#define Q_MODCONFIG_TCAM_ValidBit_B_Bitcnt		(8)
#define Q_MODCONFIG_TCAM_ValidBit_A_START		(Q_MODCONFIG_TCAM_ValidBit_B_START + Q_MODCONFIG_TCAM_ValidBit_B_Bitcnt)
#define Q_MODCONFIG_TCAM_ValidBit_A_Bitcnt		(8)
#define Q_MODCONFIG_TCAM_OrMask_START			(Q_MODCONFIG_TCAM_ValidBit_A_START + Q_MODCONFIG_TCAM_ValidBit_A_Bitcnt)
#define Q_MODCONFIG_TCAM_OrMask_Bitcnt			(14)
#define Q_MODCONFIG_TCAM_hash_Direct_START		(Q_MODCONFIG_TCAM_OrMask_START + Q_MODCONFIG_TCAM_OrMask_Bitcnt)
#define Q_MODCONFIG_TCAM_hash_Direct_Bitcnt		(1)
#define Q_MODCONFIG_TCAM_Tcam_Invalid_START		(Q_MODCONFIG_TCAM_hash_Direct_START + Q_MODCONFIG_TCAM_hash_Direct_Bitcnt)
#define Q_MODCONFIG_TCAM_Tcam_Invalid_Bitcnt	(1)
#define Q_MODCONFIG_TCAM_NReg_START				(Q_MODCONFIG_TCAM_Tcam_Invalid_START + Q_MODCONFIG_TCAM_Tcam_Invalid_Bitcnt)
#define Q_MODCONFIG_TCAM_NReg_Bitcnt			(2)
#define Q_MODCONFIG_TCAM_MReg_START				(Q_MODCONFIG_TCAM_NReg_START + Q_MODCONFIG_TCAM_NReg_Bitcnt)
#define Q_MODCONFIG_TCAM_MReg_Bitcnt			(2)


//Profile Reg
#define Q_PROFILE_REG_MODULE0_Mode_START				(0)
#define Q_PROFILE_REG_MODULE0_Mode_Bitcnt				(3)
#define Q_PROFILE_REG_MODULE1_Mode_START				(Q_PROFILE_REG_MODULE0_Mode_START+Q_PROFILE_REG_MODULE0_Mode_Bitcnt)
#define Q_PROFILE_REG_MODULE1_Mode_Bitcnt				(3)
#define Q_PROFILE_REG_MODULE2_Mode_START				(Q_PROFILE_REG_MODULE1_Mode_START+Q_PROFILE_REG_MODULE1_Mode_Bitcnt)
#define Q_PROFILE_REG_MODULE2_Mode_Bitcnt				(3)
#define Q_PROFILE_REG_MODULE3_Mode_START				(Q_PROFILE_REG_MODULE2_Mode_START+Q_PROFILE_REG_MODULE2_Mode_Bitcnt)
#define Q_PROFILE_REG_MODULE3_Mode_Bitcnt				(3)
#define Q_PROFILE_REG_AD_Width_Start					(Q_PROFILE_REG_MODULE3_Mode_START + Q_PROFILE_REG_MODULE3_Mode_Bitcnt)
#define Q_PROFILE_REG_AD_Width_Bitcnt					(3)
#define Q_PROFILE_REG_Priority_AND_Start				(Q_PROFILE_REG_AD_Width_Start + Q_PROFILE_REG_AD_Width_Bitcnt)
#define Q_PROFILE_REG_Priority_AND_Bitcnt				(24)
#define Q_PROFILE_REG_Priority_SEL_Start				(Q_PROFILE_REG_Priority_AND_Start + Q_PROFILE_REG_Priority_AND_Bitcnt)
#define Q_PROFILE_REG_Priority_SEL_Bitcnt				(24)
#define Q_PROFILE_REG_BYTE_PACKET_CNTR_RST_EN_START		(Q_PROFILE_REG_Priority_SEL_Start + Q_PROFILE_REG_Priority_SEL_Bitcnt)
#define Q_PROFILE_REG_BYTE_PACKET_CNTR_RST_EN_Bitcnt	(1)

#define Q_PROFILE_REG_MODULE_DISABLE				(0)
#define Q_PROFILE_REG_MODULE_AD						(1)
#define Q_PROFILE_REG_MODULE_BYTCNTR				(2)
#define Q_PROFILE_REG_MODULE_LRU					(3)
#define Q_PROFILE_REG_MODULE_LPM					(4)

// Counter Configuration
#define Q_BP_MODULE_PACKETCNTR_START				(0)
#define Q_BP_MODULE_PACKETCNTR_WIDTH				(25)
#define Q_BP_MODULE_BYTECNTR_START					(Q_BP_MODULE_PACKETCNTR_START+Q_BP_MODULE_PACKETCNTR_WIDTH)
#define Q_BP_MODULE_BYTECNTR_WIDTH					(39)
#define Q_BPCNTR_REG_PACKETCNTR_START				(0) // output config
#define Q_BPCNTR_REG_PACKETCNTR_WIDTH				(26)
#define Q_BPCNTR_REG_BYTECNTR_START					(32)
#define Q_BPCNTR_REG_BYTECNTR_WIDTH					(40)

/* key defines */
#define TcKey_WIDTH                                 (640)                   /* by spec */
#define TcKey_BYTE_COUNT                            ((TcKey_WIDTH + BITS_PER_BYTE - 1) / BITS_PER_BYTE)
#define TcSB_KEY_SIZE_CHARS					(160) //640 bits, 80 BYTES, 160 chars
#define KGU_KEY_WIDTH_IN_CHARS			    (TcSB_KEY_SIZE_CHARS + 1)
#define TcSB_KEY_WIDTH_BYTES				(80) //640 bits, 80 BYTES
#define TcSB_DEPTH_IN_BITS					(12) //4k
#define TcSB_COUNT_IN_BITS					(3)  //8 modules
#define TcSB_COUNT							(1<<TcSB_COUNT_IN_BITS)  //8 modules
#define TcSB_DEPTH      					(1<<TcSB_DEPTH_IN_BITS) //4k
#define TcSB_WIDTH_IN_BITS					(80) //80 bits
#define TcSB_LD5_ADDRESS_SHIFT				(TcSB_COUNT_IN_BITS)
#define TcSB_MODULE_MASK				((1<<TcSB_LD5_ADDRESS_SHIFT)-1)
#define TcSB_LD2_MODULE_SHIFT			    (TcSB_DEPTH_IN_BITS)
#define TcSB_ADDRESS_MASK				((1<<TcSB_LD2_MODULE_SHIFT)-1)
#define TcRReg_COUNT						(8)
#define TcRReg_DEPTH_IN_BITS                (32)
#define TcRReg_MODULE_OFFSET				(0x5)
#define TcRReg_ADDR_OFFSET					(0x1F)

#define TcSB_KEY_PER_MODULE_BYTES			(BYTE_COUNT(TcSB_WIDTH_IN_BITS))
#define TcSB_KEY_PER_MODULE_CHARS			(20) // 80 bits, 10 bytes, 20 chars

#define TcKGU_MODULE_SHIFT					(0x7)
#define TcKGU_ADDR_MASK					(0x7F)
#define ECCReg_START_ADDR                   (0x380)


#define LC_CHANNEL_PRIORITY_BIT_CNT         (24)

#define CPSS_NSE_MANAGER_ASCII_VAL_0        (48)

#define TcKGU_COUNT							(4)

#define ECC_REG_FLAG_START                  (0)
#define ECC_REG_FLAG_BIT_CNT                (2)
#define ECC_REG_MEMTYPE_START               (ECC_REG_FLAG_START + ECC_REG_FLAG_BIT_CNT)
#define ECC_REG_MEMTYPE_BIT_CNT             (6)
#define ECC_REG_ADDR_START                  (ECC_REG_MEMTYPE_START + ECC_REG_MEMTYPE_BIT_CNT)
#define ECC_REG_ADDR_BIT_CNT                (15)
#define ECC_REG_MODULE_START                (ECC_REG_ADDR_START + ECC_REG_ADDR_BIT_CNT)
#define ECC_REG_MODULE_BIT_CNT              (3)
#define ECC_REG_WIDTH                       (ECC_REG_MODULE_START + ECC_REG_MODULE_BIT_CNT)

#define ECC_REG_ADDRESS                     (0x3FF)
#define ECC_REG_FLAG_NONE                   (0)
#define ECC_REG_FLAG_SEC                    (2)
#define ECC_REG_FLAG_DED                    (3)

#define DELIM   ",\t\n\r "

#define TcOctopus_PROFILE_MODULE_SHIFT              (MAX_PROFILES_IN_BIT)
#define TcOctopus_PROFILE_ADDR_MASK                 ((1<<MAX_PROFILES_IN_BIT) - 1)

#define PORT_CNT                            (2)

#define Q_ECC_ERROR_START                   (0)
#define Q_ECC_ERROR_BIT_COUNT               (2)
#define Q_ECC_ISEL_START                    (Q_ECC_ERROR_START + Q_ECC_ERROR_BIT_COUNT)
#define Q_ECC_ISEL_BIT_COUNT                (4)
#define Q_ECC_ADDRESS_START                 (Q_ECC_ISEL_START + Q_ECC_ISEL_BIT_COUNT)
#define Q_ECC_ADDRESS_BIT_COUNT             (14)
#define Q_ECC_WIDTH                         (Q_ECC_ADDRESS_START+Q_ECC_ADDRESS_BIT_COUNT)


#endif /* __defines_h__ */
