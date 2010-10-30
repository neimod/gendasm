#include <stdio.h>
#include <string.h>

typedef struct
{
	unsigned int base;
	unsigned int archver;
} arm_context;

typedef struct
{
	unsigned int opcode;
	unsigned int cond;
	unsigned int imm;
	unsigned int shiftop;
	unsigned int rd;
	unsigned int rn;
	unsigned int rm;
	unsigned int rs;
	unsigned int flags;
	unsigned int fmt;
} arm_opcode;


typedef enum
{
	AND = 0,
	EOR = 1,
	SUB = 2,
	RSB = 3, 
	ADD = 4,
	ADC = 5,
	SBC = 6,
	RSC = 7,
	TST = 8,
	TEQ = 9,
	CMP = 10,
	CMN = 11,
	ORR = 12,
	MOV = 13,
	BIC = 14,
	MVN = 15,
	B = 16,
	BL = 17,
	BKPT = 18,
	BLX = 19,
	BX = 20,
	BXJ = 21,
	LDR = 22,
	LDRB = 23,
	LDRT = 24,
	LDRBT = 25,
	STR = 26,
	STRB = 27,
	STRT = 28,
	STRBT = 29,
	LDRH = 30,
	LDRSH = 31,
	LDRSB = 32,
	STRH = 33,
	LDRD = 34,
	STRD = 35,
} arm_opcodetype;

typedef enum
{
	FMT_RD_RN_IMM,
	FMT_RD_RN_RM,
	FMT_RD_RN_RM_SHIFT_IMM,
	FMT_RD_RN_RM_SHIFT,
	FMT_RD_RN_RM_SHIFT_RS,
	FMT_RN_IMM,
	FMT_RN_RM,
	FMT_RN_RM_SHIFT_IMM,
	FMT_RN_RM_SHIFT,
	FMT_RN_RM_SHIFT_RS,
	FMT_RD_IMM,
	FMT_RD_RM,
	FMT_RD_RM_SHIFT_IMM,
	FMT_RD_RM_SHIFT,
	FMT_RD_RM_SHIFT_RS,
	FMT_ADDR,
	FMT_IMM,
	FMT_RM,
	FMT_RD_ADDR_RN,
	FMT_RD_ADDR_RN_IMM,
	FMT_RD_ADDR_RN_RM,
	FMT_RD_ADDR_RN_RM_SHIFT_IMM,
	FMT_RD_ADDR_RN_RM_SHIFT,
	FMT_RD_ADDR_RN_POST_IMM,
	FMT_RD_ADDR_RN_POST_RM,
	FMT_RD_ADDR_RN_POST_RM_SHIFT_IMM,
	FMT_RD_ADDR_RN_POST_RM_SHIFT,
} arm_formats;

typedef enum
{
	LSL = 0,
	LSR = 1,
	ASR = 2,
	ROR = 3,
	RRX = 4
} arm_shifttype;

typedef enum
{
	FLAG_S = (1<<0),
	FLAG_NEGATIVE = (1<<1),
	FLAG_WRITEBACK = (1<<2),
} arm_flags;


const char* conds[16]=
{
	"EQ",
	"NE",
	"CS",
	"CC",
	"MI",
	"PL",
	"VS",
	"VC",
	"HI",
	"LS",
	"GE",
	"LT",
	"GT",
	"LE",
	"",
	""
};

const char* shifts[5]=
{
	"LSL",
	"LSR",
	"ASR",
	"ROR",
	"RRX"
};

const char* registers[16]=
{
	"R0",
	"R1",
	"R2",
	"R3",
	"R4",
	"R5",
	"R6",
	"R7",
	"R8",
	"R9",
	"R10",
	"R11",
	"R12",
	"SP",
	"LR",
	"PC",
};

const char* mnemonics[]=
{
	"AND",
	"EOR",
	"SUB",
	"RSB",
	"ADD",
	"ADC",
	"SBC",
	"RSC",
	"TST",
	"TEQ",
	"CMP",
	"CMN",
	"ORR",
	"MOV",
	"BIC",
	"MVN",
	"B",
	"BL",
	"BKPT",
	"BLX",
	"BX",
	"BXJ",
	"LDR",
	"LDRB",
	"LDRT",
	"LDRBT",
	"STR",
	"STRB",
	"STRT",
	"STRBT",
	"LDRH",
	"LDRSH",
	"LDRSB",
	"STRH",
	"LDRD",
	"STRD",
};



