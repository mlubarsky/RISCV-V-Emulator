.global eval_s

# a0 - char *expr_str
# a1 - int pos
eval_s:
    addi sp, sp, -16
    sd ra, (sp)

    addi a1,sp, 8                       # a1 points to pos on stack 8(sp)
    sw zero, (a1)                       # put 0 (zero) into this location

    call expression_s

    ld ra, (sp)
    addi sp, sp, 16
    ret

# a0 - char *expr_str
# a1 - int *pos

# t0 - int value
# t1 - char token
# t2 - int pos_val
expression_s:
    addi sp, sp, -64
    sd ra, (sp)

    sd a0, 8(sp)                        # Store a0 on stack
    sd a1, 16(sp)                       # Store a1 on stack

    call term_s
    mv t0, a0                           # t0 (value) = term_s(expr_str, pos)

    ld a0, 8(sp)                        # Restore a0 from stack
    ld a1, 16(sp)                       # Restore a1 from stack

expression_while:
    lw t2, (a1)                         # t2 (pos_val) = *a1 (pos)
    add t3, a0, t2                      # t3 = a0 (*expr_str) + t2 (pos_val)
    lb t1, (t3)                         # t1 (token) = *t3
    li t4, '+'
    beq t1, t4, expression_while_cont
    li t5, '-'
    bne t1, t5, expression_while_done

expression_while_cont:
    addi t2, t2, 1                      # t2 (pos_val) = t2 (pos_val) + 1
    sw t2, (a1)                         # *pos = t2 (pos_val)

    bne t1, t4, expression_while_else

    sd a0, 8(sp)                        # Preserve all the caller-saved registers
    sd a1, 16(sp)
    sd t0, 24(sp)
    sd t1, 32(sp)
    sd t2, 40(sp)
    sd t4, 48(sp)
    sd t5, 56(sp)

    call term_s

    ld t0, 24(sp)                       # Restore t0 (value) from stack
    add t0, t0, a0                      # t0 (value) = t0 (value) + a0

    ld a0, 8(sp)                        # Restore rest of the registers
    ld a1, 16(sp)
    ld t1, 32(sp)
    ld t2, 40(sp)
    ld t4, 48(sp)
    ld t5, 56(sp)

    j expression_while

expression_while_else:
    bne t1, t5, expression_while_done

    sd a0, 8(sp)                        # Preserve all caller-saved registers
    sd a1, 16(sp)
    sd t0, 24(sp)
    sd t1, 32(sp)
    sd t2, 40(sp)
    sd t4, 48(sp)
    sd t5, 56(sp)

    call term_s

    ld t0, 24(sp)                       # Restore t0 (value) from stack
    sub t0, t0, a0                      # t0 (value) = t0 (value) - a0

    ld a0, 8(sp)                        # Restore rest of the registers
    ld a1, 16(sp)
    ld t1, 32(sp)
    ld t2, 40(sp)
    ld t4, 48(sp)
    ld t5, 56(sp)

    j expression_while

expression_while_done:
    mv a0, t0                           # a0 = t0 (value)
    ld ra, (sp)
    addi sp, sp, 64
    ret

# a0 - char *expr_str
# a1 - int *pos

# t0 - int value
# t1 - char token
# t2 - int pos_val
term_s:
    addi sp, sp, -64
    sd ra, (sp)

    sd a0, 8(sp)                        # Store a0 on stack
    sd a1, 16(sp)                       # Store a1 on stack
                
    call factor_s
    mv t0, a0                           # t0 (value) = term_s(expr_str, pos)

    ld a0, 8(sp)                        # Restore a0 from stack
    ld a1, 16(sp)                       # Restore a1 from stack

term_while:
    lw t2, (a1)                         # t2 (pos_val) = *a1 (pos)
    add t3, a0, t2                      # t3 = a0 (*expr_str) + t2 (pos_val)
    lb t1, (t3)                         # t1 (token) = *t3
    li t4, '*'
    beq t1, t4, term_while_cont
    li t5, '/'
    bne t1, t5, term_while_done

term_while_cont:
    addi t2, t2, 1                      # t2 (pos_val) = t2 (pos_val) + 1
    sw t2, (a1)                         # *pos = t2 (pos_val)

    bne t1, t4, term_while_else

    sd a0, 8(sp)                        # Preserve all caller-saved registers
    sd a1, 16(sp)
    sd t0, 24(sp)
    sd t1, 32(sp)
    sd t2, 40(sp)
    sd t4, 48(sp)
    sd t5, 56(sp)

    call factor_s

    ld t0, 24(sp)                       # Restore t0 (value) from stack
    mul t0, t0, a0                      # t0 (value) = t0 (value) + a0

    ld a0, 8(sp)                        # Restore rest of the registers
    ld a1, 16(sp)
    ld t1, 32(sp)
    ld t2, 40(sp)
    ld t4, 48(sp)
    ld t5, 56(sp)

    j term_while

