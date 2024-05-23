#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rv_emu.h"
#include "bits.h"

#define DEBUG 0

static void unsupported(char *s, uint32_t n) {
    printf("unsupported %s 0x%x\n", s, n);
    exit(-1);
}

void emu_r_type(rv_state *rsp, uint32_t iw) {
    uint32_t rd = get_bits(iw, 7, 5);
    uint32_t rs1 = get_bits(iw, 15, 5);
    uint32_t rs2 = get_bits(iw, 20, 5);
    uint32_t funct3 = get_bits(iw, 12, 3);
    uint32_t funct7 = get_bits(iw, 25, 7);
    uint32_t imm_3 = get_bit(iw, 3);

    if (funct3 == 0b000 && funct7 == 0b0000000) {
        rsp->regs[rd] = rsp->regs[rs1] + rsp->regs[rs2];    // add
    } else if (funct3 == 0b000 && funct7 == 0b0100000) {
        rsp->regs[rd] = rsp->regs[rs1] - rsp->regs[rs2];    // sub
    } else if (funct3 == 0b000 && funct7 == 0b0000001) {
        rsp->regs[rd] = rsp->regs[rs1] * rsp->regs[rs2];    // mul
    } else if (funct3 == 0b100 && funct7 == 0b0000001) {
        rsp->regs[rd] = rsp->regs[rs1] / rsp->regs[rs2];    // div
    } else if (funct3 == 0b101 && funct7 == 0b0000000) {
        rsp->regs[rd] = rsp->regs[rs1] >> rsp->regs[rs2];   // srl
    } else if (funct3 == 0b001 && funct7 == 0b0000000) {
        if (imm_3)
            rsp->regs[rd] = ((int32_t)rsp->regs[rs1]) << ((int32_t)rsp->regs[rs2]); // sllw
        else
            rsp->regs[rd] = rsp->regs[rs1] << rsp->regs[rs2];   // sll
    } else if (funct3 == 0b101 && funct7 == 0b0100000) {
        if (imm_3)
            rsp->regs[rd] = ((int32_t)rsp->regs[rs1]) >> ((int32_t)rsp->regs[rs2]); // sraw
        else
            rsp->regs[rd] = ((int64_t)rsp->regs[rs1]) >> rsp->regs[rs2]; // sra
    } else if (funct3 == 0b111 && funct7 == 0b0000000) {
        rsp->regs[rd] = rsp->regs[rs1] & rsp->regs[rs2];    // and
    } else {
        unsupported("R-type funct3", funct3);
    }
    rsp->analysis.ir_count += 1;
    rsp->analysis.i_count += 1;
    rsp->pc += 4; // Next instruction
}

void emu_i_type(rv_state *rsp, uint32_t iw) {
    uint32_t rd = get_bits(iw, 7, 5);
    uint32_t rs1 = get_bits(iw, 15, 5);
    uint32_t funct3 = get_bits(iw, 12, 3);
    uint32_t funct7 = get_bits(iw, 25, 7);
    uint32_t immu = get_bits(iw, 20, 12);
    uint32_t shamt = get_bits(iw, 20, 6);

    int64_t imm = sign_extend(immu, 12);
 
    if (funct3 == 0b000) {
        rsp->regs[rd] = rsp->regs[rs1] + imm;   // addi
    } else if (funct3 == 0b001 && funct7 == 0b0000000) {
        rsp->regs[rd] = rsp->regs[rs1] << shamt;    // slli
    } else if (funct3 == 0b101 && funct7 == 0b0000000) {
        rsp->regs[rd] = rsp->regs[rs1] >> shamt;    // srli
    } else if (funct3 == 0b101 && funct7 == 0b0100000) {
        rsp->regs[rd] = ((int64_t) rsp->regs[rs1]) >> shamt;    // srai
    } else {
        unsupported("I-type funct3", funct3);
    }
    rsp->analysis.ir_count += 1;
    rsp->analysis.i_count += 1;
    rsp->pc += 4; // Next instruction
}

