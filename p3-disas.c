/*
 * CS 261 PA3: Mini-ELF disassembler
 *
 * Name: Aiden Smith
 */

#include "p3-disas.h"

/**********************************************************************
 *                         REQUIRED FUNCTIONS
 *********************************************************************/

y86_inst_t fetch (y86_t *cpu, byte_t *memory)
{
    y86_inst_t inst;

    // check NULL pointers
    if (cpu == NULL || memory == NULL) {
        inst.icode = INVALID;
        return inst;
    }

    // the program counter and valP
    address_t pc = cpu->pc;
    inst.valP = pc;

    // Check bounds
    if (pc >= MEMSIZE) {
        cpu->stat = ADR;
        inst.icode = INVALID;
        return inst;
    }

    // Fetch the opcode
    byte_t opcode = memory[pc];
    inst.valP++; // after reading opcode

    // split
    inst.icode = opcode >> 4; // High 4 bits
    inst.ifun.b = opcode & 0x0F; // Low 4 bits

    // instruction based on icode
    switch (inst.icode) {
        case HALT:
            if (inst.ifun.b != 0) {
                cpu->stat = INS; // Invalid
                inst.icode = INVALID;
                return inst;
            }
            inst.icode = HALT;
            break;

        case NOP:
            if (inst.ifun.b != 0) {
                cpu->stat = INS;
                inst.icode = INVALID;
                return inst;
            }
            inst.icode = NOP;
            break;

        case CMOV:
            if (inst.ifun.cmov < RRMOVQ || inst.ifun.cmov > CMOVG) {
                cpu->stat = INS;
                inst.icode = INVALID;
                return inst;
            }
            // Read register
            if (inst.valP >= MEMSIZE) {
                cpu->stat = ADR;
                inst.icode = INVALID;
                return inst;
            }
            {
                byte_t reg_byte = memory[inst.valP];
                inst.valP++;
                inst.ra = reg_byte >> 4;
                inst.rb = reg_byte & 0x0F;

                // Validate register numbers
                if (inst.ra > R14 || inst.rb > R14) {
                    cpu->stat = INS;
                    inst.icode = INVALID;
                    return inst;
                }
            }
            inst.icode = CMOV;
            break;

        case IRMOVQ:
            if (inst.ifun.b != 0) {
                cpu->stat = INS;
                inst.icode = INVALID;
                return inst;
            }
            // Read register
            if (inst.valP >= MEMSIZE) {
                cpu->stat = ADR;
                inst.icode = INVALID;
                return inst;
            }
            {
                byte_t reg_byte = memory[inst.valP];
                inst.valP++;
                inst.ra = reg_byte >> 4;
                inst.rb = reg_byte & 0x0F;

                // Validate registers
                if (inst.ra != NOREG || inst.rb > R14) {
                    cpu->stat = INS;
                    inst.icode = INVALID;
                    return inst;
                }

                // Read 8-byte value
                if (inst.valP + 8 > MEMSIZE) {
                    cpu->stat = ADR;
                    inst.icode = INVALID;
                    return inst;
                }
                uint64_t valC = 0;
                memcpy(&valC, &memory[inst.valP], 8);
                inst.valC.v = (int64_t)valC;
                inst.valP += 8;
            }
            inst.icode = IRMOVQ;
            break;

        case RMMOVQ:
        case MRMOVQ:
            if (inst.ifun.b != 0) {
                cpu->stat = INS;
                inst.icode = INVALID;
                return inst;
            }
            // Read register
            if (inst.valP >= MEMSIZE) {
                cpu->stat = ADR;
                inst.icode = INVALID;
                return inst;
            }
            {
                byte_t reg_byte = memory[inst.valP];
                inst.valP++;
                inst.ra = reg_byte >> 4;
                inst.rb = reg_byte & 0x0F;

                // Validate registers
                if (inst.ra > R14 || inst.rb > NOREG) {
                    cpu->stat = INS;
                    inst.icode = INVALID;
                    return inst;
                }

                // Read 8-byte displacement
                if (inst.valP + 8 > MEMSIZE) {
                    cpu->stat = ADR;
                    inst.icode = INVALID;
                    return inst;
                }
                uint64_t valC = 0;
                memcpy(&valC, &memory[inst.valP], 8);
                inst.valC.d = (int64_t)valC;
                inst.valP += 8;
            }
            inst.icode = inst.icode;  // RMMOVQ or MRMOVQ
            break;

        case OPQ:
            // Validate OPQ
            if (inst.ifun.op < ADD || inst.ifun.op > XOR) {
                cpu->stat = INS;
                inst.icode = INVALID;
                return inst;
            }
            // Read register
            if (inst.valP >= MEMSIZE) {
                cpu->stat = ADR;
                inst.icode = INVALID;
                return inst;
            }
            {
                byte_t reg_byte = memory[inst.valP];
                inst.valP++;
                inst.ra = reg_byte >> 4;
                inst.rb = reg_byte & 0x0F;

                // Validate registers
                if (inst.ra > R14 || inst.rb > R14) {
                    cpu->stat = INS;
                    inst.icode = INVALID;
                    return inst;
                }
            }
            inst.icode = OPQ;
            break;

        case JUMP:
            // Validate JUMP
            if (inst.ifun.jump < JMP || inst.ifun.jump > JG) {
                cpu->stat = INS;
                inst.icode = INVALID;
                return inst;
            }
            // 8-byte address
            if (inst.valP + 8 > MEMSIZE) {
                cpu->stat = ADR;
                inst.icode = INVALID;
                return inst;
            }
            {
                uint64_t dest = 0;
                memcpy(&dest, &memory[inst.valP], 8);
                inst.valC.dest = dest;
                inst.valP += 8;
            }
            inst.icode = JUMP;
            break;

        case CALL:
            if (inst.ifun.b != 0) {
                cpu->stat = INS;
                inst.icode = INVALID;
                return inst;
            }
            if (inst.valP + 8 > MEMSIZE) {
                cpu->stat = ADR;
                inst.icode = INVALID;
                return inst;
            }
            {
                uint64_t dest = 0;
                memcpy(&dest, &memory[inst.valP], 8);
                inst.valC.dest = dest;
                inst.valP += 8;
            }
            inst.icode = CALL;
            break;

        case RET:
            if (inst.ifun.b != 0) {
                cpu->stat = INS;
                inst.icode = INVALID;
                return inst;
            }
            inst.icode = RET;
            break;

        case PUSHQ:
        case POPQ:
            if (inst.ifun.b != 0) {
                cpu->stat = INS;
                inst.icode = INVALID;
                return inst;
            }
            // Read register byte
            if (inst.valP >= MEMSIZE) {
                cpu->stat = ADR;
                inst.icode = INVALID;
                return inst;
            }
            {
                byte_t reg_byte = memory[inst.valP];
                inst.valP++;
                inst.ra = reg_byte >> 4;
                inst.rb = reg_byte & 0x0F;

                // rb should be NOREG
                if (inst.ra > R14 || inst.rb != NOREG) {
                    cpu->stat = INS;
                    inst.icode = INVALID;
                    return inst;
                }
            }
            inst.icode = inst.icode;  // PUSHQ or POPQ
            break;

        case IOTRAP:
            if (inst.ifun.b > FLUSH) {
                cpu->stat = INS;
                inst.icode = INVALID;
                return inst;
            }
            inst.icode = IOTRAP;
            break;
        default:
            // Invalid
            cpu->stat = INS;
            inst.icode = INVALID;
            return inst;
    }
    cpu->stat = AOK;  // CPU is AOK
    return inst; // successful

}

