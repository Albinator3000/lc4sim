/*
 * LC4.c: Defines simulator functions for executing instructions
 */

#include "LC4.h"
#include <stdio.h>

// instructions in sections
#define INSN_OP(I) ((I) >> 12)// bits 12-15 - Opcode
#define INSN_Rt(I) ((I) & 0x7) // bits 0-2: Rt
#define INSN_Rs(I) (((I) >> 6) & 0x7)//bits 6-8: Rs
#define INSN_Rd(I) (((I) >> 9) & 0x7)//bits 9-11 - Rd
#define INSN_IMM6(I) ((I) & 0x3F)// bits 0-5: IMM6
#define INSN_IMM9(I) ((I) & 0x1FF)// bits 0-8: IMM9
#define INSN_IMM11(I) ((I) & 0x7FF)// bits 0-11: IMM11
#define SEXT(val, bits) (((val) & (1 << ((bits)-1))) ? ((val) | (~((1 << (bits)) - 1))) : (val))

/*
 * Reset the machine state as Pennsim would do
 */
void Reset(MachineState* CPU)
{
    CPU->PC = 0x8200;// Set PC
    CPU->PSR = 0;// Clear PSR
    for (int i = 0; i < 8; i++) { // Clear REGs
        CPU->R[i] = 0;
    }
    ClearSignals(CPU);// Clear CTRLSIGs
}


/*
 * Clear all of the control signals (set to 0)
 */
void ClearSignals(MachineState* CPU)
{
	// Clear MUX
    CPU->rsMux_CTL = 0;
    CPU->rtMux_CTL = 0;
    CPU->rdMux_CTL = 0;
    
    // Clear WE
    CPU->regFile_WE = 0;
    CPU->NZP_WE = 0;
    CPU->DATA_WE = 0;
    
    // Clear OUT
    CPU->regInputVal = 0;
    CPU->NZPVal = 0;
    CPU->dmemAddr = 0;
    CPU->dmemValue = 0;
}


/*
 * This function should write out the current state of the CPU to the file output.
 */
void WriteOut(MachineState* CPU, FILE* output)
{
    unsigned short instr = CPU->memory[CPU->PC]; // get instruction
    
    fprintf(output, "%04X ", CPU->PC); // print PC hex
    
    for (int i = 15; i >= 0; i--) {
        fprintf(output, "%d", (instr >> i) & 1); // print bin instr
    }
    fprintf(output, " ");
    
    // Print control signals and values
    fprintf(output, "%d ", CPU->regFile_WE); //REG WE
    fprintf(output, "%d ", CPU->rdMux_CTL);  //
    fprintf(output, "%04X ", CPU->regInputVal);  // register value
    fprintf(output, "%d ", CPU->NZP_WE); // NZP WE
    fprintf(output, "%d ", CPU->NZPVal); // NZP Val
    fprintf(output, "%d ", CPU->DATA_WE); // DATA WE
    fprintf(output, "%04X ", CPU->dmemAddr); // dmemAddr
    fprintf(output, "%04X", CPU->dmemValue); //DmemVal
    fprintf(output, "\n"); //want instructions to be on different lines
}


/*
 * This function should execute one LC4 datapath cycle.
 */
