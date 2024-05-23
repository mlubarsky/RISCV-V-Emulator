.global fib_rec_s


# fib_rec - compute the nth fibonacci number

# a0 - int n

fib_rec_s:
    addi sp, sp, -32
    sd ra, (sp)

    li t0, 1
    bgt a0, t0, recstep     # s = n > 1, c = n <= 1
    j done

recstep:
    sd a0, 8(sp)            # store n on stack
    addi a0, a0, -1         # a0 = (n - 1)
    call fib_rec_s           
    sd a0, 16(sp)           # store fib_rec(n - 1) on stack

    ld a0, 8(sp)            # load n from stack
    addi a0, a0, -2         # a0 = (n - 2)
    call fib_rec_s
    sd a0, 24(sp)           # store fib_rec(n - 2) on stack

    ld t1, 16(sp)           # load fib_rec(n - 1)
    ld t2, 24(sp)           # load fib_rec(n - 2)
    
    add a0, t1, t2          # add result into a0

done:
    ld ra, (sp)
    addi sp, sp, 32
    ret
