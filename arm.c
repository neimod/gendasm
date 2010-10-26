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
} arm_opcodetype;

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
	FLAG_SHIFTOP_REG = (1<<1),
	FLAG_SHIFTOP_IMM = (1<<2),
	FLAG_OP_IMM = (1<<3),
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

const char* mnemonics[18]=
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
};


const char* formats[18]=
{
	"%o%c%s %d, %n, %i",			// AND
	"%o%c%s %d, %n, %i",			// EOR
	"%o%c%s %d, %n, %i",			// SUB
	"%o%c%s %d, %n, %i",			// RSB
	"%o%c%s %d, %n, %i",			// ADD
	"%o%c%s %d, %n, %i",			// ADC
	"%o%c%s %d, %n, %i",			// SBC
	"%o%c%s %d, %n, %i",			// RSC
	"%o%c %n, %i",					// TST
	"%o%c %n, %i",					// TEQ
	"%o%c %n, %i",					// CMP
	"%o%c %n, %i",					// CMN
	"%o%c%s %d, %n, %i",			// ORR
	"%o%c%s %d, %i",				// MOV
	"%o%c%s %d, %n, %i",			// BIC
	"%o%c%s %d, %i",				// MVN
	"%o%c %a",						// B
	"%o%c %a",						// BL
};

static void arm_dataprocess_reg_shift(unsigned int in, arm_opcode* op);
static void arm_dataprocess_imm(unsigned int in, arm_opcode* op);
static void arm_dataprocess_imm_shift(unsigned int in, arm_opcode* op);
void arm_format(arm_context* ctx, char* buf, arm_opcode* op);

%GENDASM.DECODER%

void arm_undefined(unsigned int in, arm_opcode* op)
{
	op->opcode = ~0;
}

void arm_dataprocess(unsigned int in, arm_opcode* op)
{
	if (in & (1<<25))
		arm_dataprocess_imm(in, op);
	else if ((in & (1<<4)) == 0)
		arm_dataprocess_imm_shift(in, op);
	else
		arm_dataprocess_reg_shift(in, op);
}

void arm_dataprocess_imm(unsigned int in, arm_opcode* op)
{
	unsigned int imm8 = in & 255;
	unsigned int rot = ((in >> 8) & 15)*2;

	op->opcode = (in >> 21) & 15;
	op->cond = (in >> 28) & 15;
	op->imm = (imm8>>rot) | (imm8<<(32-rot));
	op->shiftop = 0;
	op->rd = (in >> 12) & 15;
	op->rn = (in >> 16) & 15;
	op->rm = 0;
	op->rs = 0;
	op->flags = FLAG_OP_IMM;
	if (in & (1<<20))
		op->flags |= FLAG_S;
}

void arm_dataprocess_imm_shift(unsigned int in, arm_opcode* op)
{
	op->opcode = (in >> 21) & 15;
	op->cond = (in >> 28) & 15;
	op->imm = (in >> 7) & 31;
	op->shiftop = (in >> 5) & 3;
	op->rd = (in >> 12) & 15;
	op->rn = (in >> 16) & 15;
	op->rm = (in >> 0) & 15;
	op->rs = 0;
	op->flags = FLAG_SHIFTOP_IMM;
	if (in & (1<<20))
		op->flags |= FLAG_S;

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

void arm_dataprocess_reg_shift(unsigned int in, arm_opcode* op)
{
	op->opcode = (in >> 21) & 15;
	op->cond = (in >> 28) & 15;
	op->imm = 0;
	op->shiftop = (in >> 5) & 3;
	op->rd = (in >> 12) & 15;
	op->rn = (in >> 16) & 15;
	op->rm = (in >> 0) & 15;
	op->rs = (in >> 8) & 15;
	op->flags = FLAG_SHIFTOP_REG;
	if (in & (1<<20))
		op->flags |= FLAG_S;
}



void arm_format(arm_context* ctx, char* buf, arm_opcode* op)
{
	char tmp[256];
	const char* fmt;
	unsigned int fmtlen;
	
	*buf = 0;
	
	if (op->opcode == ~0)
	{
		return;
	}

	fmt = formats[op->opcode];
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
				case 'a':
					sprintf(tmp, "0x%X", ctx->base + op->imm);
					strcat(buf, tmp);
				break;

				case 'o':
					strcat(buf, mnemonics[op->opcode]);
				break;

				case 's':
					if (op->flags & FLAG_S)
						strcat(buf, "S");
				break;

				case 'c':
					strcat(buf, conds[op->cond]);
				break;

				case 'n':
					strcat(buf, registers[op->rn]);
				break;

				case 'd':
					strcat(buf, registers[op->rd]);
				break;

				case 'i':
					if (op->flags & FLAG_SHIFTOP_IMM)
					{
						//Rm LSL #imm
						strcat(buf, registers[op->rm]);

						if (op->shiftop == RRX)
							strcat(buf, ", RRX");
						else if (op->imm != 0)
						{
							strcat(buf, ", ");
							strcat(buf, shifts[op->shiftop]);
							sprintf(tmp, " #%d", op->imm);
							strcat(buf, tmp);
						}
					}
					else if (op->flags & FLAG_SHIFTOP_REG)
					{
						//Rm LSL Rs
						strcat(buf, registers[op->rm]);
						strcat(buf, ", ");
						strcat(buf, shifts[op->shiftop]);
						strcat(buf, " ");
						strcat(buf, registers[op->rs]);
					}
					else if (op->flags & FLAG_OP_IMM)
					{
						//#imm
						sprintf(tmp, "#0x%X", op->imm);
						strcat(buf, tmp);
					}
				break;

				default:
				break;
			}
		}
	}
}