int UpdateMachineState(MachineState* CPU, FILE* output)
{
    if (CPU->PC == 0x80FF) { // exit the program
				printf("reached PC == 0x80FF so we leave the program");
        return 1;
    }
    
    ClearSignals(CPU); // reset signals
    unsigned short instr = CPU->memory[CPU->PC]; // get instr
    WriteOut(CPU, output); // current state
    unsigned short opcode = INSN_OP(instr); // parse instruction
    
    switch (opcode) {
        case 0: // BRN
            BranchOp(CPU, output);
            break;
        case 1: // ARTH
            ArithmeticOp(CPU, output);
            break;
        case 2: // CMPR
            ComparativeOp(CPU, output);
            break;
				case 4: // JSR/JSRR  
            JSROp(CPU, output);
            break;
        case 5: // LOG
            LogicalOp(CPU, output);
            break;
        case 6: // LDR
						{
							unsigned short instr = CPU->memory[CPU->PC];
							unsigned short base_reg = INSN_Rs(instr); // bits 6-8
							short offset = SEXT(INSN_IMM6(instr), 6); // sign extend offset
							unsigned short addr = CPU->R[base_reg] + offset; // calculate addr
							
							// set control signals
							CPU->regFile_WE = 1; // LDRneeds DATA WE enabled for accessing data mem
							CPU->rdMux_CTL = INSN_Rd(instr);
							CPU->regInputVal = CPU->memory[addr]; // load from mem
							CPU->R[CPU->rdMux_CTL] = CPU->regInputVal;
							CPU->NZP_WE = 1;
							SetNZP(CPU, CPU->regInputVal);
							CPU->PC++;
						}
						break;
        case 7: // STR
					{	
						unsigned short instr = CPU->memory[CPU->PC];
						unsigned short base_reg = INSN_Rs(instr); // bits 6-8
						short offset = SEXT(INSN_IMM6(instr), 6); // sign extend offset
						unsigned short addr = CPU->R[base_reg] + offset; // calculate addr
						CPU->DATA_WE = 1; // STR needs Data WE access enabled for accessing data meme
						CPU->dmemAddr = addr;
						CPU->dmemValue = CPU->R[INSN_Rd(instr)]; // store Rd to memory
						CPU->memory[addr] = CPU->dmemValue;
						CPU->PC++;
					}
            break;
				case 8:// RTI
            CPU->PSR = 0;
            CPU->regFile_WE = 0;
            CPU->NZP_WE = 0;
            CPU->DATA_WE = 0;
            CPU->PC = CPU->R[7];
            break;
				case 9: // CONST
            {
							CPU->regFile_WE = 1;
							CPU->rdMux_CTL = INSN_Rd(instr); //Rd
							CPU->regInputVal = SEXT(INSN_IMM9(instr), 9); //get Imm9 val
							CPU->R[CPU->rdMux_CTL] = CPU->regInputVal; 
							CPU->NZP_WE = 1;
							SetNZP(CPU, CPU->regInputVal);
							CPU->PC++;
            }
            break;
				case 10: // SLL/SRA/SRL/MOD
            ShiftModOp(CPU, output);
            break;
        case 12: // JMP/JMPR
            JumpOp(CPU, output);
            break;
        case 13:	// HICONST
						CPU->regFile_WE = 1; // set control sigs
						CPU->NZP_WE = 1;
						CPU->DATA_WE = 0;
						CPU->rdMux_CTL = INSN_Rd(instr); // get dest register
						CPU->regInputVal = (CPU->R[CPU->rdMux_CTL] & 0x00FF) | ((instr & 0x00FF) << 8); // keep low 8 bits, set high 8 bits       
						CPU->R[CPU->rdMux_CTL] = CPU->regInputVal;
						SetNZP(CPU, CPU->regInputVal);
						CPU->PC++;
						break;
				case 15:	// TRAP
						CPU->PSR = 1; //enter OS mode
						CPU->regFile_WE = 1; //set control sigs
						CPU->NZP_WE = 1;
						CPU->DATA_WE = 0;
						CPU->regInputVal = CPU->PC+1;//save ret addr
						CPU->rdMux_CTL = 7; //write to R7
						SetNZP(CPU, CPU->regInputVal);
						CPU->R[7] = CPU->PC+1;// store return address in reg 7
						CPU->PC = (0x8000 | (instr & 0x00FF)); // jump to trap vector
						break;
        
        
        default:
            //PC=PC+1
            CPU->PC++;
            break;
    }
    
    return 0; // Continue execution
}



//////////////// PARSING HELPER FUNCTIONS ///////////////////////////



/*
 * Parses rest of branch operation and updates state of machine.
 */
