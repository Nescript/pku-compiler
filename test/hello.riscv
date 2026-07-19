  .text
  .globl main
main:
  addi sp, sp, -32
  li t0, 6
  li t1, 2
  div t2, t0, t1
  sw t2, 0(sp)
  li t0, 1
  li t1, 0
  xor t2, t0, t1
  seqz t2, t2
  sw t2, 4(sp)
  lw t0, 4(sp)
  lw t1, 0(sp)
  add t2, t0, t1
  sw t2, 8(sp)
  lw t0, 8(sp)
  li t1, 100
  sub t2, t0, t1
  sw t2, 12(sp)
  lw t0, 12(sp)
  li t1, 32
  add t2, t0, t1
  sw t2, 16(sp)
  lw a0, 16(sp)
  addi sp, sp, 32
  ret
