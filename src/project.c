#include "spimcore.h"

// TODO:
// figure out the rest of instruction_decode()'s control signals

/* ALU */
/* 10 Points */
void ALU(unsigned A,unsigned B,char ALUControl,unsigned *ALUresult,char *Zero)
{

}

/* instruction fetch */
/* 10 Points */
int instruction_fetch(unsigned PC,unsigned *Mem,unsigned *instruction)
{
	// this should (ideally) check for word alignment and check for out-of-bounds
	if((PC >> 2) % 4 != 0 || (PC >> 2) > MEMSIZE) return 1;

	// this assumes Mem is byte addressable
	*instruction = MEM(PC);

	return 0;
}


/* instruction partition */
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
{
	// For the following, I use an alternating (1010...) bit string to represent
	// the value that we are trying to isolate. X's represent "don't care"s.

	/* R, I, and J-type */
	// op (bits 31-26)
	// 1010 10XX XXXX XXXX XXXX XXXX XXXX XXXX >> 26
	// 0000 0000 0000 0000 0000 0000 0010 1010
	*op = instruction >> 26;

	/* R and I-type */
	// rs (bits 25-21)
	// XXXX XX10 101X XXXX XXXX XXXX XXXX XXXX &
	// 0000 0011 1111 1111 1111 1111 1111 1111 =
	// 0000 0010 101X XXXX XXXX XXXX XXXX XXXX >> 21
	// 0000 0000 0000 0000 0000 0000 000
	*r1 = (instruction & 0x03FFFFFF) >> 21;

	/* R and I-type */
	// rt (bits 20-16)
	// XXXX XXXX XXX1 0101 XXXX XXXX XXXX XXXX &
	// 0000 0000 0001 1111 1111 1111 1111 1111 =
	// 0000 0000 0001 0101 XXXX XXXX XXXX XXXX >> 16
	// 0000 0000 0000 0000 0000 0000 0001 0101
	*r2 = (instruction & 0x001FFFFF) >> 16;

	/* R-type */
	// rd (bits 15-11)
	// XXXX XXXX XXXX XXXX 1010 1XXX XXXX XXXX &
	// 0000 0000 0000 0000 1111 1111 1111 1111 =
	// 0000 0000 0000 0000 1010 1XXX XXXX XXXX >> 11
	// 0000 0000 0000 0000 0000 0000 0001 0101
	*r3 = (instruction & 0x0000FFFF) >> 11;

	/* R-type */
	// funct (bits 5-0)
	// XXXX XXXX XXXX XXXX XXXX XXXX XXX1 0101 &
	// 0000 0000 0000 0000 0000 0000 0001 1111 =
	// 0000 0000 0000 0000 0000 0000 0001 0101
	*funct = instruction & 0x0000001F;

	/* I-type */
	// immediate (bits 15-0)
	// XXXX XXXX XXXX XXXX 1010 1010 1010 1010 &
	// 0000 0000 0000 0000 1111 1111 1111 1111 =
	// 0000 0000 0000 0000 1010 1010 1010 1010
	*offset = instruction & 0x0000FFFF;

	/* J-type */
	// jump address (bits 25-0)
	// XXXX XX10 1010 1010 1010 1010 1010 1010 &
	// 0000 0011 1111 1111 1111 1111 1111 1111 =
	// 0000 0010 1010 1010 1010 1010 1010 1010
	*jsec = instruction & 0x03FFFFFF;
}


/* instruction decode */
/* 15 Points */
int instruction_decode(unsigned op,struct_controls *controls)
{
	// I have been going off of what the FAQ says for these values, but we should
	// also check the logic involved as I am not entirely confident that they are
	// correct.

	// set up some nice "don't care" values for controls
	controls.MemRead = 2;
	controls.MemWrite = 2;
	controls.RegWrite = 2;

	controls.RegDst = 2;
	controls.Jump = 2;
	controls.Branch = 2;
	controls.MemtoReg = 2;

	controls.ALUSrc = 2;
	controls.ALUOp = 0;

	if(op == 0x00) {
		/* R-type instructions */
		// See FAQ, Q8 regarding what do with ALUOp for R-type instructions
		controls.RegDst = 1;
		controls.ALUSrc = 0;
		controls.ALUOp = 7;
	} else if(op == 0x03) { // j
		controls.Jump = 1;
	} else {
		/* I-type instructions */
		controls.ALUSrc = 1;
		if(op == 0x04) { // beq
			controls.Branch = 1;
		} else if(op == 0x08) { // addi
		} else if(op == 0x0A) { // slti
			controls.ALUOp = 2;
		} else if(op == 0x0B) { // sltiu
			controls.ALUOp = 3;
		} else if(op == 0x0F) { // lui
		} else if(op == 0x23) { // lw
			controls.MemRead = 1;
			controls.RegWrite = 1;
		} else if(op == 0x2B) { // sw
			controls.RegDst = 2;
			controls.MemWrite = 1;
			controls.MemToReg = 1;
		} else { // invalid instruction
			return 1;
		}
	}

	return 0;
}


/* Read Register */
/* 5 Points */
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2)
{

}


/* Sign Extend */
/* 10 Points */
void sign_extend(unsigned offset,unsigned *extended_value)
{

}


/* ALU operations */
/* 10 Points */
int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,unsigned funct,char ALUOp,char ALUSrc,unsigned *ALUresult,char *Zero)
{

}


/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{

}


/* Write Register */
/* 10 Points */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{

}


/* PC update */
/* 10 Points */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{
    //jsec is the 26 bit immediate (bit 2 -28)
    //*PC += 4;
    //if(Branch && Jump) PC += extended_value;
}

