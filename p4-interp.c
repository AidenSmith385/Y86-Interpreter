/*
 * CS 261 PA4: Mini-ELF interpreter
 *
 * Name: Aiden Smith
 */

#include "p4-interp.h"
#include <inttypes.h>

bool Cond(flag_t zf, flag_t sf, flag_t of, int ifun);

/**********************************************************************
 *                         REQUIRED FUNCTIONS
 *********************************************************************/

y86_reg_t decode_execute (y86_t *cpu, y86_inst_t *inst, bool *cnd, y86_reg_t *valA)
{
    y86_reg_t valE = 0;

    // Check for NULL pointers
    if (!cpu || !inst || !cnd || !valA) {
        if (cpu) {
            cpu->stat = INS;
        }
        return 0;
    }

    *cnd = false; // Initialize condition code to false
    y86_icode_t icode = inst->icode;
    int ifun = inst->ifun.b;

    // Switch based on instruction code
    switch (icode) {
        case HALT:
            // Execution stage for HALT is handled in fetch stage
            // Set CPU status to HLT (handled in fetch)
            cpu->stat = HLT;
            break;

        case NOP:
            // Do nothing
            break;

        case CMOV:
            // Conditional move instructions
            if (ifun < 0 || ifun > 6) {
                cpu->stat = INS; // Invalid function code
                break;
            }
            if (inst->ra >= NUMREGS || inst->rb >= NUMREGS) {
                cpu->stat = INS; // Invalid register
                break;
            }
            *valA = cpu->reg[inst->ra]; // Dec stage: valA ← R[rA]
            *cnd = Cond(cpu->zf, cpu->sf, cpu->of, ifun); // Exe stage: Cnd ← Cond(CC, ifun)
            valE = *valA; // Exe stage: valE ← valA
            break;

        case IRMOVQ:
            if (inst->rb >= NUMREGS) {
                cpu->stat = INS; // Invalid register
                break;
            }
            // Immediate to register move
            valE = inst->valC.v; // Exe stage: valE ← valC
            break;

        case RMMOVQ:
            if (inst->ra >= NUMREGS || inst->rb >= NUMREGS) {
                cpu->stat = INS; // Invalid register
                break;
            }
            // Register to memory move
            *valA = cpu->reg[inst->ra]; // Dec stage: valA ← R[rA]
            {
                y86_reg_t valB = cpu->reg[inst->rb]; // Dec stage: valB ← R[rB]
                valE = valB + inst->valC.d; // Exe stage: valE ← valB + valC
            }
            break;

        case MRMOVQ:
            if (inst->ra >= NUMREGS || inst->rb >= NUMREGS) {
                cpu->stat = INS; // Invalid register
                break;
            }
            // Memory to register move
            {
                y86_reg_t valB = cpu->reg[inst->rb]; // Dec stage: valB ← R[rB]
                valE = valB + inst->valC.d; // Exe stage: valE ← valB + valC
            }
            break;

        case OPQ:
            // Arithmetic and logical operations
            if (ifun < 0 || ifun > 3) {
                cpu->stat = INS; // Invalid operation
                break;
            }
            if (inst->ra >= NUMREGS || inst->rb >= NUMREGS) {
                cpu->stat = INS; // Invalid register
                break;
            }
            *valA = cpu->reg[inst->ra]; // Dec stage: valA ← R[rA]
            {
                y86_reg_t valB = cpu->reg[inst->rb]; // Dec stage: valB ← R[rB]
                switch (inst->ifun.op) {
                    case ADD:
                        valE = valB + *valA; // Exe stage: valE ← valB + valA
                        break;
                    case SUB:
                        valE = valB - *valA; // Exe stage: valE ← valB - valA
                        break;
                    case AND:
                        valE = valB & *valA; // Exe stage: valE ← valB & valA
                        break;
                    case XOR:
                        valE = valB ^ *valA; // Exe stage: valE ← valB ^ valA
                        break;
                    default:
                        cpu->stat = INS; // Invalid operation
                        break;
                }
            }
            break;

        case JUMP:
            if (ifun < 0 || ifun > 6) {
                cpu->stat = INS; // Invalid function code
                break;
            }
            // Jump instructions
            *cnd = Cond(cpu->zf, cpu->sf, cpu->of, ifun); // Exe stage: Cnd ← Cond(CC, ifun)
            break;

        case CALL:
            // Procedure call
            {
                y86_reg_t valB = cpu->reg[RSP]; // Dec stage: valB ← R[%rsp]
                valE = valB - 8; // Exe stage: valE ← valB - 8
            }
            break;

        case RET:
            // Return from procedure
            *valA = cpu->reg[RSP]; // Dec stage: valA ← R[%rsp]
            valE = *valA + 8; // Exe stage: valE ← valA + 8
            break;

        case PUSHQ:
            if (inst->ra >= NUMREGS) {
                cpu->stat = INS; // Invalid register
                break;
            }
            // Push onto stack
            *valA = cpu->reg[inst->ra]; // Dec stage: valA ← R[rA]
            {
                y86_reg_t valB = cpu->reg[RSP]; // Dec stage: valB ← R[%rsp]
                valE = valB - 8; // Exe stage: valE ← valB - 8
            }
            break;

        case POPQ:
            if (inst->ra >= NUMREGS) {
                cpu->stat = INS; // Invalid register
                break;
            }
            // Pop from stack
            *valA = cpu->reg[RSP]; // Dec stage: valA ← R[%rsp]
            valE = *valA + 8; // Exe stage: valE ← valA + 8
            break;

        case IOTRAP:
            // I/O trap instruction
            if (ifun < 0 || ifun > 5) {
                cpu->stat = INS; // Invalid trap ID
                break;
            }
            // Execution is handled in memory stage
            break;

        default:
            // Invalid instruction
            cpu->stat = INS;
            break;
    }

    return valE;
}