const char* formats[]=
{
	"%o%c%z %d, %n, %i",				// FMT_RD_RN_IMM
	"%o%c%z %d, %n, %m",				// FMT_RD_RN_RM
	"%o%c%z %d, %n, %m, %h #%i",		// FMT_RD_RN_RM_SHIFT_IMM
	"%o%c%z %d, %n, %m, %h",			// FMT_RD_RN_RM_SHIFT
	"%o%c%z %d, %n, %m, %h %s",			// FMT_RD_RN_RM_SHIFT_RS
	
	"%o%c%z %n, %i",					// FMT_RN_IMM
	"%o%c%z %n, %m",					// FMT_RN_RM
	"%o%c%z %n, %m, %h #%i",			// FMT_RN_RM_SHIFT_IMM
	"%o%c%z %n, %m, %h",				// FMT_RN_RM_SHIFT
	"%o%c%z %n, %m, %h %s",				// FMT_RN_RM_SHIFT_RS
	
	"%o%c%z %d, %i",					// FMT_RD_IMM
	"%o%c%z %d, %m",					// FMT_RD_RM
	"%o%c%z %d, %m, %h #%i",			// FMT_RD_RM_SHIFT_IMM
	"%o%c%z %d, %m, %h",				// FMT_RD_RM_SHIFT
	"%o%c%z %d, %m, %h %s",				// FMT_RD_RM_SHIFT_RS

	"%o%c%z %a",						// FMT_ADDR
	"%o%c%z %i",						// FMT_IMM
	"%o%c%z %m",						// FMT_RM
	
	"%o%c%z %d, [%n]%b",				// FMT_RD_ADDR_RN
	"%o%c%z %d, [%n, #%u%i]%b",			// FMT_RD_ADDR_RN_IMM
	"%o%c%z %d, [%n, %u%m]%b",			// FMT_RD_ADDR_RN_RM
	"%o%c%z %d, [%n, %u%m, %h #%i]%b",	// FMT_RD_ADDR_RN_RM_SHIFT_IMM
	"%o%c%z %d, [%n, %u%m, %h]%b",		// FMT_RD_ADDR_RN_RM_SHIFT
	"%o%c%z %d, [%n], #%u%i",			// FMT_RD_ADDR_RN_POST_IMM
	"%o%c%z %d, [%n], %u%m",			// FMT_RD_ADDR_RN_POST_RM
	"%o%c%z %d, [%n], %u%m, %h #%i",	// FMT_RD_ADDR_RN_POST_RM_SHIFT_IMM
	"%o%c%z %d, [%n], %u%m, %h",		// FMT_RD_ADDR_RN_POST_RM_SHIFT
};


static void arm_dataprocess_reg_shift(unsigned int in, arm_opcode* op);
static void arm_dataprocess_imm(unsigned int in, arm_opcode* op);
static void arm_dataprocess_imm_shift(unsigned int in, arm_opcode* op);
static void arm_shift(unsigned int in, arm_opcode* op);
static void arm_nullstub(unsigned int in, arm_opcode* op);

static void arm_clearop(arm_opcode* op);
void arm_format(arm_context* ctx, char* buf, arm_opcode* op);


// --- BEGIN GENERATED DECODER ---
%GENDASM.DECODER%
// --- END GENERATED DECODER ---

void arm_ldrstrmisc(unsigned int in, arm_opcode* op)
{
	unsigned int type = ((in>>5) & 3) | ((in>>18)&4);


	arm_clearop(op);
		
	switch(type)
	{
		case 1: op->opcode = STRH; break;
		case 2: op->opcode = LDRD; break;
		case 3: op->opcode = STRD; break;
		case 5: op->opcode = LDRH; break;
		case 6: op->opcode = LDRSB; break;
		case 7: op->opcode = LDRSH; break;
		default: arm_undefined(in, op); return;
	}
	
	op->cond = (in >> 28) & 15;
	op->rn = (in>>16) & 15;
	op->rd = (in>>12) & 15;
	
	if (in & (1<<22))
	{
		op->imm = ((in & 0xF00)>>4) | (in & 15);
		
		if (in & (1<<24))
			op->fmt = FMT_RD_ADDR_RN_IMM;
		else
			op->fmt = FMT_RD_ADDR_RN_POST_IMM;
	}
	else
	{
		op->rm = in & 15;
		
		if (in & (1<<24))
			op->fmt = FMT_RD_ADDR_RN_RM;
		else
			op->fmt = FMT_RD_ADDR_RN_POST_RM;
	}

	if ((in & (1<<23)) == 0)
		op->flags |= FLAG_NEGATIVE;
		
	if (in & (1<<21))
		op->flags |= FLAG_WRITEBACK;
}