/**********************************************************************
 *                         OPTIONAL FUNCTIONS
 *********************************************************************/

void disassemble (y86_inst_t *inst)
{
    // numbers to registers
    const char *reg_names[16] = {
        "%rax", "%rcx", "%rdx", "%rbx",
        "%rsp", "%rbp", "%rsi", "%rdi",
        "%r8",  "%r9",  "%r10", "%r11",
        "%r12", "%r13", "%r14", "NOREG"
    };

    // names for OPq
    const char *opq_names[4] = {
        "addq", "subq", "andq", "xorq"
    };

    const char *cmov_names[] = {
        "rrmovq", // 0
        "cmovle", // 1
        "cmovl",  // 2
        "cmove",  // 3
        "cmovne", // 4
        "cmovge", // 5
        "cmovg"   // 6
    };

    const char *jump_names[] = {
        "jmp",    // 0
        "jle",    // 1
        "jl",     // 2
        "je",     // 3
        "jne",    // 4
        "jge",    // 5
        "jg"      // 6
    };

    switch (inst->icode) {
        case HALT:
            printf("halt");
            break;

        case NOP:
            printf("nop");
            break;

        case CMOV:
            if (inst->ifun.cmov >= RRMOVQ && inst->ifun.cmov <= CMOVG) {
                printf("%s %s, %s",
                    cmov_names[inst->ifun.cmov],
                    reg_names[inst->ra],
                    reg_names[inst->rb]);
            }
            break;

        case IRMOVQ:
            printf("irmovq 0x%lx, %s",
                   inst->valC.v,
                   reg_names[inst->rb]);
            break;

        case RMMOVQ:
            if (inst->rb != NOREG) {
            printf("rmmovq %s, 0x%lx(%s)",
                reg_names[inst->ra],
                inst->valC.d,
                reg_names[inst->rb]);
            } else {
                printf("rmmovq %s, 0x%lx",
                    reg_names[inst->ra],
                    inst->valC.d);
            }
            break;

        case MRMOVQ:
            if (inst->rb != NOREG) {
            printf("mrmovq 0x%lx(%s), %s",
                inst->valC.d,
                reg_names[inst->rb],
                reg_names[inst->ra]);
            } else {
                printf("mrmovq 0x%lx, %s",
                    inst->valC.d,
                    reg_names[inst->ra]);
            }
            break;

        case OPQ:
            if (inst->ifun.op >= ADD && inst->ifun.op <= XOR) {
                printf("%s %s, %s",
                       opq_names[inst->ifun.op],
                       reg_names[inst->ra],
                       reg_names[inst->rb]);
            }
            break;

        case JUMP:
            if (inst->ifun.jump >= JMP && inst->ifun.jump <= JG) {
                printf("%s 0x%lx",
                       jump_names[inst->ifun.jump],
                       inst->valC.dest);
            }
            break;

        case CALL:
            printf("call 0x%lx", inst->valC.dest);
            break;

        case RET:
            printf("ret");
            break;

        case PUSHQ:
            printf("pushq %s", reg_names[inst->ra]);
            break;

        case POPQ:
            printf("popq %s", reg_names[inst->ra]);
            break;

        case IOTRAP:
            printf("iotrap %d", inst->ifun.b);
            break;

        default:
            break;
    }
}

