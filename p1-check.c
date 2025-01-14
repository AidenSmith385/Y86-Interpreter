/*
 * CS 261 PA1: Mini-ELF header verifier
 *
 * Name: Aiden Smith
 */

#include "p1-check.h"

/**********************************************************************
 *                         REQUIRED FUNCTIONS
 *********************************************************************/

/**
 * Read and validate the Mini-ELF header.
 *
 * @param file file pointer to read the header
 * @param hdr  elf_hdr_t pointer to struct
 * @return true if the header was a success
 */
bool read_header (FILE *file, elf_hdr_t *hdr)
{

    if (file == NULL || hdr == NULL) {
        return false;  // invalid
    }
    // if the file is large enough to contain the Mini-ELF header
    if (fseek(file, 0, SEEK_END) != 0) {
        return false;
    }

    // rewind pointer to beginning
    rewind(file);

    if (fread(hdr, sizeof(elf_hdr_t), 1, file) != 1) {
        return false;  // error
    }

    uint32_t themagic = hdr->magic;
    #define magicnum 0x00464C45 // expected num
    if (themagic != magicnum) { // validate the magic number
        return false;  // invalid
    }

    return true; // valid
}

/**********************************************************************
 *                         OPTIONAL FUNCTIONS
 *********************************************************************/

/**
 * Prints hex and structured output of mini-elf header.
 *
 * @param hdr Pointer to structure containing Mini-ELF header
 */
void dump_header (elf_hdr_t *hdr)
{
    // print first 16 bytes in hex in specified format
    uint8_t *bytes = (uint8_t *)hdr; // header to a byte array
    for (int i = 0; i < 16; i++) {
        if (i == 8) {
            printf(" ");
        }
        if (i != 15) {
            printf("%02x ", bytes[i]);
        } else {
            printf("%02x", bytes[i]);
        }
    }
    printf("\n");

    // Version
    printf("Mini-ELF version %d\n", hdr->e_version);

    // Entry point
    printf("Entry point 0x%x\n", hdr->e_entry);

    // Program headers
    printf("There are %d program headers, starting at offset %d (0x%x)\n",
           hdr->e_num_phdr, hdr->e_phdr_start, hdr->e_phdr_start);

    // Symbol table
    if (hdr->e_symtab) {
        printf("There is a symbol table starting at offset %d (0x%x)\n",
            hdr->e_symtab, hdr->e_symtab);
    } else {
        printf("There is no symbol table present\n");
    }

    // String table
    if (hdr->e_strtab) {
        printf("There is a string table starting at offset %d (0x%x)\n",
            hdr->e_strtab, hdr->e_strtab);
    } else {
        printf("There is no string table present\n");
    }
}

