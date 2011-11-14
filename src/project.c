#include "spimcore.h"

#define DEBUG_PROJECT 1

// TODO:
// figure out the rest of instruction_decode()'s control signals

/* ALU */
/* 10 Points */
void ALU(unsigned A, unsigned B, char ALUControl, unsigned *ALUresult, char *Zero)
{
	if(DEBUG_PROJECT) printf("DEBUG: A = %d, B = %d\n", A, B);

	// operands are passed in as unsigned values.
	// we will need these for operation on signed
	// integers
	int signedA = (int)A;
	int signedB = (int)B;

	// called from ALU_operations
	// this is where we calculate the result of an ALU operation (based on the ALUControl)
	switch(ALUControl) {
		case 0:
			//*ALUresult = signedA + signedB;
			*ALUresult = A + B;
			break;
		case 1:
			//*ALUresult = signedA - signedB;
			*ALUresult = A - B;
			break;
		case 2:
			*ALUresult = signedA < signedB;
			break;
		//case 3 is for unsigned
		case 3:
			*ALUresult = A < B;
			break;
		case 4:
			*ALUresult = A & B;
			break;
		case 5:
			*ALUresult = A | B;
			break;
		case 6:
			*ALUresult = B << 16;
			break;
		case 7:
			*ALUresult = ~A;
			break;
		default:
			printf("Invalid control value %d passed to the ALU.\n", ALUControl);
	}
	*Zero = !(*ALUresult);

	if(DEBUG_PROJECT) printf("DEBUG: ALUControl = %d, ALUresult = %d, Zero = %d\n", ALUControl, *ALUresult, *Zero);
}

/* instruction fetch */
/* 10 Points */
int instruction_fetch(unsigned PC, unsigned *Mem, unsigned *instruction)
{
	//TODO: check appropriate return value for errors (alignment and
	//out of bounds are considered two different errors)

	if(DEBUG_PROJECT) printf("DEBUG: wordalign = %d, pc = 0x%X, (pc >> 2) = 0x%X, memsize = 0x%X\n", !(PC % 4), PC, PC >> 2, MEMSIZE);

	// this should (ideally) check for word alignment and check for out-of-bounds
	if(PC % 4 != 0 || (PC >> 2) >= MEMSIZE) {
		if(DEBUG_PROJECT) printf("DEBUG: HALT!\n");
		return 1;
	}

	// Mem is an array of words and PC is the actual address value
	*instruction = Mem[PC >> 2];

	return 0;
}


/* instruction partition */
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1, unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
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
	// 0000 0000 0000 0000 0000 0000 0001 0101
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
	// XXXX XXXX XXXX XXXX XXXX XXXX XX10 1010 &
	// 0000 0000 0000 0000 0000 0000 0011 1111 =
	// 0000 0000 0000 0000 0000 0000 0010 1010
	*funct = instruction & 0x0000003F;

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

	if(DEBUG_PROJECT) printf("DEBUG: op = 0x%X, r1 = 0x%X, r2 = 0x%X, r3 = 0x%X, funct = 0x%X, offset = 0x%X, jsec = 0x%X\n", *op, *r1, *r2, *r3, *funct, *offset, *jsec);
}


/* instruction decode */
/* 15 Points */
int instruction_decode(unsigned op, struct_controls *controls)
{
	// I have been going off of what the FAQ says for these values, but we should
	// also check the logic involved as I am not entirely confident that they are
	// correct.

	// set up some nice default values for controls
	controls->MemRead = 2;
	controls->MemWrite = 2;
	controls->RegWrite = 2;

	controls->RegDst = 2;
	controls->Branch = 2;

	controls->ALUSrc = 2;
	controls->ALUOp = 0;

	// these NEED to be 0, otherwise some stuff gets messed up in PC_update,
	// write_register, etc.
	controls->MemtoReg = 0;
	controls->Jump = 0;

	switch(op) {
		case 0x00:
			/* R-type instructions */
			// See FAQ, Q8 regarding what do with ALUOp for R-type instructions
			controls->RegWrite = 1;
			controls->RegDst = 1;
			controls->ALUSrc = 0;
			controls->ALUOp = 7;
			break;
		case 0x03: // j
			controls->Jump = 1;
			break;
		default:
			/* I-type instructions */
			controls->ALUSrc = 1;
			controls->RegDst = 0;
			controls->RegWrite = 1;
			switch(op) {
				case 0x04: // beq
					controls->Branch = 1;
					controls->RegWrite = 0;
					break;
				case 0x08: // addi
					break;
				case 0x0A: // slti
					controls->ALUOp = 2;
					break;
				case 0x0B: // sltiu
					controls->ALUOp = 3;
					break;
				case 0x0F: // lui
					controls->ALUOp = 6;
					break;
				case 0x23: // lw
					controls->MemRead = 1;
					controls->MemtoReg = 1;
					break;
				case 0x2B: // sw
					controls->RegDst = 2;
					controls->MemWrite = 1;
					controls->RegWrite = 0;
					break;
				default: // invalid instruction
					if(DEBUG_PROJECT) printf("DEBUG: HALT!\n");
					printf("DEBUG: op = 0x%X!\n", op);
					return 1;
			}
	}

	return 0;
}