void BranchOp(MachineState* CPU, FILE* output)
{
    unsigned short instr = CPU->memory[CPU->PC];
    unsigned short subop = (instr >> 9) & 0x7; //bits 9-11 for branch cond
    short offset = SEXT(INSN_IMM9(instr), 9);//. sign extend 9-bit offset
    int should_branch = 0; // flag to determin if we branch
    
    switch (subop) {
        case 0: // PC = PC + 1, no comparison needed
            should_branch = 0;
            break;
        case 1: // BRP: pos
            should_branch = (CPU->PSR & 0x0001) != 0; // check P bit
            break;
        case 2: // BRZ: zero
            should_branch = (CPU->PSR & 0x0002) != 0; //check Z bit
            break;
        case 3: // BRZP: zero or pos
            should_branch = (CPU->PSR & 0x0003) != 0; // check Z or P bits
            break;
        case 4: // BRN: neg
            should_branch = (CPU->PSR & 0x0004) != 0; // check N bit
            break;
        case 5: // BRNP: neg or pos
            should_branch = (CPU->PSR & 0x0005) != 0; // check N or P bits
            break;
        case 6: // BRNZ: neg or zero
            should_branch = (CPU->PSR & 0x0006) != 0; // check N or Z bits
            break;
        case 7: // BRNZP: "PC = PC+1" -> advance regardless of cond outcome
            should_branch = 1;
            break;
    }
    
    if (should_branch) {
        CPU->PC = CPU->PC + 1 + offset; // PC = PC + 1 + offset
    } else {
        CPU->PC++; // PC = PC + 1 only
    }
}

/*
 * Parses rest of arithmetic operation and prints out.
 */
void ArithmeticOp(MachineState* CPU, FILE* output)
{
    unsigned short instr = CPU->memory[CPU->PC];
    unsigned short subop = (instr >> 3) & 0x7; // make subopcode
		CPU->rdMux_CTL = INSN_Rd(instr); //set control segs
		CPU->regFile_WE = 1;
    CPU->NZP_WE = 1;
    short return_val;
    
    switch (subop) {
        case 0: // ADD
            if ((instr & 0x20) == 0) { // bit 5 = 0, register mode
                return_val = CPU->R[INSN_Rs(instr)] + CPU->R[INSN_Rt(instr)]; // add two reg vals
            } else { // bit 5 = 1, immediate mode
                return_val = CPU->R[INSN_Rs(instr)] + SEXT(INSN_IMM6(instr), 6); // add reg + imm6 val
            }
            break;
        case 1: // MUL
            if ((instr & 0x20) == 0) {
                return_val = CPU->R[INSN_Rs(instr)] * CPU->R[INSN_Rt(instr)]; // multiply 2 reg vals
            } else {
                return_val = CPU->R[INSN_Rs(instr)] * SEXT(INSN_IMM6(instr), 6);
            }
            break;
        case 2: // SUB
            if ((instr & 0x20) == 0) {
                return_val = CPU->R[INSN_Rs(instr)] - CPU->R[INSN_Rt(instr)]; // sub regs
            } else {
                return_val = CPU->R[INSN_Rs(instr)] - SEXT(INSN_IMM6(instr), 6); // sub num from reg
            }
            break;
        case 3: // DIV
            if ((instr & 0x20) == 0) {
                if (CPU->R[INSN_Rt(instr)] != 0) { // divide by a reg
                    return_val = CPU->R[INSN_Rs(instr)] / CPU->R[INSN_Rt(instr)];
                } else { //no divide by 0
                    return_val = 0;
                }
            } else {
                short number = SEXT(INSN_IMM6(instr), 6); //the number to divide is a stand alone value
                if (number != 0) { //number not 0
                    return_val = CPU->R[INSN_Rs(instr)] / number;
                } else { // divide by 0 error
                    return_val = 0;
                }
            }
            break;
        default:
            return_val = 0;
            break;
    }
    
    CPU->regInputVal = return_val;
    CPU->R[CPU->rdMux_CTL] = return_val;
    SetNZP(CPU, return_val);
    CPU->PC++;
}

/*
 * Parses rest of comparative operation and prints out.
 */
