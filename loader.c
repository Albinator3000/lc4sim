/*
 * loader.c : Defines loader functions for opening and loading object files
 */

#include "loader.h"

// memory array location
unsigned short memoryAddress;


unsigned short convert_endianness(unsigned short x) { //measure the endianness
   	unsigned short smaller_bits = (x & 0x00FF) << 8; 
	 	unsigned short larger_bits = (x & 0xFF00) >> 8; 
    unsigned short complete_string = larger_bits | smaller_bits;
    return complete_string;
}

/* 
 * Read an object file and modify the machine state as described in the writeup
 */
int ReadObjectFile(char* filename, MachineState* CPU)
{
  FILE* file = fopen(filename, "rb"); // opens the file
	if (file == NULL) { // if no file, throw error
			printf("error 1: no file \n");
			return -1;
	}
	printf("File opened successfully: %s\n", filename);

	unsigned short section_type, address, count, data; // variables for reading file sections

	while (fread(&section_type, sizeof(unsigned short), 1, file) == 1) { // parse the various file headers:
	// 0xCADE, 0xDADA, 0xC3B7, 0xF17E, 0x715E
			printf("Read section type: 0x%04X (before endianness)\n", section_type);
			section_type = convert_endianness(section_type);
			printf("Section type after conversion: 0x%04X\n", section_type);
			section_type = convert_endianness(section_type); // set endianness for header

			switch (section_type) { // case/break for header, loop through each section
			case 0xCADE: // CODE
			{
				if (fread(&address, sizeof(unsigned short), 1, file) != 1) { // read addr where code should be loaded
						fclose(file);
						printf("error 2: code adddr not loaded \n");
						return -1;
				}
				address = convert_endianness(address); // addr in obj file w/ correct endianness

				if (fread(&count, sizeof(unsigned short), 1, file) != 1) { // get number of instrs
						fclose(file);
						printf("error 3: code instructions not loaded \n");
						return -1;
				}
				count = convert_endianness(count); // our number of instrs

				
				for (int i = 0; i < count; i++) { //get each instr and store in CPU mem
						if (fread(&data, sizeof(unsigned short), 1, file) != 1) {
								fclose(file);
								printf("error 4: instruction %d of %d in code section \n", i+1, count);
								return -1;
						}

						CPU->memory[address + i] = convert_endianness(data); // populate mem with the CODE data at the given addr
				}
				printf("Processing %s section at address 0x%04X with %d items\n", 
       	section_type == 0xCADE ? "CODE" : "DATA", address, count);
				break;
			}

			case 0xDADA: // DATA
			{
				if (fread(&address, sizeof(unsigned short), 1, file) != 1) { // read addr where data should be loaded
					fclose(file);
					printf("error 5: data addr not loaded \n");
					return -1;
				}
				address = convert_endianness(address); // addr in obj file w/ correct endianness

				if (fread(&count, sizeof(unsigned short), 1, file) != 1) { // get number of data values
					fclose(file);
					printf("error 6: data values not loaded \n");
					return -1;
				}
				count = convert_endianness(count); // our number of data values

				for (int i = 0; i < count; i++) { //get each data value and store in CPU mem
					if (fread(&data, sizeof(unsigned short), 1, file) != 1) {
						fclose(file);
						printf("error 7: data value %d of %d in data section \n", i+1, count);
						return -1;
					}
					CPU->memory[address + i] = convert_endianness(data); // populate mem with the DATA values at the given addr
				}
				printf("Processing %s section at address 0x%04X with %d items\n", 
       	section_type == 0xDADA ? "CODE" : "DATA", address, count);
				break;
			}

			case 0xC3B7: // SYMBOL
			{
				if (fread(&address, sizeof(unsigned short), 1, file) != 1) { // read addr (part of header but not used)
					fclose(file);
					printf("error 8: symbol addr not loaded \n");
					return -1;
				}
				address = convert_endianness(address);

				if (fread(&count, sizeof(unsigned short), 1, file) != 1) { // get length of symbol string
					fclose(file);
					printf("error 9: symbol string length not loaded \n");
					return -1;
				}
				count = convert_endianness(count); // our symbol string length

				fseek(file, count, SEEK_CUR); // skip symbol data (not needed for this assignment)

				break;
			}

			case 0xF17E: // FILENAME
			{
				if (fread(&count, sizeof(unsigned short), 1, file) != 1) { // get length of filename string
					fclose(file);
					printf("error 10: filename string length not loaded \n");
					return -1;
				}
				count = convert_endianness(count); // our filename string length

				fseek(file, count, SEEK_CUR);
				break;
			}

			case 0x715E: // LINE NUMBER
			{
				unsigned short line_num, file_index; // read rest of header (addr, line number, file index)
				if (fread(&address, sizeof(unsigned short), 1, file) != 1 ||
				    fread(&line_num, sizeof(unsigned short), 1, file) != 1 ||
				    fread(&file_index, sizeof(unsigned short), 1, file) != 1) {
					fclose(file);
					printf("error 11: line number header not loaded \n");
					return -1;
				}
				 address = convert_endianness(address);
				 line_num = convert_endianness(line_num);
				 file_index = convert_endianness(file_index);
				
				break;
			}

			default:
				fclose(file); // unknown section type - should not happen with valid files
				printf("error 12: unknown section type 0x%04X \n", section_type);
				return -1;
		}
	}

	fclose(file); // close the file
  return 0; // success
}
