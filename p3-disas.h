#ifndef __CS261_P3__
#define __CS261_P3__

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "elf.h"
#include "y86.h"

/**
 * @brief Load a Y86 instruction from memory
 *
 * @param cpu Pointer to Y86 CPU structure with the PC address to be loaded
 * @param memory Pointer to the beginning of the Y86 address space
 * @returns Populated Y86 instruction structure
 */
y86_inst_t fetch (y86_t *cpu, byte_t *memory);

/**
 * @brief Print the disassembly of a Y86 instruction to standard out
 *
 * @param inst Pointer to Y86 instruction structure to be printed
 */
void disassemble (y86_inst_t *inst);

/**
 * @brief Print the disassembly of a Y86 code segment
 *
 * @param memory Pointer to the beginning of the Y86 address space
 * @param phdr Program header of segment to be printed
 * @param hdr File header (needed to detect the entry point)
 */
void disassemble_code   (byte_t *memory, elf_phdr_t *phdr, elf_hdr_t *hdr);

/**
 * @brief Print the disassembly of a Y86 read/write data segment
 *
 * @param memory Pointer to the beginning of the Y86 address space
 * @param phdr Program header of segment to be printed
 */
void disassemble_data   (byte_t *memory, elf_phdr_t *phdr);

/**
 * @brief Print the disassembly of a Y86 read-only data segment
 *
 * @param memory Pointer to the beginning of the Y86 address space
 * @param phdr Program header of segment to be printed
 */
void disassemble_rodata (byte_t *memory, elf_phdr_t *phdr);

#endif