void emu_load_type(rv_state *rsp, uint32_t iw) {
    uint32_t rd = get_bits(iw, 7, 5);
    uint32_t rs1 = get_bits(iw, 15, 5);
    uint32_t funct3 = get_bits(iw, 12, 3);
    uint32_t immu = get_bits(iw, 20, 12);

    int64_t imm = sign_extend(immu, 12);
    uint64_t targ_addr = rsp->regs[rs1] + imm;

    if (funct3 == 0b000) {
        rsp->regs[rd] = *((uint8_t *) targ_addr);   // lb
    } else if (funct3 == 0b010) {
        rsp->regs[rd] = *((uint32_t *) targ_addr);  // lw
    } else if (funct3 == 0b011) {
        rsp->regs[rd] = *((uint64_t *) targ_addr);  // ld
    } else {
        unsupported("Load I-type funct3", funct3);
    }
    rsp->analysis.ld_count += 1;
    rsp->analysis.i_count += 1;
    rsp->pc += 4; // Next instruction
}

void emu_s_type(rv_state *rsp, uint32_t iw) {
    uint32_t rs1 = get_bits(iw, 15, 5);
    uint32_t rs2 = get_bits(iw, 20, 5);
    uint32_t funct3 = get_bits(iw, 12, 3);
    uint32_t imm_4_0 = get_bits(iw, 7, 5);
    uint32_t imm_11_5 = get_bits(iw, 25, 7);

    uint32_t immu = (imm_11_5 << 5) | (imm_4_0 << 0);

    int64_t imm = sign_extend(immu, 12);
    uint64_t targ_addr = rsp->regs[rs1] + imm;

    if (funct3 == 0b000) {
        *((uint8_t *) targ_addr) = ((uint8_t)rsp->regs[rs2]);   // sb
    } else if (funct3 == 0b010) {
        *((uint32_t *) targ_addr) = ((uint32_t)rsp->regs[rs2]); // sw
    } else if (funct3 == 0b011) {
        *((uint64_t *) targ_addr) = ((uint64_t)rsp->regs[rs2]); // sd
    } else {
        unsupported("S-type funct3", funct3);
    }
    rsp->analysis.st_count += 1;
    rsp->analysis.i_count += 1;
    rsp->pc += 4; // Next instruction
}

void emu_b_type(rv_state *rsp, uint32_t iw) {
    uint32_t funct3 = get_bits(iw, 12, 3);
    uint32_t rs1 = get_bits(iw, 15, 5);
    uint32_t rs2 = get_bits(iw, 20, 5);

    uint32_t imm_11 = get_bit(iw, 7); 
    uint32_t imm_4_1 = get_bits(iw, 8, 4);
    uint32_t imm_10_5 = get_bits(iw, 25, 6);
    uint32_t imm_12 = get_bit(iw, 31);

    uint32_t immu = (imm_12 << 12) | (imm_11 << 11) | (imm_10_5 << 5) | (imm_4_1 << 1); 
    int64_t imm = sign_extend(immu, 13); 

    int taken = 0;

    if (funct3 == 0b100) {  // blt
        if (((int64_t)rsp->regs[rs1]) < ((int64_t)rsp->regs[rs2])) {
            rsp->pc += imm;    // Jumps to the branch if statement is true
            taken = 1;
        } else {
            rsp->pc += 4;      // Next instruction
        }
    } else if (funct3 == 0b001) {   // bne
        if (rsp->regs[rs1] != rsp->regs[rs2]) {
            rsp->pc += imm;
            taken = 1;
        } else {
            rsp->pc += 4;
        }
    } else if (funct3 == 0b000) {   // beq
        if (rsp->regs[rs1] == rsp->regs[rs2]) {
            rsp->pc += imm;
            taken = 1;
        } else {
            rsp->pc += 4;

        }
    } else if (funct3 == 0b101) {   // bge
        if (((int64_t)rsp->regs[rs1]) >= ((int64_t)rsp->regs[rs2])) {
            rsp->pc += imm;
            taken = 1;
        } else {
            rsp->pc += 4;
        }
    } else {
        unsupported("B-type funct3", funct3);
    }

    if (taken == 1) {
        rsp->analysis.b_taken += 1;
    } else if (taken == 0) {
        rsp->analysis.b_not_taken += 1;
    }
    rsp->analysis.i_count += 1;
}