void ComparativeOp(MachineState* CPU, FILE* output)
{
    unsigned short instr = CPU->memory[CPU->PC];
    unsigned short subop = (instr >> 7) & 0x3; // bits 7-8 for CMP subopcode
    CPU->NZP_WE = 1; // set cntrl sigs
    short return_val;
    switch (subop) {
        case 0: // CMP
            if ((instr & 0x20) == 0) { // bit 5 = 0: reg
                return_val = CPU->R[INSN_Rs(instr)] - CPU->R[INSN_Rt(instr)]; // compare 2 reg vals
            } else { // bit 5 = 1: immediate
                return_val = CPU->R[INSN_Rs(instr)] - SEXT(INSN_IMM6(instr), 6); // compare reg with imm6
            }
            break;
        case 1: // CMPU
            if ((instr & 0x20) == 0) { //unsigned compare regs
                unsigned short val1 = (unsigned short)CPU->R[INSN_Rs(instr)]; //Rs val
                unsigned short val2 = (unsigned short)CPU->R[INSN_Rt(instr)]; //Rt val
                return_val = val1 - val2; // Rt from Rs
            } else { // unsigned compare with immediate
                unsigned short val1 = (unsigned short)CPU->R[INSN_Rs(instr)];
                unsigned short val2 = INSN_IMM6(instr); // no sign extension for unsigned
                return_val = val1 - val2;
            }
            break;
        case 2: // CMPI
            return_val = CPU->R[INSN_Rs(instr)] - SEXT(INSN_IMM6(instr), 6); // ompare with signed immediate
            break;
        case 3: // CMPIU  
            {
                unsigned short val1 = (unsigned short)CPU->R[INSN_Rs(instr)];
                unsigned short val2 = INSN_IMM6(instr);//unsigned immediate comp
                return_val = val1 - val2; //syb IMM6 from Rs
            }
            break;
        default:
            return_val = 0;
            break;
    }
    SetNZP(CPU, return_val); // only set NZP, no register write
    CPU->PC++;
}

/*
 * Parses rest of logical operation and prints out.
 */
void LogicalOp(MachineState* CPU, FILE* output)
{
    unsigned short instr = CPU->memory[CPU->PC];
    unsigned short subop = (instr >> 3) & 0x7; //bits 3-5 for log subopcode
    CPU->rdMux_CTL = INSN_Rd(instr); //set control sigs
    CPU->regFile_WE = 1;
    CPU->NZP_WE = 1;
    short return_val;
    
    switch (subop) {
        case 0: // AND
            if ((instr & 0x20) == 0) { // bit 5 = 0, reg mode
                return_val = CPU->R[INSN_Rs(instr)] & CPU->R[INSN_Rt(instr)]; // AND two reg vals
            } else { // bit 5 = 1, immediate mode
                return_val = CPU->R[INSN_Rs(instr)] & SEXT(INSN_IMM6(instr), 6); // AND reg with imm6
            }
            break;
        case 1: // NOT
            return_val = ~CPU->R[INSN_Rs(instr)]; // bitwise NOT of register
            break;
        case 2: // OR
            if ((instr & 0x20) == 0) {
                return_val = CPU->R[INSN_Rs(instr)] | CPU->R[INSN_Rt(instr)]; // OR two reg vals
            } else {
                return_val = CPU->R[INSN_Rs(instr)] | SEXT(INSN_IMM6(instr), 6); //OR reg with imm6
            }
            break;
        case 3: // XOR
            if ((instr & 0x20) == 0) {
                return_val = CPU->R[INSN_Rs(instr)] ^ CPU->R[INSN_Rt(instr)]; // XOR two reg vals
            } else {
                return_val = CPU->R[INSN_Rs(instr)] ^ SEXT(INSN_IMM6(instr), 6); // XOR reg with imm6
            }
            break;
        default:
            return_val = 0;
            break;
    }
    
    CPU->regInputVal = return_val;
    CPU->R[CPU->rdMux_CTL] = return_val;
    SetNZP(CPU, return_val);
    CPU->PC++;
}

/*
 * Parses rest of jump operation and prints out.
 */
