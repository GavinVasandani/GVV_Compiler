.globl f
f:
addi sp, sp, -40
sw ra, 8(sp)
sw a0, 0(sp)
sw a1, 4(sp)
lw a0, 0(sp)
lw a1, 4(sp)
add a0, a0, a1
mv a0, a0
addi sp, sp, 40
jr ra 
addi sp, sp, 40
