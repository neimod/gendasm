#include "types.h"

%GENDASM.DECODER%

void armv4_undefined(unsigned int in, opcode* op)
{
}

void armv4_dataprocess(unsigned int in, opcode* op)
{
	op->type = table[in];
}
