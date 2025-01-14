/*
 * CS 261: Main driver
 *
 * Name: Aiden Smith
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <inttypes.h>  // For PRIx64
#include <stdbool.h>

#include "p1-check.h"
#include "p2-load.h"
#include "p3-disas.h"
#include "p4-interp.h"

/*
 * helper function for printing help text
 */
void usage (char **argv)
{
    printf("Usage: %s <option(s)> mini-elf-file\n", argv[0]);
    printf(" Options are:\n");
    printf("  -h      Display usage\n");
    printf("  -H      Show the Mini-ELF header\n");
    printf("  -a      Show all with brief memory\n");
    printf("  -f      Show all with full memory\n");
    printf("  -s      Show the program headers\n");
    printf("  -m      Show the memory contents (brief)\n");
    printf("  -M      Show the memory contents (full)\n");
    printf("  -d      Disassemble code contents\n");
    printf("  -D      Disassemble data contents\n");
    printf("  -e      Execute program\n");
    printf("  -E      Execute program (trace mode)\n");
}

int main (int argc, char **argv)
{
    int opt;
    int show_header = 0;
    int show_phdrs = 0;
    int disas_code = 0;
    int disas_data = 0;
    int show_mem = 0;
    int exec_mode = 0; // 0: no execution, 1: execute, 2: trace mode
    int full_mem = 0;

    /* Parse command-line arguments */
    while ((opt = getopt(argc, argv, "hHsmdDMafeE")) != -1) {
        switch (opt) {
            case 'h':
                usage(argv);
                return EXIT_SUCCESS;
            case 'H':
                show_header = 1;
                break;
            case 's':
                show_phdrs = 1;
                break;
            case 'd':
                disas_code = 1;
                break;
            case 'D':
                disas_data = 1;
                break;
            case 'm':
                show_mem = 1;
                break;
            case 'M':
                show_mem = 1;
                full_mem = 1;
                break;
            case 'a':
                show_header = 1;
                show_phdrs = 1;
                show_mem = 1;
                disas_code = 1;
                disas_data = 1;
                break;
            case 'f':
                show_header = 1;
                show_phdrs = 1;
                show_mem = 1;
                full_mem = 1;
                disas_code = 1;
                disas_data = 1;
                break;
            case 'e':
                if (exec_mode == 2) {
                    usage(argv);
                    return EXIT_FAILURE;
                }
                exec_mode = 1;
                break;
            case 'E':
                if (exec_mode == 1) {
                    usage(argv);
                    return EXIT_FAILURE;
                }
                exec_mode = 2;
                break;
            case '?':
                usage(argv);
                return EXIT_FAILURE;
            default:
                usage(argv);
                return EXIT_FAILURE;
        }
    }

    if (optind >= argc) {
        usage(argv);
        return EXIT_FAILURE;
    }

    /* Open the file */
    const char *filename = argv[optind];
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Failed to read file\n");
        return EXIT_FAILURE;
    }

    elf_hdr_t hdr;
    if (!read_header(file, &hdr)) {
        printf("Failed to read file\n");
        fclose(file);
        return EXIT_FAILURE;
    }

    /* Show the header */
    if (show_header) {
        dump_header(&hdr);
    }

    /* Allocate memory for program headers */
    elf_phdr_t *phdrs = malloc(hdr.e_num_phdr * sizeof(elf_phdr_t));
    if (!phdrs) {
        fclose(file);
        return EXIT_FAILURE;
    }

    /* Read program headers */
    for (int i = 0; i < hdr.e_num_phdr; i++) {
        uint16_t offset = hdr.e_phdr_start + i * sizeof(elf_phdr_t);
        if (!read_phdr(file, offset, &phdrs[i])) {
            printf("Failed to read file\n"); // Bad header
            free(phdrs);
            fclose(file);
            return EXIT_FAILURE;
        }
    }

    /* Show program headers */
    if (show_phdrs) {
        dump_phdrs(hdr.e_num_phdr, phdrs);
    }

    /* Allocate memory space */
    byte_t memory[MEMSIZE] = {0};

    /* Load segments into memory */
    for (int i = 0; i < hdr.e_num_phdr; i++) {
        if (!load_segment(file, memory, &phdrs[i])) {
            free(phdrs);
            fclose(file);
            return EXIT_FAILURE;
        }
    }

    /* Display memory contents */
    if (show_mem && exec_mode != 2) { // Do not dump memory here if in trace mode
        uint16_t start = 0;
        uint16_t end = MEMSIZE - 1;
        if (!full_mem) {
            // Find the range of used memory
            start = MEMSIZE - 1;
            end = 0;
            for (int i = 0; i < hdr.e_num_phdr; i++) {
                elf_phdr_t *phdr = &phdrs[i];
                uint16_t seg_start = phdr->p_vaddr;
                uint16_t seg_end = seg_start + phdr->p_size;
                if (seg_start < start) start = seg_start;
                if (seg_end > end) end = seg_end;
            }
        }
        printf("Memory contents from %04x to %04x:\n", start, end);
        dump_memory(memory, start, end);
    }

    /* Disassemble code segments */
    if (disas_code) {
        int print_flag = 0;
        for (int i = 0; i < hdr.e_num_phdr; i++) {
            elf_phdr_t *phdr = &phdrs[i];
            if (phdr->p_type == CODE) {
                if (!print_flag) {
                    printf("Disassembly of executable contents:\n");
                    print_flag = 1;
                }
                disassemble_code(memory, phdr, &hdr);
                printf("\n"); // Between segments
            }
        }
    }

    /* Disassemble data segments */
    if (disas_data) {
        int print_flag = 0;
        for (int i = 0; i < hdr.e_num_phdr; i++) {
            elf_phdr_t *phdr = &phdrs[i];
            if (phdr->p_type == DATA) {
                if (!print_flag) {
                    printf("Disassembly of data contents:\n");
                    print_flag = 1;
                }
                if (phdr->p_flags == 6) {
                    disassemble_data(memory, phdr);
                    printf("\n");
                } else if (phdr->p_flags == 4) {
                    disassemble_rodata(memory, phdr);
                    printf("\n");
                }
            }
        }
    }


    // p4444
    bool first = true;
    if (exec_mode > 0) {
        /* Initialize CPU */
        y86_t cpu = {0};
        cpu.pc = hdr.e_entry;
        cpu.stat = AOK;
        if (first) {
            printf("Beginning execution at 0x%04x\n", hdr.e_entry);

            if (exec_mode == 2) {
                printf("Y86 CPU state:\n");
                dump_cpu_state(&cpu);
                first = false;
            }
        }
        int instruction_count = 0;

        while (cpu.stat == AOK || cpu.stat == HLT) {
            /* Fetch instruction */
            y86_inst_t inst = fetch(&cpu, memory);

            if (cpu.stat == ADR || cpu.stat == INS) {
                break;
            }

            instruction_count++;

            /* Decode and execute */
            bool cnd = false;
            y86_reg_t valA = 0;
            y86_reg_t valE = decode_execute(&cpu, &inst, &cnd, &valA);

            if (cpu.stat == ADR || cpu.stat == INS) {
                break;
            }

            if (first == false) {
                printf("\n");
            }
            /* Memory access, write-back, and PC update */
            memory_wb_pc(&cpu, &inst, memory, cnd, valA, valE);

            if (exec_mode == 2) {
                printf("Executing: ");
                disassemble(&inst);
                printf("\n");
                printf("Y86 CPU state:\n");
                dump_cpu_state(&cpu);                
            }

            if (cpu.stat == HLT || cpu.stat != AOK) {
                break; // Exit loop when halt is encountered
            }
        }

        if (exec_mode != 2) {
            /* Print final CPU state */
            printf("Y86 CPU state:\n");
            dump_cpu_state(&cpu);
        }

        printf("Total execution count: %d\n", instruction_count);

        if (first == false) {
                printf("\n");
        }

        if (exec_mode == 2) {
            /* Trace mode: dump memory contents */
            dump_memory(memory, 0, MEMSIZE);
        }
    }

    /* Clean up */
    free(phdrs);
    fclose(file);
    return EXIT_SUCCESS;
}