void emu_j_type(rv_state *rsp, uint32_t iw) {
    uint32_t rd = get_bits(iw, 7, 5);

    if (rd != 0) {   
        rsp->regs[rd] = ((uint64_t)rsp->pc) + 4;
    }

    uint32_t imm_20 = get_bit(iw, 31);
    uint32_t imm_10_1 = get_bits(iw, 21, 10);
    uint32_t imm_11 = get_bit(iw, 20);
    uint32_t imm_19_12 = get_bits(iw, 12, 8);

    uint32_t immu = (imm_20 << 20) | (imm_19_12 << 12) | (imm_11 << 11) | (imm_10_1 << 1);
    int64_t imm = sign_extend(immu, 20);

    rsp->analysis.j_count += 1;
    rsp->analysis.i_count += 1;
    rsp->pc += imm;   
}

void emu_jalr(rv_state *rsp, uint32_t iw) {
    uint32_t rs1 = get_bits(iw, 15, 4);  // Will be ra (aka x1)
    uint64_t val = rsp->regs[rs1];  // Value of regs[1]

    rsp->analysis.j_count += 1;
    rsp->analysis.i_count += 1;
    rsp->pc = val;  // PC = return address
}

static void rv_one(rv_state *state) {
    uint32_t iw  = *((uint32_t*) state->pc);
    iw = cache_lookup(&state->i_cache, (uint64_t) state->pc);

    uint32_t opcode = get_bits(iw, 0, 7);

#if DEBUG
    printf("iw: %08x\n", iw);
#endif
    switch (opcode) {
        case FMT_R:
        case FMT_WORD_R:
            // R-type and R-type word instructions
            emu_r_type(state, iw);
            break;
        case FMT_I_ARITH:
            // I-type instruction
            emu_i_type(state, iw);
            break;
        case FMT_I_LOAD:
            // Load I-type instruction
            emu_load_type(state, iw);
            break;
        case FMT_S:
            // S-type instruction
            emu_s_type(state, iw);
            break;
        case FMT_SB:
            // B-type instruction
            emu_b_type(state, iw);
            break;
        case FMT_UJ:
            // J-type instruction
            emu_j_type(state, iw);
            break;
        case FMT_I_JALR:
            // JALR (aka RET) is a variant of I-type instructions
            emu_jalr(state, iw);
            break;
        default:
            unsupported("Unknown opcode: ", opcode);
    }
}

void rv_init(rv_state *state, uint32_t *target, 
             uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3) {
    state->pc = (uint64_t) target;
    state->regs[RV_A0] = a0;
    state->regs[RV_A1] = a1;
    state->regs[RV_A2] = a2;
    state->regs[RV_A3] = a3;

    state->regs[RV_ZERO] = 0;  // zero is always 0  (:
    state->regs[RV_RA] = RV_STOP;
    state->regs[RV_SP] = (uint64_t) &state->stack[STACK_SIZE];

    memset(&state->analysis, 0, sizeof(rv_analysis));
    cache_init(&state->i_cache);
}

uint64_t rv_emulate(rv_state *state) {
    while (state->pc != RV_STOP) {
        rv_one(state);
    }
    return state->regs[RV_A0];
}

static void print_pct(char *fmt, int numer, int denom) {
    double pct = 0.0;

    if (denom)
        pct = (double) numer / (double) denom * 100.0;
    printf(fmt, numer, pct);
}

void rv_print(rv_analysis *a) {
    int b_total = a->b_taken + a->b_not_taken;

    printf("=== Analysis\n");
    print_pct("Instructions Executed  = %d\n", a->i_count, a->i_count);
    print_pct("R-type + I-type        = %d (%.2f%%)\n", a->ir_count, a->i_count);
    print_pct("Loads                  = %d (%.2f%%)\n", a->ld_count, a->i_count);
    print_pct("Stores                 = %d (%.2f%%)\n", a->st_count, a->i_count);    
    print_pct("Jumps/JAL/JALR         = %d (%.2f%%)\n", a->j_count, a->i_count);
    print_pct("Conditional branches   = %d (%.2f%%)\n", b_total, a->i_count);
    print_pct("  Branches taken       = %d (%.2f%%)\n", a->b_taken, b_total);
    print_pct("  Branches not taken   = %d (%.2f%%)\n", a->b_not_taken, b_total);
}