#define MAX_HEX_BYTES_CODE 30
void disassemble_code(byte_t *memory, elf_phdr_t *phdr, elf_hdr_t *hdr)
{

    y86_t cpu;
    y86_inst_t inst;
    address_t start_addr = phdr->p_vaddr;
    address_t end_addr = start_addr + phdr->p_size;
    cpu.pc = start_addr;

    printf("  0x%03lx:                               | .pos 0x%03lx code\n", start_addr, start_addr);

    // Loop over instructions
    while (cpu.pc < end_addr) {
        address_t instr_addr = cpu.pc;
        inst = fetch(&cpu, memory);

        // print error and break
        if (inst.icode == INVALID) {
            printf("Invalid opcode: 0x%02x\n", memory[instr_addr]);
            break;
        }

        // instruction matches the entry point
        if (instr_addr == hdr->e_entry) {
            printf("  0x%03lx:                               | _start:\n", instr_addr);
        }

        printf("  0x%03lx: ", instr_addr);

        // get hex bytes of the instruction
        char hex_bytes[MAX_HEX_BYTES_CODE];
        int hex_len = 0;
        for (address_t addr = instr_addr; addr < inst.valP && addr < MEMSIZE; addr++) {
            if (hex_len + 3 > MAX_HEX_BYTES_CODE) {
                break;
            }
            hex_len += snprintf(&hex_bytes[hex_len], MAX_HEX_BYTES_CODE - hex_len + 1, "%02x ", memory[addr]);
        }

        // Pad the hex_bytes characters
        printf("%-30s", hex_bytes);

        // Print the assembly code
        printf("|   ");
        disassemble(&inst);
        printf("\n");

        // Update the PC to the next instruction
        cpu.pc = inst.valP;
    }
}

