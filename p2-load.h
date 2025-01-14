#ifndef __CS261_P2__
#define __CS261_P2__

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "elf.h"
#include "y86.h"

/**
 * @brief Load a Mini-ELF program header from an open file stream
 *
 * @param file File stream to use for input
 * @param offset Byte offset in file where the program header is located
 * @param phdr Pointer to memory where the Mini-ELF program header should be loaded
 * @returns True if the header was successfully loaded and verified, false otherwise
 */
bool read_phdr (FILE *file, uint16_t offset, elf_phdr_t *phdr);

/**
 * @brief Load a Mini-ELF program segment from an open file stream
 *
 * @param file File stream to use for input
 * @param memory Pointer to the beginning of the Y86 address space into which
 * the segment should be loaded
 * @param phdr Pointer to the program header for the segment that should be loaded
 * @returns True if the segment was successfully loaded, false otherwise
 */
bool load_segment (FILE *file, byte_t *memory, elf_phdr_t *phdr);

/**
 * @brief Print Mini-ELF program header information to standard out
 *
 * @param numphdrs Number of program headers to print
 * @param phdrs Pointer to array of program headers with info to print
 */
void dump_phdrs (uint16_t numphdrs, elf_phdr_t *phdrs);

/**
 * @brief Print a portion of a Y86 address space
 *
 * @param memory Pointer to the beginning of the Y86 address space
 * @param start Byte offset where printing should begin
 * @param end Byte offset where printing should end
 */
void dump_memory (byte_t *memory, uint16_t start, uint16_t end);

#endif
