/*
 * $Revision: $
 * $Date:  $
 * $Author:  $
 *
 * Copyright 2002 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Definition of the public ICodeInjector class
 */

#ifndef __CODEINJECTOR_H__
#define __CODEINJECTOR_H__

enum INSTRUCTIONBITSIZE		{ Size16Bit, Size32Bit, Size64Bit, SizeUnknown };

enum MonRegNumList
{
		MON_REG_EAX		= 0,
		MON_REG_ECX		= 1,
		MON_REG_EDX		= 2,
		MON_REG_EBX		= 3,
		MON_REG_ESP		= 4,
		MON_REG_EBP		= 5,
		MON_REG_ESI		= 6,
		MON_REG_EDI		= 7,
		MON_REG_R8		= 8,
		MON_REG_R9		= 9,
		MON_REG_R10		= 10,
		MON_REG_R11		= 11,
		MON_REG_R12		= 12,
		MON_REG_R13		= 13,
		MON_REG_R14		= 14,
		MON_REG_R15		= 15,
		MON_REG_R16		= 16,
		MON_REG_R17		= 17,
		MON_REG_R18		= 18,
		MON_REG_R19		= 19,
		MON_REG_R20		= 20,
		MON_REG_R21		= 21,
		MON_REG_R22		= 22,
		MON_REG_R23		= 23,
		MON_REG_R24		= 24,
		MON_REG_R25		= 25,
		MON_REG_R26		= 26,
		MON_REG_R27		= 27,
		MON_REG_R28		= 28,
		MON_REG_R29		= 29,
		MON_REG_R30		= 30,
		MON_REG_R31		= 31,

		MON_REG_ST0		= 32,
		MON_REG_ST1		= 33,
		MON_REG_ST2		= 34,
		MON_REG_ST3		= 35,
		MON_REG_ST4		= 36,
		MON_REG_ST5		= 37,
		MON_REG_ST6		= 38,
		MON_REG_ST7		= 39,
		MON_REG_ST		= 40,

		MON_REG_MM0		= 41,
		MON_REG_MM1		= 42,
		MON_REG_MM2		= 43,
		MON_REG_MM3		= 44,
		MON_REG_MM4		= 45,
		MON_REG_MM5		= 46,
		MON_REG_MM6		= 47,
		MON_REG_MM7		= 48,

		MON_REG_XMM0	= 49,
		MON_REG_XMM1	= 50,
		MON_REG_XMM2	= 51,
		MON_REG_XMM3	= 52,
		MON_REG_XMM4	= 53,
		MON_REG_XMM5	= 54,
		MON_REG_XMM6	= 55,
		MON_REG_XMM7	= 56,
		MON_REG_XMM8	= 57,
		MON_REG_XMM9	= 58,
		MON_REG_XMM10	= 59,
		MON_REG_XMM11	= 60,
		MON_REG_XMM12	= 61,
		MON_REG_XMM13	= 62,
		MON_REG_XMM14	= 63,
		MON_REG_XMM15	= 64,
		MON_REG_XMM16	= 65,
		MON_REG_XMM17	= 66,
		MON_REG_XMM18	= 67,
		MON_REG_XMM19	= 68,
		MON_REG_XMM20	= 69,
		MON_REG_XMM21	= 70,
		MON_REG_XMM22	= 71,
		MON_REG_XMM23	= 72,
		MON_REG_XMM24	= 73,
		MON_REG_XMM25	= 74,
		MON_REG_XMM26	= 75,
		MON_REG_XMM27	= 76,
		MON_REG_XMM28	= 77,
		MON_REG_XMM29	= 78,
		MON_REG_XMM30	= 79,
		MON_REG_XMM31	= 80,

		MON_REG_CR0		= 81,
		MON_REG_CR1		= 82,
		MON_REG_CR2		= 83,
		MON_REG_CR3		= 84,
		MON_REG_CR4		= 85,
		MON_REG_CR5		= 86,
		MON_REG_CR6		= 87,
		MON_REG_CR7		= 88,
		MON_REG_CR8		= 89,
		MON_REG_CR9		= 90,
		MON_REG_CR10	= 91,
		MON_REG_CR11	= 92,
		MON_REG_CR12	= 93,
		MON_REG_CR13	= 94,
		MON_REG_CR14	= 95,
		MON_REG_CR15	= 96,

