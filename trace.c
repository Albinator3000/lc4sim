/*
 * trace.c: location of main() to start the simulator
 */

#include "loader.h"

// Global Vars for CPU State
MachineState CPU_STATE;  // set machine state of CPU
MachineState* CPU = &CPU_STATE;  // pointer holding machine state addr

int main(int argc, char** argv)
{
    if (argc < 3) { // need at least 3 args: prog name, output file, and obj input file
        printf("error1 : need prog name, output file and obj file \n");
        return -1;
    }
    
    memset(CPU->memory, 0, sizeof(CPU->memory)); // set CPU mem to 0
    Reset(CPU); // change PC to 0x8200 and empties reg vals
    
    for (int i = 2; i < argc; i++) { // confirm all obj files are there
        FILE* test_file = fopen(argv[i], "rb");
        if (test_file == NULL) {
            printf("Error: Cannot open file %s\n", argv[i]);
            return -1;
        }
        fclose(test_file);
    }
    
    for (int i = 2; i < argc; i++) { //put obj files in mEm
        if (ReadObjectFile(argv[i], CPU) != 0) {
            printf("Error: Failed to read object file %s\n", argv[i]);
            return -1;  // No free() needed!
        }
    }
    

    FILE* output_file = fopen(argv[1], "w"); // get output file
    if (output_file == NULL) {
        printf("Error: Cannot create output file %s\n", argv[1]);
        return -1;  // No free() needed!
    }
    
   
    printf("Memory contents loaded:\n"); //print memory
    for (int i = 0; i < 65536; i++) {
			if (CPU->memory[i] != 0) {
					printf("address: %05X contents: 0x%04X\n", i, CPU->memory[i]);
			}
    }
    
    fclose(output_file); // close the file
    
    return 0;
}