void memory_wb_pc (y86_t *cpu, y86_inst_t *inst, byte_t *memory,
        bool cnd, y86_reg_t valA, y86_reg_t valE)
{
    // Check for NULL pointers
    if (!cpu || !inst || !memory) {
        if (cpu) {
            cpu->stat = INS;
        }
        return;
    }

    // Extract instruction fields
    y86_icode_t icode = inst->icode;

    // Initialize valM (value from memory) if needed
    y86_reg_t valM = 0;


    // Switch based on instruction code
    switch (icode) {
        case HALT:
            // Update PC according to the semantics
            cpu->pc = inst->valP;
            // CPU status is already set to HLT during the execute stage
            break;

        case NOP:
            // No operation
            // PC update
            cpu->pc = inst->valP;
            break;

        case CMOV:
            // Conditional move
            if (cnd) {
                if (inst->rb >= NUMREGS) {
                    cpu->stat = INS;
                    break;
                }
                // Write Back stage
                cpu->reg[inst->rb] = valE; // R[rB] ← valE
            }
            // PC update
            cpu->pc = inst->valP;
            break;

        case IRMOVQ:
            // Immediate to register move
            // Write Back stage
            if (inst->rb >= NUMREGS) {
                cpu->stat = INS;
                break;
            }
            cpu->reg[inst->rb] = valE; // R[rB] ← valE
            // PC update
            cpu->pc = inst->valP;
            break;

        case RMMOVQ:
            // Register to memory move
            // Memory stage
            if (valE + 7 >= MEMSIZE) {
                cpu->stat = ADR;
                break;
            } else {
                // M8[valE] ← valA
                for (int i = 0; i < 8; i++) {
                    memory[valE + i] = (valA >> (8 * i)) & 0xFF;
                }
            }
            // PC update
            cpu->pc = inst->valP;
            break;

        case MRMOVQ:
            // Memory to register move
            if (inst->ra >= NUMREGS) {
                cpu->stat = INS;
                break;
            }
            // Memory stage
            if (valE + 7 >= MEMSIZE) {
                cpu->stat = ADR;
            } else {
                // valM ← M8[valE]
                valM = 0;
                for (int i = 0; i < 8; i++) {
                    valM |= ((y86_reg_t)memory[valE + i]) << (8 * i);
                }
                // Write Back stage
                cpu->reg[inst->ra] = valM; // R[rA] ← valM
            }
            // PC update
            cpu->pc = inst->valP;
            break;

        case OPQ:
            // Arithmetic and logical operations
            if (inst->rb >= NUMREGS) {
                cpu->stat = INS;
                break;
            }
            // Write Back stage
            cpu->reg[inst->rb] = valE; // R[rB] ← valE
            // PC update
            cpu->pc = inst->valP;
            break;

        case JUMP:
            // Jump instructions
            // PC update
            if (cnd) {
                cpu->pc = inst->valC.dest; // PC ← valC
            } else {
                cpu->pc = inst->valP; // PC ← valP
            }
            break;

        case CALL:
            // Procedure call
            {
                // Memory stage
                if (valE + 7 >= MEMSIZE) {
                    cpu->stat = ADR;
                } else {
                    // M8[valE] ← valP (Push return address onto stack)
                    for (int i = 0; i < 8; i++) {
                        memory[valE + i] = (inst->valP >> (8 * i)) & 0xFF;
                    }
                    // Write Back stage
                    cpu->reg[RSP] = valE; // R[%rsp] ← valE
                }
                // PC update
                if (cpu->stat == AOK) {
                    cpu->pc = inst->valC.dest; // PC ← valC (Destination address)
                } else {
                    cpu->pc = inst->valP; // PC ← valP (In case of error)
                }
            }
            break;

        case RET:
            // Return from procedure
            // Memory stage
            if (valA + 7 >= MEMSIZE) {
                cpu->stat = ADR;
            } else {
                // valM ← M8[valA] (Pop return address from stack)
                valM = 0;
                for (int i = 0; i < 8; i++) {
                    valM |= ((y86_reg_t)memory[valA + i]) << (8 * i);
                }
                // Write Back stage
                cpu->reg[RSP] = valE; // R[%rsp] ← valE
            }
            // PC update
            if (cpu->stat == AOK) {
                cpu->pc = valM; // PC ← valM (Return address)
            } else {
                cpu->pc = inst->valP; // PC ← valP (In case of error)
            }
            return; // Return early since PC is already updated

        case PUSHQ:
            // Push onto stack
            if (inst->ra >= NUMREGS) {
                cpu->stat = INS;
                break;
            }
            // Memory stage
            if (valE + 7 >= MEMSIZE) {
                cpu->stat = ADR;
            } else {
                // M8[valE] ← valA
                for (int i = 0; i < 8; i++) {
                    memory[valE + i] = (valA >> (8 * i)) & 0xFF;
                }
                // Write Back stage
                cpu->reg[RSP] = valE; // R[%rsp] ← valE
            }
            // PC update
            cpu->pc = inst->valP;
            break;

        case POPQ:
            // Pop from stack
            if (inst->ra >= NUMREGS) {
                cpu->stat = INS;
                break;
            }
            // Memory stage
            if (valA + 7 >= MEMSIZE) {
                cpu->stat = ADR;
            } else {
                // valM ← M8[valA]
                valM = 0;
                for (int i = 0; i < 8; i++) {
                    valM |= ((y86_reg_t)memory[valA + i]) << (8 * i);
                }
                // Write Back stage
                cpu->reg[RSP] = valE; // R[%rsp] ← valE
                cpu->reg[inst->ra] = valM; // R[rA] ← valM
            }
            // PC update
            cpu->pc = inst->valP;
            break;

        case IOTRAP:
            // I/O trap instruction
            if (inst->ifun.b < 0 || inst->ifun.b > 5) {
                cpu->stat = INS; // Invalid trap ID
                break;
            }
            // PC update
            cpu->pc = inst->valP;
            break;

        default:
            // Invalid instruction
            cpu->stat = INS;
            break;
    }

    // For all cases except RET (which returns early), ensure the PC is updated
    if (icode != RET && icode != HALT) {
        cpu->pc = cpu->pc;
    }
}

