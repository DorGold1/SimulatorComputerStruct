#   program reads matrix A from address 0x100
#   and matrix B from address 0x110
#   calcualte C=A*B into address 0x120
#   all matrices are sized 4*4 and indices are in range [1,4]

#   A Matrix
.word 0x100 1
.word 0x101 1
.word 0x102 1
.word 0x103 1
.word 0x104 2
.word 0x105 2
.word 0x106 2
.word 0x107 2
.word 0x108 0
.word 0x109 3
.word 0x10A 3
.word 0x10B 3
.word 0x10C 1
.word 0x10D 1
.word 0x10E 1
.word 0x10F 1
#################
#   B Matrix
.word 0x110 1
.word 0x111 0
.word 0x112 0
.word 0x113 0
.word 0x114 0
.word 0x115 1
.word 0x116 0
.word 0x117 0
.word 0x118 0
.word 0x119 0
.word 0x11A 0
.word 0x11B 1
.word 0x11c 0
.word 0x11d 0
.word 0x11e 0
.word 0x11f 1
#################

add $s0, $zero, $zero, $zero, 0, 0                       # $s0 = running index (denote as i) = 0
add $s1, $zero, $zero, $zero, 0, 0                       # $s1 = running index (denote as j) = 0
add $s2, $imm2, $zero, $zero, 0, 0x120                   # $s2 = address of C(0,0)
LOOPi:
    beq $zero, $s0, $imm1, $imm2, 4, finish_LOOPi        # end condition of loop
    LOOPj:
        beq $zero, $s1, $imm1, $imm2, 4, finish_LOOPj    # end condition of loop
        add $a0, $s0, $zero, $zero, 0, 0                 # $a0 = i
        add $a1, $s1, $zero, $zero, 0, 0                 # $a1 = j
        jal $ra, $zero, $zero, $imm2, 0, getVal          # $v0 = value of C(i,j)
        mac $t0, $s0, $imm1, $s1, 4, 0                   # $t0 = 4*i+j
        sw $v0, $s2, $t0, $zero, 0, 0                    # C(i,j) = $v0
        
        add $s1, $s1, $imm2, $zero, 0, 1                 # j++
        beq $zero, $zero, $zero, $imm2, 0, LOOPj	     # uncond jump to LOOPj

    finish_LOOPj:
        add $s0, $s0, $imm2, $zero, 0, 1                 # i++
        beq $zero, $zero, $zero, $imm2, 0, LOOPi	     # uncond jump to LOOPi

finish_LOOPi: halt $zero, $zero, $zero, $zero, 0, 0		 # halt - exit program


getVal:
# assuming $a0 = i , $a1 = j --> get value for C(i,j)

    add $sp, $sp, $imm2, $zero, 0, -3		                 # adjust stack for 3 items
    sw $zero, $sp, $imm2, $s1, 0, 2			                 # save $s2
    sw $zero, $sp, $imm2, $s1, 0, 1			                 # save $s1
    sw $zero, $sp, $imm2, $s0, 0, 0			                 # save $s0

    mac $t0, $t0, $imm1, $zero, 4, 0                          # $t0 =  4*i
    add $s0, $imm1, $t0, $imm2, 0x100, -4                    # $s0 = address of A(i,0)
    add $s1, $imm1, $a1, $imm2, 0x110, -1                    # $s1 = address of B(0,j)
    add $t0, $zero, $zero, $zero, 0, 0                       # $t0 = running index (denote as k) = 0
    add $v0, $zero, $zero, $zero, 0, 0                       # $v0 = result value = 0

    LOOPval:
        beq $zero, $t0, $imm1, $imm2, 4, finish_LOOPval		 # end condition of loop
        lw $t1, $s0, $t0, $zero, 0, 0                        # load A(i,k) into $t1
        mac $t2, $t0, $imm1, $zero, 4, 0                     # $t2 = 4*k
        lw $t2, $s1, $t2, $zero, 0, 0                        # load B(k,j) into $t2
        mac $v0, $t1, $t2, $v0, 0, 0                         # $v0 = $v0 + A(i,k)*B(j,k)

        add $t0, $t0, $imm2, $zero, 0, 1                     # k++
        beq $zero, $zero, $zero, $imm2, 0, LOOPval           # uncond jump to LOOPval

    finish_LOOPval:
        lw $s0, $sp, $imm2, $zero, 0, 0			             # restore $s0
        lw $s1, $sp, $imm2, $zero, 0, 1			             # restore $s1
        lw $s1, $sp, $imm2, $zero, 0, 2			             # restore $s2
        beq $zero, $zero, $zero, $ra, 0, 0		             # return