#define MAX_HEX_BYTES_DATA 24
void disassemble_data (byte_t *memory, elf_phdr_t *phdr)
{
    if (memory == NULL || phdr == NULL) {
        return;
    }

    address_t start_addr = phdr->p_vaddr;
    address_t end_addr = start_addr + phdr->p_size;

    // Check if addresses are in bounds
    if (start_addr >= MEMSIZE || end_addr > MEMSIZE) {
        printf("Error: Data segment addresses out of memory bounds\n");
        return;
    }

    // Print the .pos and data label
    printf("  0x%lx:                               | .pos 0x%lx data\n", start_addr, start_addr);

    // Loop in 8-byte increments
    for (address_t addr = start_addr; addr < end_addr; addr += 8) {
        // bytes left to read
        int bytes_left;
        if (end_addr - addr >= 8) {
            bytes_left = 8;
        } else {
            bytes_left = (int)(end_addr - addr);
        }


        printf("  0x%lx: ", addr); // print address

        char hex_bytes[MAX_HEX_BYTES_DATA];
        int hex_len = 0;
        for (int i = 0; i < bytes_left; i++) {
            hex_len += snprintf(&hex_bytes[hex_len], MAX_HEX_BYTES_DATA - hex_len + 1, "%02x ", memory[addr + i]);
        }

        // Pad the hex_bytes characters
        printf("%-24s", hex_bytes);

        // Read the 8-byte value
        uint64_t quad_value = 0;
        memcpy(&quad_value, &memory[addr], bytes_left);

        // Print the .quad in hex
        printf("      |   .quad 0x%lx\n", quad_value);
    }
}

void disassemble_rodata(byte_t *memory, elf_phdr_t *phdr)
{
    if (memory == NULL || phdr == NULL) {
        return;
    }

    address_t start_addr = phdr->p_vaddr;
    address_t end_addr = start_addr + phdr->p_size;

    // Check within memory bounds
    if (start_addr >= MEMSIZE || end_addr > MEMSIZE) {
        return;
    }

    // Print the .pos and rodata label
    printf("  0x%lx:                               | .pos 0x%lx rodata\n", start_addr, start_addr);

    address_t addr = start_addr;
    while (addr < end_addr) {
        // Initialize buffers
        char hex_bytes[MAX_HEX_BYTES_CODE] = {0};
        char ascii_str[256] = {0};
        int hex_len = 0;
        int str_len = 0;

        // starting address
        address_t string_start_addr = addr;

        // Read characters until end
        while (addr < end_addr) {
            // Append hex
            if (hex_len + 3 <= MAX_HEX_BYTES_CODE) {
                hex_len += snprintf(&hex_bytes[hex_len], MAX_HEX_BYTES_CODE - hex_len + 1, "%02x ", memory[addr]);
            }

            // Append character to string
            ascii_str[str_len++] = memory[addr];

            addr++;
        }

        // Print the address
        printf("  0x%lx: ", string_start_addr);

        // Pad the hex_bytes characters
        printf("%-30s", hex_bytes);

        // Print and ASCII string
        printf("|   .string \"%s\"\n", ascii_str);

        // If the string was longer than the hex_bytes field, print additional lines for the remaining hex bytes
        address_t extra_addr = string_start_addr + (MAX_HEX_BYTES_CODE / 3);
        while (addr > extra_addr) {
            // bytes already printed
            int bytes_printed = (MAX_HEX_BYTES_CODE / 3);
            int bytes_remaining = (addr - string_start_addr) - bytes_printed;

            if (bytes_remaining <= 0) {
                break;
            }

            // Prepare the next line
            hex_len = 0;
            for (int i = 0; i < 10 && bytes_remaining > 0; i++, bytes_printed++, bytes_remaining--) {
                hex_len += snprintf(&hex_bytes[hex_len], MAX_HEX_BYTES_CODE - hex_len + 1, "%02x ", memory[string_start_addr + bytes_printed]);
            }

            // Print the address and hex bytes
            printf("  0x%lx: ", extra_addr);
            printf("%-30s|\n", hex_bytes);

            extra_addr += (MAX_HEX_BYTES_CODE / 3);
        }
    }
}