void arm_ldrstr(unsigned int in, arm_opcode* op)
{
	arm_clearop(op);
	
	//PW
	//00  LDR, postindex
	//01  LDRT, postindex
	//10  LDR, preindex without writeback
	//11  LDR, preindex with writeback

	if (in & (1<<20))
	{
		if ((in & (1<<24)) == 0 && (in & (1<<21)))
		{
			if (in & (1<<22))
				op->opcode = LDRBT;
			else
				op->opcode = LDRT;
		}
		else
		{
			if (in & (1<<22))
				op->opcode = LDRB;
			else
				op->opcode = LDR;
		}
	}
	else
	{
		if ((in & (1<<24)) == 0 && (in & (1<<21)))
		{
			if (in & (1<<22))
				op->opcode = STRBT;
			else
				op->opcode = STRT;
		}
		else
		{
			if (in & (1<<22))
				op->opcode = STRB;
			else
				op->opcode = STR;
		}
	}

	op->cond = (in >> 28) & 15;
	op->rn = (in >> 16) & 15;
	op->rd = (in >> 12) & 15;
	
	if ((in & (1<<25)) == 0)
	{
		op->imm = in & 0xFFF;
	}
	else
	{
		op->rm = in & 15;

		arm_shift(in, op);
	}
	
	if ((in & (1<<23)) == 0)
		op->flags |= FLAG_NEGATIVE;
		
		
	if (in & (1<<24))
	{
		if (in & (1<<21))
			op->flags |= FLAG_WRITEBACK;
			

		if ((in & (1<<25)) == 0)
		{
			if (op->imm)
				op->fmt = FMT_RD_ADDR_RN_IMM;
			else
				op->fmt = FMT_RD_ADDR_RN;
		}
		else if (op->shiftop == RRX)
			op->fmt = FMT_RD_ADDR_RN_RM_SHIFT;
		else if (op->imm)
			op->fmt = FMT_RD_ADDR_RN_RM_SHIFT_IMM;
		else
			op->fmt = FMT_RD_ADDR_RN_RM;
	}
	else
	{
		if ((in & (1<<25)) == 0)
		{
			if (op->imm)
				op->fmt = FMT_RD_ADDR_RN_POST_IMM;
			else
				op->fmt = FMT_RD_ADDR_RN;
		}
		else if (op->shiftop == RRX)
			op->fmt = FMT_RD_ADDR_RN_POST_RM_SHIFT;
		else if (op->imm)
			op->fmt = FMT_RD_ADDR_RN_POST_RM_SHIFT_IMM;
		else
			op->fmt = FMT_RD_ADDR_RN_POST_RM;
	}
}


void arm_bxj(unsigned int in, arm_opcode* op)
{
	arm_clearop(op);

	op->opcode = BXJ;
	op->cond = (in >> 28) & 15;
	op->rm = in & 15;
	op->fmt = FMT_RM;
}

void arm_bx(unsigned int in, arm_opcode* op)
{
	arm_clearop(op);

	op->opcode = BX;
	op->cond = (in >> 28) & 15;
	op->rm = in & 15;
	op->fmt = FMT_RM;
}


void arm_bkpt(unsigned int in, arm_opcode* op)
{
	arm_clearop(op);

	op->opcode = BKPT;
	op->imm = ((in&0xFFF00)>>4) | (in & 15);
	op->fmt = FMT_IMM;
}

void arm_blx_imm(unsigned int in, arm_opcode* op)
{
	arm_clearop(op);
	
	op->opcode = BLX;
	
	if (in & 0x800000)
		op->imm = (in | (~0xFFFFFF)) + 8;
	else
		op->imm = (in & 0xFFFFFF) + 8;
		
	if (in & (1<<24))
		op->imm |= 2;
	else
		op->imm &= ~2;
		
	op->fmt = FMT_ADDR;
}

void arm_blx_reg(unsigned int in, arm_opcode* op)
{
	arm_clearop(op);
	
	op->opcode = BLX;
	op->cond = (in >> 28) & 15;
	op->rm = in & 15;
	op->fmt = FMT_RM;
}


void arm_branch(unsigned int in, arm_opcode* op)
{
	arm_clearop(op);
	
	if (in & 0x1000000)
		op->opcode = BL;
	else
		op->opcode = B;
	op->cond = (in >> 28) & 15;

	if (in & 0x800000)
		op->imm = ((in | (~0xFFFFFF)) << 2) + 8;
	else
		op->imm = ((in & 0xFFFFFF) << 2) + 8;
		
	op->fmt = FMT_ADDR;
}


void arm_dataprocess(unsigned int in, arm_opcode* op)
{
	if (in & (1<<25))
		arm_dataprocess_imm(in, op);
	else if ((in & (1<<4)) == 0)
		arm_dataprocess_imm_shift(in, op);
	else
		arm_dataprocess_reg_shift(in, op);
		
	if (in & (1<<20))
	{
		switch(op->opcode)
		{
			case CMP:
			case CMN:
			case TST:
			case TEQ:
				break;
				
			default:
				op->flags |= FLAG_S;
		}
	}
	
	switch(op->opcode)
	{
		case CMP:
		case CMN:
		case TEQ:
		case TST:
			op->fmt += 5;
		break;
		
		case MOV:
		case MVN:
			op->fmt += 10;
		break;
	}
}