term_while_else:
    bne t1, t5, term_while_done

    sd a0, 8(sp)
    sd a1, 16(sp)                       # Preserve all caller-saved registers
    sd t0, 24(sp)
    sd t1, 32(sp)
    sd t2, 40(sp)
    sd t4, 48(sp)
    sd t5, 56(sp)

    call factor_s

    ld t0, 24(sp)                       # Restore t0 (value) from stack
    div t0, t0, a0                      # t0 (value) = t0 (value) - a0

    ld a0, 8(sp)                        # Restore rest of the registers
    ld a1, 16(sp)
    ld t1, 32(sp)
    ld t2, 40(sp)
    ld t4, 48(sp)
    ld t5, 56(sp)

    j term_while

term_while_done:
    mv a0, t0                           # a0 = t0 (value)
    ld ra, (sp)
    addi sp, sp, 64
    ret

# a0 - char *expr_str
# a1 - int *pos

# t0 - value
# t1 - token
factor_s:
    addi sp, sp, -64
    sd ra, (sp)

    lw t3, (a1)
    add t4, a0, t3
    lb t1, (t4)                         # token = expr_str[*pos]

    li t6, '('
    bne t1, t6, factor_else             # if (token == '(')
    lw t5, (a1)
    addi t5, t5, 1
    sw t5, (a1)                         # *pos += 1
    
    sd a0, 8(sp)
    sd a1, 16(sp)   
    sd t1, 32(sp)              
    call expression_s
    mv t0, a0                           # value = expression_s(expr_str, pos)

    ld a0, 8(sp)
    ld a1, 16(sp)
    ld t1, 32(sp)

    lw t3, (a1)
    add t4, a0, t3
    lb t1, (t4)                         # token = expr_str[*pos]

    li t6, ')'
    bne t1, t6, factor_missing_parend   # if (token == ')')

    lw t5, (a1)
    addi t5, t5, 1
    sw t5, (a1)                         # *pos += 1

factor_inner_if:
    j factor_done

factor_missing_parend:
    li t0, 0
    j factor_done

factor_else:
    sd a0, 8(sp)                        # Preserve caller-saved registers
    sd a1, 16(sp)
    sd t1, 32(sp)
    
    call number_s
    mv t0, a0

    ld a0, 8(sp)                        # Restore caller-saved registers
    ld a1, 16(sp)
    ld t1, 32(sp)
    
    j factor_done
    
factor_done:
    mv a0, t0

    ld ra, (sp)
    addi sp, sp, 64
    ret

# a0 - char *expr_str
# a1 - int *pos

# t0 - int value
# t1 - char token
number_s:   
    addi sp, sp, -64
    sd ra, (sp) 
    li t0, 0                            # value = 0

number_loop: 
    lw t3, (a1)                         # t3 = *pos
    add t4, a0, t3
    lb t1, (t4)                         # token = *expr_str[*pos] ?
    
    sd a0, 8(sp)                        # Preserve caller-saved registers
    sd a1, 16(sp)
    sd t0, 24(sp)
    sd t1, 32(sp)

    mv a0, t1
    call isdigit_s 
    beq a0, zero, number_done           # if (!(isdigit(token)))

    ld a0, 8(sp)
    ld a1, 16(sp)                       # Restore caller-saved registers
    ld t0, 24(sp)
    ld t1, 32(sp)

    addi t1, t1, -48                    # t1 = (token - '0')
    li t3, 10               
    mul t0, t0, t3                      # t0 = value * 10
    add t0, t0, t1                      # value = t0 + t1

    lw t5, (a1)
    addi t5, t5, 1
    sw t5, (a1)                         # *pos += 1
    j number_loop
       
number_done:
    ld a0, 8(sp)                        # Restore caller-saved registers
    ld a1, 16(sp)
    ld t0, 24(sp)
    ld t1, 32(sp)

    mv a0, t0
    
    ld ra, 0(sp)
    addi sp, sp, 64
    ret

# a0 - char c
isdigit_s:
    li t0, '0'                           
    li t1, '9'
    blt a0, t0, isdigit_else            # c >= '0'
    bgt a0, t1, isdigit_else            # c <= '9'
    li a0, 1
    j isdigit_done

isdigit_else:
    li a0, 0

isdigit_done:
    ret
