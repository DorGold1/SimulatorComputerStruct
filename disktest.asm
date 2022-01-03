	sll $sp, $imm1, $imm2, $zero, 1, 12	    # set $sp = 1 << 12 = 4096
	add $a0, $imm1, $zero, $zero, 7, 0      #set sector to read from     
	add $a1, $imm1, $zero, $zero, 8, 0      #set sector to write to
	beq $zero, $zero, $zero, $imm1, read_from_disk, 0 # make first read

read_from_disk:
	beq $zero, $a1, $imm1, $imm2, 0, halt # finished copying everything
	in $t0, $imm1, $zero, $zero, 17, 0 # get diskstatus
	beq $zero, $t0, $imm2, $imm1, read_from_disk, 1 #wait for disk to be available
	out $zero, $imm1, $zero, $a0, 15, 0 # $a0 is sector to read from.
	out $zero, $imm1, $zero, $zero, 16, 0 # set buffer
	out $zero, $imm1, $zero, $imm2, 14 , 1 # set disk cmd to be 1 for command to execute
	beq $zero, $zero, $zero, $imm1, write_to_disk, 0 # go to write sector

write_to_disk:
	in $t0, $imm1, $zero, $zero, 17, 0 # get diskstatus
	beq $zero, $t0, $imm2, $imm1, write_to_disk, 1 # wait for disk to be available
	out $zero, $imm1, $zero, $a1, 15, 0 # set sector to write to
	out $zero, $imm1, $zero, $imm2, 14, 2 # make write command
	sub $a0, $a0, $imm1, $zero, 1, 0 # decrement $a0
	sub $a1, $a1, $imm1, $zero, 1, 0 # decrement $a1
	beq $zero, $zero, $zero, $imm1, read_from_disk, 0 # go to read sector

halt:
	halt $zero, $zero, $zero, $zero, 0, 0	# halt