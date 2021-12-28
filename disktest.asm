#   program that moves disk sectors [0,7] to sectors [1,8]
#   program will use sector 8 as tmp variable for moving each sector

out $zero, $zero, $imm1, $imm2, 1, 1        # enable irq1


Write_to_sector_8:
# assuming $a0 is index of sector to copy
# assuming $a1 is the buffer's address

wait_for_free: in $t0, $imm2, $zero, $zero, 0, 17      # $t0 = disk status
beq $zero, $t0, $imm1, $imm2, 1, wait_for_free         # $t0 = 1 --> keep waiting for disk to be free

out $zero, $zero, $imm1, $a0, 15, 0                    # R[disksector] = $a0
out $zero, $zero, $imm1, $a1, 16, 0                    # R[diskbuffer] = $a1