		MON_REG_DR0		= 97,
		MON_REG_DR1		= 98,
		MON_REG_DR2		= 99,
		MON_REG_DR3		= 100,
		MON_REG_DR4		= 101,
		MON_REG_DR5		= 102,
		MON_REG_DR6		= 103,
		MON_REG_DR7		= 104,
		MON_REG_DR8		= 105,
		MON_REG_DR9		= 106,
		MON_REG_DR10	= 107,
		MON_REG_DR11	= 108,
		MON_REG_DR12	= 109,
		MON_REG_DR13	= 110,
		MON_REG_DR14	= 111,
		MON_REG_DR15	= 112,

		// ordered by modrm reg values
		MON_REG_ES		= 113,
		MON_REG_CS		= 114,
		MON_REG_SS		= 115,
		MON_REG_DS		= 116,
		MON_REG_FS		= 117,
		MON_REG_GS		= 118,

		MON_REG_YMM0	= 119,
		MON_REG_YMM1	= 120,
		MON_REG_YMM2	= 121,
		MON_REG_YMM3	= 122,
		MON_REG_YMM4	= 123,
		MON_REG_YMM5	= 124,
		MON_REG_YMM6	= 125,
		MON_REG_YMM7	= 126,
		MON_REG_YMM8	= 127,
		MON_REG_YMM9	= 128,
		MON_REG_YMM10	= 129,
		MON_REG_YMM11	= 130,
		MON_REG_YMM12	= 131,
		MON_REG_YMM13	= 132,
		MON_REG_YMM14	= 133,
		MON_REG_YMM15	= 134,
		MON_REG_YMM16	= 135,
		MON_REG_YMM17	= 136,
		MON_REG_YMM18	= 137,
		MON_REG_YMM19	= 138,
		MON_REG_YMM20	= 139,
		MON_REG_YMM21	= 140,
		MON_REG_YMM22	= 141,
		MON_REG_YMM23	= 142,
		MON_REG_YMM24	= 143,
		MON_REG_YMM25	= 144,
		MON_REG_YMM26	= 145,
		MON_REG_YMM27	= 146,
		MON_REG_YMM28	= 147,
		MON_REG_YMM29	= 148,
		MON_REG_YMM30	= 149,
		MON_REG_YMM31	= 150,

		MON_REG_NONE	= 151
};

typedef UINT16 MonRegMask;

enum MonRegListType { MON_DST, MON_SRC, MON_MEM };

struct MonRegisterList
{
	MonRegListType type;
	UINT8 numRegs;
	UINT8 registers[8];
};

struct INSTRUCTIONINFO
{
	UINT64 nStructureSize;												// Size of this structure, in bytes
	ADDRTYPE nLinearPC;													// Linear address of this instruction
	ADDRTYPE nPhysicalPC1;												// Physical address of this instruction
	ADDRTYPE nPhysicalPC2;												// (UINT64)-1 if no page wrap, else second portion of physical address

	bool bProtectionEnabled;
	bool bPagingEnabled;
	bool bCSDBit;
	bool bCSLBit;

	UINT8 nLength;														// Length of instruction, in bytes
	UINT8 nPrefixCount;													// # of prefix bytes before opcode

	bool bHasThirdOpcode;												// Is 3rd opcode present?
	UINT8 nThirdOpcode;													// 3rd opcode (after ModRM) for 3DNow

	//	Data and address size indicators

	INSTRUCTIONBITSIZE eDataSize;
	INSTRUCTIONBITSIZE eAddressSize;

	//	ModRegR/M Decoding

	bool bHasModrm;
	bool bHasSib;
	bool bHasScale;
	bool bHasIndex;
	bool bHasBase;
	bool bHasImmediate;
	bool bHasDisplacement;

	//	Various prefix decoding flags and values

	bool bHasPrefix;
	bool bHasSegOvrdPrefix;
	bool bHasRepPrefix;
	bool bHasRepnePrefix;
	bool bHasLockPrefix;
	bool bHasDataOvrdPrefix;
	bool bHasAddrOvrdPrefix;
	bool bHasRexPrefix;
	UINT8 nRexPrefix;

