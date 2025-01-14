/*
 * CS 261 PA2: Mini-ELF loader
 *
 * Name: Aiden Smith
 */

#include "p2-load.h"

/**********************************************************************
 *                         REQUIRED FUNCTIONS
 *********************************************************************/

/*
 * Read a Mini-ELF header from the file into phdr.
 * Failure or header invalid, return false.
 */
bool read_phdr (FILE *file, uint16_t offset, elf_phdr_t *phdr)
{
    // parameters are valid
    if (file == NULL || phdr == NULL) {
        return false;
    }
    if (fseek(file, offset, SEEK_SET) != 0) {
        return false; // couldn't seek to correct location
    }

    // header from the file into phdr struct
    if (fread(phdr, sizeof(elf_phdr_t), 1, file) != 1) {
        return false; // Failed to read the header
    }

    // magic number
    if (phdr->magic != 0xDEADBEEF) {
        return false; // Invalid magic number
    }
    return true;
}


/*
 * Read data from file into memory, based on the header.
 * Returns false if segment is invalid, unsupported,
 * or writes past the end.
 */
bool load_segment (FILE *file, byte_t *memory, elf_phdr_t *phdr)
{
    // parameter
    if (file == NULL || memory == NULL || phdr == NULL) {
        return false;
    }
    if (phdr->p_size == 0 || phdr->p_offset < 0) {
        return true; // no data to load
    }

    // segment may write past the memory
    if (phdr->p_vaddr + phdr->p_size > MEMSIZE) {
        return false;
    }

    // Seek to the segment's offset in the file
    if (fseek(file, phdr->p_offset, SEEK_SET) != 0) {
        return false; // failed
    }

    // Read data into the virtual memory
    if (fread(&memory[phdr->p_vaddr], sizeof(byte_t), phdr->p_size, file) != phdr->p_size) {
        return false;
    }


    return true;
}

/**********************************************************************
 *                         OPTIONAL FUNCTIONS
 *********************************************************************/

/*
 * Print the Mini-ELF headers passed from phdrs array. The output should have offset,
 * size, virtual address, type, and flags.
 */
void dump_phdrs (uint16_t numphdrs, elf_phdr_t *phdrs)
{
    if (phdrs == NULL) {
        return; // nothing
    }

    // program headers table
    printf(" Segment   Offset    Size      VirtAddr  Type      Flags\n");

    // iterate and print the details
    for (uint16_t i = 0; i < numphdrs; i++) {

        elf_phdr_t *phdr = &phdrs[i];

        const char *str;
        switch (phdr->p_type) {
            case CODE:
                str = "CODE";
                break;
            case DATA:
                str = "DATA";
                break;
            case STACK:
                str = "STACK";
                break;
            case HEAP:
                str = "HEAP";
                break;
            default:
                str = "UNKNOWN";
                break;
        }

        // permission flags R, W, or X
        char flags[4] = {' ', ' ', ' ', '\0'};
        if (phdr->p_flags & 0x4) flags[0] = 'R';
        if (phdr->p_flags & 0x2) flags[1] = 'W';
        if (phdr->p_flags & 0x1) flags[2] = 'X';

        // the formatted info
        printf("  %02u       0x%04x    0x%04x    0x%04x    %-8s  %s\n",
               i, phdr->p_offset, phdr->p_size, phdr->p_vaddr, str, flags);
    }
}

/*
 * Print contents of virtual memory from start to end.
 * 16 byte alignment and only output requested byte.
 * If not aligned bytes should be printed as empty spaces.
 */
void dump_memory(byte_t *memory, uint16_t start, uint16_t end) {
    
    // 4 digit hexadecimal format
    printf("Contents of memory from %04x to %04x:\n", start, end);

    if (memory == NULL || start >= end || end > MEMSIZE) {
        return; // parameter check
    }

    // Loop through range
    for (uint16_t addr = start; addr < end; addr += 16) {

        // avoid trailing spaces
        bool fb = true;
        printf("  %04x  ", addr);
        // Determine the last byte in the current line that should be printed
        uint16_t line_end = (addr + 16 < end) ? addr + 16 : end;

        // Print the memory. MUST ALIGN
        for (uint16_t i = 0; i < 16; i++) {
            uint16_t current_addr = addr + i;

            if (current_addr < start || current_addr >= line_end) {
                continue;
            }

            // pring space before if not the first byte
            if (!fb) {
                printf(" ");
            } else {
                fb = false;
            }

            // byte in hexadecimal
            printf("%02x", memory[current_addr]);

            // allignment
            if (i == 7 && (addr + 8) < line_end) {
                printf(" ");
            }
        }
        printf("\n");
    }
}