/**********************************************************************
 *                         OPTIONAL FUNCTIONS
 *********************************************************************/

void dump_cpu_state (y86_t *cpu)
{
    if (!cpu) {
        printf("CPU state is NULL\n");
        return;
    }

    // Map status codes to strings
    const char *stat_str;
    switch (cpu->stat) {
        case AOK:
            stat_str = "AOK";
            break;
        case HLT:
            stat_str = "HLT";
            break;
        case ADR:
            stat_str = "ADR";
            break;
        case INS:
            stat_str = "INS";
            break;
        default:
            stat_str = "UNK"; // Unknown status
            break;
    }

    // Print CPU state
    printf("    PC: %016" PRIx64 "   flags: Z%d S%d O%d     %s\n",
           cpu->pc, cpu->zf, cpu->sf, cpu->of, stat_str);

    // Print registers in the expected format
    printf("  %%rax: %016" PRIx64 "    %%rcx: %016" PRIx64 "\n",
           cpu->reg[RAX], cpu->reg[RCX]);
    printf("  %%rdx: %016" PRIx64 "    %%rbx: %016" PRIx64 "\n",
           cpu->reg[RDX], cpu->reg[RBX]);
    printf("  %%rsp: %016" PRIx64 "    %%rbp: %016" PRIx64 "\n",
           cpu->reg[RSP], cpu->reg[RBP]);
    printf("  %%rsi: %016" PRIx64 "    %%rdi: %016" PRIx64 "\n",
           cpu->reg[RSI], cpu->reg[RDI]);
    printf("   %%r8: %016" PRIx64 "     %%r9: %016" PRIx64 "\n",
           cpu->reg[R8], cpu->reg[R9]);
    printf("  %%r10: %016" PRIx64 "    %%r11: %016" PRIx64 "\n",
           cpu->reg[R10], cpu->reg[R11]);
    printf("  %%r12: %016" PRIx64 "    %%r13: %016" PRIx64 "\n",
           cpu->reg[R12], cpu->reg[R13]);
    printf("  %%r14: %016" PRIx64 "\n",
           cpu->reg[R14]);
}





/**********************************************************************
 *                HELPER FUNCTIONS FOR DECODE EXECUTE
 *********************************************************************/

/**
 * @brief Evaluate the condition for conditional moves and jumps.
 *
 * @param zf Zero Flag.
 * @param sf Sign Flag.
 * @param of Overflow Flag.
 * @param ifun Function code (ifun) specifying the condition.
 * @return True if the condition is met, false otherwise.
 */
bool Cond(flag_t zf, flag_t sf, flag_t of, int ifun)
{
    switch (ifun) {
        case 0: // rrmovq or jmp (unconditional)
            return true;
        case 1: // le (less or equal)
            return (sf != of) || zf;
        case 2: // l (less than)
            return (sf != of);
        case 3: // e (equal)
            return zf;
        case 4: // ne (not equal)
            return !zf;
        case 5: // ge (greater or equal)
            return (sf == of);
        case 6: // g (greater than)
            return (sf == of) && !zf;
        default:
            return false;
    }
}