	//	Offset from first byte to the opcode, ModRegR/M, and SIB, if present

	UINT8 nOpcodeOffset;
	UINT8 nModrmOffset;
	UINT8 nSibOffset;
	UINT8 nDisplacementOffset;
	UINT8 nImmediateOffset;

	// More instruction info
	UINT8 nSIBBaseReg;
	UINT8 nSIBIndexReg;
	UINT8 nSibScale;
	UINT64 nDisplacement;
	UINT8 nSegment;

	struct MonRegisterList DestRegs; // dest. reg operands
	struct MonRegisterList SourceRegs; // source regs
	struct MonRegisterList MemRegs; // mem address related regs

};


struct X8664REG
{
	UINT16 SelectorES;
	UINT16 FlagsES;
	UINT32 LimitES;
	UINT64 BaseES;						// Only low 32 bits implemented
	UINT16 SelectorCS;
	UINT16 FlagsCS;
	UINT32 LimitCS;
	UINT64 BaseCS;						// Only low 32 bits implemented
	UINT16 SelectorSS;
	UINT16 FlagsSS;
	UINT32 LimitSS;
	UINT64 BaseSS;						// Only low 32 bits implemented
	UINT16 SelectorDS;
	UINT16 FlagsDS;
	UINT32 LimitDS;
	UINT64 BaseDS;						// Only low 32 bits implemented
	UINT16 SelectorFS;
	UINT16 FlagsFS;
	UINT32 LimitFS;
	UINT64 BaseFS;
	UINT16 SelectorGS;
	UINT16 FlagsGS;
	UINT32 LimitGS;
	UINT64 BaseGS;
	UINT16 SelectorGDT;					// Not used
	UINT16 FlagsGDT;					// Not used
	UINT32 LimitGDT;					// Only low 16 bits implemented
	UINT64 BaseGDT;
	UINT16 SelectorLDT;
	UINT16 FlagsLDT;
	UINT32 LimitLDT;
	UINT64 BaseLDT;
	UINT16 SelectorIDT;					// Not used
	UINT16 FlagsIDT;					// Not used
	UINT32 LimitIDT;					// Only low 16 bits implemented
	UINT64 BaseIDT;
	UINT16 SelectorTR;
	UINT16 FlagsTR;
	UINT32 LimitTR;
	UINT64 BaseTR;
	UINT64 IORestartRIP;
	UINT64 IORestartRCX;
	UINT64 IORestartRSI;
	UINT64 IORestartRDI;
	UINT64 IORestartDword;
	UINT8  IORestartFlag;
	UINT8  HLTRestartFlag;
	UINT8  NMIBlocked;
	UINT8  CurrentPL;
	UINT8  Reserved1[4];
	UINT64 EFERRegister;
	UINT8  Reserved2[32];
	UINT32 MachineState;
	UINT32 Revision;
	UINT64 SMMBase;
	UINT8  Reserved3[64];
	UINT64 RegisterCR4;
	UINT64 RegisterCR3;
	UINT64 RegisterCR0;
	UINT64 RegisterDR7;
	UINT64 RegisterDR6;
	UINT64 RegisterEFlags;
	UINT64 RegisterRIP;
	UINT64 IntegerRegs[16];	// R15, R14, R13, R12, R11, R10, R9, R8, RDI, RSI, RBP, RSP, RBX, RDX, RCX, RAX		
};

