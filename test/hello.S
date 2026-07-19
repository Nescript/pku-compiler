  .text
  .globl main
main:
  addi sp, sp, -16
  li t0, 6
  li t1, 0
  xor t2, t0, t1
  seqz t2, t2
  sw t2, 0(sp)
  li t0, 0
  lw t1, 0(sp)
  sub t2, t0, t1
  sw t2, 4(sp)
  li t0, 0
  lw t1, 4(sp)
  sub t2, t0, t1
  sw t2, 8(sp)
  lw a0, 8(sp)
  addi sp, sp, 16
  ret
