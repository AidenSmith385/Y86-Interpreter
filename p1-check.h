#ifndef __CS261_P1__
#define __CS261_P1__

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "elf.h"

/**
 * @brief Load a Mini-ELF header from an open file stream
 *
 * @param file File stream to use for input
 * @param hdr Pointer to region where the Mini-ELF header should be loaded
 * @returns True if the header was successfully loaded and verified, false otherwise
 */
bool read_header (FILE *file, elf_hdr_t *hdr);

/**
 * @brief Print Mini-ELF header information to standard out
 *
 * @param hdr Header with info to print
 */
void dump_header (elf_hdr_t *hdr);

#endif