void arm_dataprocess_imm(unsigned int in, arm_opcode* op)
{
	unsigned int imm8 = in & 255;
	unsigned int rot = ((in >> 8) & 15)*2;
	
	
	arm_clearop(op);

	op->opcode = (in >> 21) & 15;
	op->cond = (in >> 28) & 15;
	op->imm = (imm8>>rot) | (imm8<<(32-rot));
	op->rd = (in >> 12) & 15;
	op->rn = (in >> 16) & 15;
	
	op->fmt = FMT_RD_RN_IMM;
}

void arm_shift(unsigned int in, arm_opcode* op)
{
	op->shiftop = (in >> 5) & 3;
	op->imm = (in >> 7) & 31;

	switch(op->shiftop)
	{
		case LSR:
		case ASR:
			if (op->imm == 0)
				op->imm = 32;
		break;

		case ROR:
			if (op->imm == 0)
				op->shiftop = RRX;
		break;

		default:
			break;
	}
}

void arm_dataprocess_imm_shift(unsigned int in, arm_opcode* op)
{
	arm_clearop(op);

	op->opcode = (in >> 21) & 15;
	op->cond = (in >> 28) & 15;
	op->rd = (in >> 12) & 15;
	op->rn = (in >> 16) & 15;
	op->rm = (in >> 0) & 15;
	
	arm_shift(in, op);
	
	if (op->shiftop == RRX)
		op->fmt = FMT_RD_RN_RM_SHIFT;
	else if (op->imm)
		op->fmt = FMT_RD_RN_RM_SHIFT_IMM;
	else
		op->fmt = FMT_RD_RN_RM;
}

void arm_dataprocess_reg_shift(unsigned int in, arm_opcode* op)
{
	arm_clearop(op);
	
	op->opcode = (in >> 21) & 15;
	op->cond = (in >> 28) & 15;
	op->shiftop = (in >> 5) & 3;
	op->rd = (in >> 12) & 15;
	op->rn = (in >> 16) & 15;
	op->rm = (in >> 0) & 15;
	op->rs = (in >> 8) & 15;

	op->fmt = FMT_RD_RN_RM_SHIFT_RS;
}



void arm_format(arm_context* ctx, char* buf, arm_opcode* op)
{
	char tmp[256];
	const char* fmt;
	unsigned int fmtlen;
	
	*buf = 0;
	
	if (op->fmt == ~0)
	{
		return;
	}

	fmt = formats[op->fmt];
	fmtlen = strlen(fmt);
	

	while(*fmt)
	{
		if (*fmt != '%')
		{
			strncat(buf, fmt, 1);
			fmt++;
		}
		else
		{
			fmt++;

			switch(*fmt++)
			{
				case 'i':
					sprintf(tmp, "0x%X", op->imm);
					strcat(buf, tmp);
				break;
				
				case 'b':
					if (op->flags & FLAG_WRITEBACK)
						strcat(buf, "!");
				break;
				
				case 'u':
					if (op->flags & FLAG_NEGATIVE)
						strcat(buf, "-");
				break;
				
				case 'm':
					strcat(buf, registers[op->rm]);
				break;
				
				case 'a':
					sprintf(tmp, "0x%X", ctx->base + op->imm);
					strcat(buf, tmp);
				break;

				case 'o':
					strcat(buf, mnemonics[op->opcode]);
				break;

				case 'z':
					if (op->flags & FLAG_S)
						strcat(buf, "S");
				break;

				case 'c':
					strcat(buf, conds[op->cond]);
				break;
				
				case 'h':
					strcat(buf, shifts[op->shiftop]);
				break;

				case 'n':
					strcat(buf, registers[op->rn]);
				break;

				case 'd':
					strcat(buf, registers[op->rd]);
				break;

				case 's':
					strcat(buf, registers[op->rs]);
				break;

				default:
				break;
			}
		}
	}
}

void arm_undefined(unsigned int in, arm_opcode* op)
{
	arm_clearop(op);
}

void arm_clearop(arm_opcode* op)
{
	op->opcode = 0;
	op->shiftop = 0;
	op->cond = 14;
	op->imm = 0;
	op->shiftop = 0;
	op->rd = 0;
	op->rn = 0;
	op->rm = 0;
	op->rs = 0;
	op->flags = 0;
	op->fmt = ~0;
}

void arm_nullstub(unsigned int in, arm_opcode* op)
{
	arm_clearop(op);
}
