	sll $sp, $imm1, $imm2, $zero, 1, 12	    # set $sp = 1 << 12 = 4096
	lw $a0, $zero, $imm2, $zero, 0, 0x100	# load n
	lw $a1, $zero, $imm2, $zero, 0, 0x101	# load k
	add $sp, $sp, $imm2, $zero, 0, -4   	# adjust stack for 4 items
	jal $ra $zero, $zero, $imm2, 0, bin		# calc $v0 = bin(n,k)
	sw $v0, $zero, $imm2, $zero, 0, 0x102	# save calc
	halt $zero, $zero, $zero, $zero, 0, 0	# halt
	
bin:
	beq $zero, $a0, $a1, $imm1, return, 0	# if n == k return 1
	beq $zero, $a1, $imm1, $imm2, 0, return # if k == 0 return 1
	add $sp, $sp, $imm2, $zero, 0, -4     	# adjust stack for 4 items
	sw $zero, $sp, $imm2, $s0, 0, 3			# save $s0 - for next calculation
	sw $zero, $sp, $imm2, $a1, 0, 2			# save k
	sw $zero, $sp, $imm2, $a0, 0, 1			# save n
	sw $zero, $sp, $imm2, $ra, 0, 0			# save return address
	sub $a0, $a0, $imm1, $zero, 1, 0 		# k = k-1 
	jal $ra, $zero, $zero, $imm2, 0, bin	# calc bin(n, k-1)
	add $s0, $v0, $zero, $zero, 0, 0 		# save result in $s0
	lw $a0, $sp, $imm2, $zero, 0, 1			# restore k
	lw $a1, $sp, $imm2, $zero, 0, 2			# restore n	
	sub $a0, $a0, $imm1, $zero, 1, 0		# calc n-1
	sub $a1, $a1, $imm1, $zero, 1, 0		# calc k-1
	jal $ra, $zero, $zero, $imm2, 0, bin	# calc bin(n-1, k-1)
	add $v0, $v0, $s0, $zero, 0, 0			# result = bin(n, k-1) + bin(n-1,k-1)
	lw $ra, $sp, $imm2, $zero, 0, 0			# restore $ra
	lw $s0, $sp, $imm2, $zero, 0, 3			# restore $s0
	add $sp, $sp, $imm1, $zero, 4, 0		# restore stack pointer
	beq $zero, $zero, $zero, $ra, 0, 0		# and return

return:
	add $v0, $imm1, $zero, $zero, 1, 0		# return 1
	beq $zero, $zero, $zero, $ra, 0, 0		# and return

.word 0x100 5
.word 0x101 2