/*
 * LC4.c: Defines simulator functions for executing instructions
 */

#include "LC4.h"
#include <stdio.h>

/*
 * Reset the machine state as Pennsim would do
 */
void Reset(MachineState* CPU)
{
	// Set PC
    CPU->PC = 0x8200;
    
    // Clear PSR
    CPU->PSR = 0;
    
    // Clear REGs
    for (int i = 0; i < 8; i++) {
        CPU->R[i] = 0;
    }
    
    // Clear CTRLSIGs
    ClearSignals(CPU);
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

}


/*
 * This function should execute one LC4 datapath cycle.
 */
int UpdateMachineState(MachineState* CPU, FILE* output)
{
    
    return 0;
}



//////////////// PARSING HELPER FUNCTIONS ///////////////////////////



/*
 * Parses rest of branch operation and updates state of machine.
 */
void BranchOp(MachineState* CPU, FILE* output)
{
    
}

/*
 * Parses rest of arithmetic operation and prints out.
 */
void ArithmeticOp(MachineState* CPU, FILE* output)
{
    
}

/*
 * Parses rest of comparative operation and prints out.
 */
void ComparativeOp(MachineState* CPU, FILE* output)
{
    
}

/*
 * Parses rest of logical operation and prints out.
 */
void LogicalOp(MachineState* CPU, FILE* output)
{
    
}

/*
 * Parses rest of jump operation and prints out.
 */
void JumpOp(MachineState* CPU, FILE* output)
{
    
}

/*
 * Parses rest of JSR operation and prints out.
 */
void JSROp(MachineState* CPU, FILE* output)
{
    
}

/*
 * Parses rest of shift/mod operations and prints out.
 */
void ShiftModOp(MachineState* CPU, FILE* output)
{
    
}

/*
 * Set the NZP bits in the PSR.
 */
void SetNZP(MachineState* CPU, short result)
{
    
}