void JumpOp(MachineState* CPU, FILE* output)
{
    unsigned short instr = CPU->memory[CPU->PC];
    unsigned short subop = (instr >> 11) & 0x1; // bit 11 for JMP vs JMPR
    if (subop == 0) { // JMP - bit 11 = 0
        short offset = SEXT(INSN_IMM11(instr), 11); // sign extend imm11
        CPU->PC = CPU->PC + 1 + offset; // PC = PC + 1 + offset
    } else { // JMPR - bit 11 = 1
        unsigned short base_reg = INSN_Rs(instr); // bits 8-6 for base register
        short offset = SEXT(INSN_IMM6(instr), 6); // sign extend 6-bit offset
        CPU->PC = CPU->R[base_reg] + offset; // PC = Rs + offset
    }
}

/*
 * Parses rest of JSR operation and prints out.
 */
void JSROp(MachineState* CPU, FILE* output)
{
    unsigned short instr = CPU->memory[CPU->PC];
    unsigned short subop = (instr >> 11) & 0x1; // bit 11 for JSR vs JSRR
    
    CPU->regFile_WE = 1;
    CPU->rdMux_CTL = 7; // always write to R7
    CPU->regInputVal = CPU->PC + 1; // return addr
    CPU->R[7] = CPU->regInputVal; //reg 7 is where we will store our return val
    CPU->NZP_WE = 1;
    SetNZP(CPU, CPU->regInputVal);
    
    if (subop == 0) { // JSR - bit 11 = 0
        short offset = SEXT(INSN_IMM11(instr), 11); //sign extend 11-bit offset
        CPU->PC = CPU->PC + 1 + offset; // PC = PC + 1 + offset
    } else { // JSRR - bit 11 = 1
        unsigned short base_reg = INSN_Rs(instr); // Rs
        short offset = SEXT(INSN_IMM6(instr), 6); // sign extend 6-bit offset
        CPU->PC = CPU->R[base_reg] + offset; // PC = Rs + offset
    }
}

/*
 * Parses rest of shift/mod operations and prints out.
 */
void ShiftModOp(MachineState* CPU, FILE* output)
{
    unsigned short instr = CPU->memory[CPU->PC];
    unsigned short subop = (instr >> 4) & 0x3; // bits 4-5 for shift/mod subopcode
    CPU->rdMux_CTL = INSN_Rd(instr);//set control sigs
    CPU->regFile_WE = 1;
    CPU->NZP_WE = 1;
    short return_val;
    unsigned short shift_amt = instr & 0xF;// bits 0-3 for shift amount
    
    switch (subop) {
        case 0: // SLL
            return_val = CPU->R[INSN_Rs(instr)] << shift_amt; // shift left by shift_amt
            break;
        case 1: // SRA
            return_val = CPU->R[INSN_Rs(instr)] >> shift_amt; // arith right shift
            break;
        case 2: // SRL
            {
                unsigned short temp = (unsigned short)CPU->R[INSN_Rs(instr)];
                temp = temp >> shift_amt;//logical right shift
                return_val = (short)temp;
            }
            break;
        case 3: // MOD
            if (CPU->R[INSN_Rt(instr)] != 0) { // no mod by zero
                return_val = CPU->R[INSN_Rs(instr)] % CPU->R[INSN_Rt(instr)]; // Rs mod Rt
            } else { // mod by 0 error
                return_val = 0;
            }
            break;
        default:
            return_val = 0;
            break;
    }
    CPU->regInputVal = return_val;
    CPU->R[CPU->rdMux_CTL] = return_val;
    SetNZP(CPU, return_val);
    CPU->PC++;
}

/*
 * Set the NZP bits in the PSR.
 */
void SetNZP(MachineState* CPU, short result)
{
    CPU->PSR &= 0xFFF8; //remove existing NZP val
    if (result > 0) {
        CPU->PSR |= 0x0001; // Set P bit (bit 0)
        CPU->NZPVal = 1; //NZP Pos #001
    } else if (result == 0) {
        CPU->PSR |= 0x0002; // Set Z bit (bit 1)
        CPU->NZPVal = 2; //NZP Zero #010
    } else {
        CPU->PSR |= 0x0004; // Set N bit (bit 2)
        CPU->NZPVal = 4; //NZP Neg #100
    }
}