/* Read Register */
/* 5 Points */
// Reg is the register file (array of unsigned ints)
// The Reg array is initialized to 0 with a memset function
// See line 82 in spimcore.c
// r1 and r2 are the register numbers to be loaded
void read_register(unsigned r1, unsigned r2, unsigned *Reg, unsigned *data1, unsigned *data2)
{
	// Reg[0] = 0 by default so we don't need to check if r1 or r2 are equal to 0
	// may need to check for out of bounds here (or elsewhere) to make sure r1
	// and r2 are legal
	*data1 = Reg[r1];
	*data2 = Reg[r2];
}


/* Sign Extend */
/* 10 Points */
void sign_extend(unsigned offset, unsigned *extended_value)
{
	// AND out the upper bits and store the lower bits of offset into
	// extended_value
	*extended_value = (offset & 0x0000FFFF);

	// check if bit 15 is set (ie, "is this negative?")
	// if so, set the upper 16 bits to 1
	if(*extended_value & 0x00008000)
		*extended_value |= 0xFFFF0000;
}


/* ALU operations */
/* 10 Points */
// TODO: Need to account for ALUControl = 7 below
int ALU_operations(unsigned data1, unsigned data2, unsigned extended_value, unsigned funct, char ALUOp, char ALUSrc, unsigned *ALUresult, char *Zero)
{
	// ALU is not called from core. It looks like we will need to call it here. 

	// invalid op is anything outside the range [0, 7]
	if(ALUOp < 0 || ALUOp > 7) {
		if(DEBUG_PROJECT) printf("DEBUG: HALT!\n");
		return 1;
	}

	// check for no-op conditions
	if(ALUOp == 7 && funct == 0) return 0;

	// ALUControl is passed to ALU(...) and determines what ALU operation to do
	// See item #4 (page 3) of the Final Project PDF for the possible values
	char ALUControl = ALUOp;

	// ALUOp == 7 essentially means "use the function code"
	if(ALUOp == 7) {
		// determine ALUControl based on the funct code
		switch(funct) {
			case 0x20: // add
				ALUControl = 0;
				break;
			case 0x22: // sub
				ALUControl = 1;
				break;
			case 0x2A: // slt
				ALUControl = 2;
				break;
			case 0x2B: // sltu
				ALUControl = 3;
				break;
			case 0x24: // and
				ALUControl = 4;
				break;
			case 0x25: // or
				ALUControl = 5;
				break;
			default: // invalid function
				if(DEBUG_PROJECT) printf("DEBUG: HALT!\n");
				return 1;
		}
	}

	ALU(data1, (ALUSrc ? extended_value : data2), ALUControl, ALUresult, Zero);
	return 0;
}


/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult, unsigned data2, char MemWrite, char MemRead, unsigned *memdata, unsigned *Mem)
{
	// check for memory out of bounds
	if((ALUresult >> 2) >= MEMSIZE) {
		if(DEBUG_PROJECT) printf("DEBUG: HALT!\n");
		return 1;
	}

	//read or write if MemRead/MemWrite are nonzero
	if(MemRead) {
	    if(DEBUG_PROJECT) printf("Reading memory address %X, value %X.\n", ALUresult >> 2, Mem[ALUresult>>2]);
		*memdata = Mem[ALUresult >> 2];
	}
	if(MemWrite) {
	    if(DEBUG_PROJECT) printf("Reading memory address %X, value %X.\n", ALUresult >> 2, data2);
		Mem[ALUresult >> 2] = data2;
	}
	
	return 0;
}


/* Write Register */
/* 10 Points */
void write_register(unsigned r2, unsigned r3, unsigned memdata, unsigned ALUresult, char RegWrite, char RegDst, char MemtoReg, unsigned *Reg)
{
	// TODO: Need to check what happens if we get passed a no-op with non-zero
	// r2 or r3 ... I am guessing that it will (for right now) just overwrite
	// those registers with 0 (and that's bad).
	//
	//
	// ASSUMPTION: RegWrite is the control signal that determines whether a 
	// reg write will happen
	if(RegWrite) {
		unsigned write_data = (MemtoReg == 1 ? memdata : ALUresult);

		if(DEBUG_PROJECT) printf("DEBUG: RegDst = %d, write_data = %d, r2 = %d, r3 = %d\n", RegDst, write_data, r2, r3);

		if(!RegDst)
			Reg[r2] = (r2 ? write_data : 0);
		else
			Reg[r3] = (r3 ? write_data : 0);
	}
}


/* PC update */
/* 10 Points */
void PC_update(unsigned jsec, unsigned extended_value, char Branch, char Jump, char Zero, unsigned *PC)
{
	if(DEBUG_PROJECT) printf("DEBUG: PC before update = 0x%X\n", *PC);
	*PC += 4;

	if(Branch && Zero) *PC += (extended_value << 2);

	// extended_value is the branch offset
	// Zero is the zeq output from the ALU
	// jsec is the 26 bit immediate (bit 2 -28 of PC)

	if(Jump){
		*PC &= 0xF0000000;

		*PC |= (jsec<<2);
	}
	if(DEBUG_PROJECT) printf("DEBUG: PC after update = 0x%X\n", *PC);
}