struct FX86REG
{
	UINT16	fcw;            // fpu control word
	UINT16	fsw;            // fpu status word
	UINT16	ftw;            // fpu tag word (8bits)
	UINT16	fop;            // fpu opcode
	UINT32	ip;             // fpu instruction pointer offset
	UINT16	cs;             // fpu instruction pointer selector
	UINT16	reserved2;
	UINT32	dp;             // fpu data pointer offset
	UINT16	ds;             // fpu data pointer selector
	UINT16	reserved3;
	UINT32	mxcsr;          // MXCSR register state
	UINT32	mxcsrMask;      // changed jray (mask for MXCSR)
	UINT64  mmx0Mant;
	UINT16  mmx0Exp;
	UINT16	reserved5;
	UINT32	reserved6;
	UINT64  mmx1Mant;
	UINT16  mmx1Exp;
	UINT16	reserved7;
	UINT32	reserved8;
	UINT64  mmx2Mant;
	UINT16  mmx2Exp;
	UINT16	reserved9;
	UINT32	reserved10;
	UINT64  mmx3Mant;
	UINT16  mmx3Exp;
	UINT16	reserved11;
	UINT32	reserved12;
	UINT64  mmx4Mant;
	UINT16  mmx4Exp;
	UINT16	reserved13;
	UINT32	reserved14;
	UINT64  mmx5Mant;
	UINT16  mmx5Exp;
	UINT16	reserved15;
	UINT32	reserved16;
	UINT64  mmx6Mant;
	UINT16  mmx6Exp;
	UINT16	reserved17;
	UINT32	reserved18;
	UINT64  mmx7Mant;
	UINT16  mmx7Exp;
	UINT16	reserved19;
	UINT32	reserved20;
	UINT128	xmm[16];		// xmm registers
	UINT128	reserved[6];
};

enum INJECTSIZE { isByte, isWord, isDword, isQword };
enum INJECTOP { ioMove, ioAdd, ioSubtract };
enum INSTRUCTIONREGISTER { irDest, irSource1, irSource2, irSource3 };

class ICodeInjector
{
public:
	virtual ~ICodeInjector() {};										// Virtual destructor

//	Routines available to the owner of this object to effect code generation

public:

//	Functions used to generate code into the code stream

	virtual void EmitArbitraryBytes(UINT8 *pBytes, int nLength) = 0;
	virtual void Mem2Mem2(void *pSource1AndDest, void *pSource2, INJECTSIZE isSize, INJECTOP ioOperation) = 0;
	virtual void Imm2Mem2(void *pSource1AndDest, UINT64 nSource1, INJECTSIZE isSize, INJECTOP ioOperation) = 0;

//	Functions used to inject event notification into the code stream

	virtual void AddCodeFetchEvent() = 0;
	virtual void AddMemoryEvent() = 0;
	virtual void AddBranchEvent() = 0;
	virtual void AddExcEvent() = 0;
	virtual void AddHeartBeatEvent() = 0;
	virtual void AddTokens(UINT64* pTokens, int num) = 0;
	virtual bool AddRegisterValueTokens(const char *psRegisterName) = 0;

//	Functions used to gain information about the instruction that is currently being translated
	virtual int OpcodeBytes(UINT8 *pDestination, int nLength) = 0;
	virtual bool BuildInstructionInfo(INSTRUCTIONINFO *pInfo, UINT64 nSizeOfStruct) = 0;
	virtual UINT8 ConvertRegInfo(UINT8 reg) = 0;

//  Routines that fetch and set register information
	virtual void GetX8664Reg(X8664REG& x8664Reg) = 0;
	virtual void SetX8664Reg(X8664REG& x8664Reg) = 0;
	virtual void GetFX86Reg(FX86REG& Fx86Reg) = 0;
	virtual void SetFX86Reg(FX86REG& Fx86Reg) = 0;

// Routines accessing the machines simulated memory
	virtual ADDRTYPE GetPhysAddress(ADDRTYPE LinAddress) = 0;
	virtual char *Disassemble(UINT8 *pBuffer, int &Length) = 0;
	virtual bool ReadPhysicalMemory(ADDRTYPE PhysAddress, UINT8 *pBuffer, unsigned int nCount, UINT32 *pBytesDone) = 0;
	virtual bool WritePhysicalMemory(ADDRTYPE PhysAddress, UINT8 *pBuffer, unsigned int nCount, UINT32 *pBytesDone) = 0;
	virtual bool ReadLinearMemory(ADDRTYPE LinAddress, UINT8 *pBuffer, unsigned int nCount, UINT32 *pBytesDone) = 0;
	virtual bool WriteLinearMemory(ADDRTYPE LinAddress, UINT8 *pBuffer, unsigned int nCount, UINT32 *pBytesDone) = 0;
	virtual bool ReadIOPort(UINT16 Port, UINT8 *pBuffer, unsigned int nCount) = 0;
	virtual bool WriteIOPort(UINT16 Port, UINT8 *pBuffer, unsigned int nCount) = 0;

};
#endif


