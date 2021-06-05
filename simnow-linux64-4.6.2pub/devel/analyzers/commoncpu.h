/* 
** Copyright 1995 - 2005 ADVANCED MICRO DEVICES, INC.  All Rights Reserved. 
*/

#ifndef __COMMONCPU_H__
#define __COMMONCPU_H__

enum e_SimControl
{
	Control_None,
	Control_Break,
	Control_Save,
	Control_Exit
};

typedef UINT32 MEMTYPE;

//***	Define the different memory types

//**	MEMTYPE_CACHE_MASK - These bits effect cachability of memory

static const MEMTYPE MEMTYPE_CACHE_MASK   = (1 << 2) | (1 << 1) | (1 << 0);
static const MEMTYPE MEMTYPE_NONCACHEABLE = 0x00;
static const MEMTYPE MEMTYPE_WRITECOMBINE = 0x01;
static const MEMTYPE MEMTYPE_WRITETHROUGH = 0x04;
static const MEMTYPE MEMTYPE_WRITEPROTECT = 0x05;
static const MEMTYPE MEMTYPE_WRITEBACK    = 0x06;
static const MEMTYPE MEMTYPE_UNKNOWN      = 0x07;

//**	MEMTYPE_BID_MASK - These bits effect which buss the access is for - i.e. DRAM or PCI

static const MEMTYPE MEMTYPE_WRITEDRAM	= (1 << 3);
static const MEMTYPE MEMTYPE_READDRAM   = (1 << 4);
static const MEMTYPE MEMTYPE_BID_MASK   = (MEMTYPE_READDRAM | MEMTYPE_WRITEDRAM);

//**	MEMTYPE_APICRAM - This bit is used to redirect accesses to the APIC module

static const MEMTYPE MEMTYPE_APICRAM    = (1 << 5);

//*** MEMTYPE_<access>

static const MEMTYPE MEMTYPE_CODE       = (1 << 6);
static const MEMTYPE MEMTYPE_READ       = (1 << 7);

//*		MEMTYPE_NOSNOOP - Used to force the northbridge not to generate snoops for READMEM/WRITEMEM/GETMEMPTR coming in over ncHT

static const MEMTYPE MEMTYPE_NOSNOOP	= (1 << 8);

struct X8664SMMSTATE
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
	UINT64 SVMGuest;
	UINT64 SVMGuestVMCBPA;
	UINT64 SVMGuestVirtIntr;
	UINT8  Reserved2[8];
	UINT32 MachineState;
	UINT32 Revision;
	UINT64 SMMBase;
	UINT8  Reserved3[24];
	UINT64 SVMGuestPAT;
	UINT64 SVMHostEFER;
	UINT64 SVMHostCR4;
	UINT64 SVMHostCR3;
	UINT64 SVMHostCR0;
	UINT64 RegisterCR4;
	UINT64 RegisterCR3;
	UINT64 RegisterCR0;
	UINT64 RegisterDR7;
	UINT64 RegisterDR6;
	UINT64 RegisterEFlags;
	UINT64 RegisterRIP;
	UINT64 IntegerRegs[16];			// R15, R14, R13, ... RDX, RCX, RAX
};

struct FXSAVEAREA
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

// the following is just to make sure the above structures are packed
static inline void CPU_STRUCTURES_ARE_PACKED(bool b)
{
	switch (b)
	{
		case (sizeof(X8664SMMSTATE) == 512) &&
		     (sizeof(FXSAVEAREA) == 512) :
			break;
		case false:
			break;
	}
}

#endif
