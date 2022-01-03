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





	lw $v0, $zero, $imm2, $zero, 0, 0x100	# load radius
	mac $s1, $v0, $v0, $zero, 0, 0 			# save radius^2 for pitagoras
	add $s0, $imm1, $zero, $zero, 127, 0    # set $t0 = center of circle
	out $zero, $imm1, $zero, $imm2, 21, 255 # set colour of pixel to white
	add $a0, $zero, $imm2, $zero, 0, 0      # set a0 = 0
	add $a1	, $zero, $imm2, $zero, 0, 0     # set a1 = 0
	beq $zero, $zero, $zero, $imm1, check_colour,0 # check pixels 0,0

iterate:
	add $a1	, $a1, $imm2, $zero, 0, 1     # set a1 +=1
	beq $zero, $a1, $imm1, $imm2, 256, check_ending # go to next row 
	beq $zero, $zero, $zero, $imm1, check_colour , 0 # check if coloured pixel needs to be coloured

check_ending:
	add $a1	, $zero, $zero, $zero, 0, 0   # set a1 = 0
	add $a0, $a0, $imm2, $zero, 0, 1      # set a0 += 1
	beq $zero, $a0, $imm1, $imm2, 256, halt # check if checked all rows
	beq $zero, $zero, $zero, $imm2, 0, check_colour # check the next row 

halt:
	halt $zero, $zero, $zero, $zero, 0, 0	# halt
	
check_colour:
	sub $t0, $s0, $a0, $zero, 0, 0 # getting ready for pitagoras
	sub $t1, $s0, $a1, $zero, 0, 0 # getting ready for pitagoras
	mac $t0, $t0, $t0, $zero, 0, 0 # getting ready for pitagoras
	mac $t1, $t1, $t1, $zero, 0, 0 # getting ready for pitagoras
	add $to, $t0, $t1, $zero, 0, 0 # getting ready for pitagoras
	ble $zero, $t0, $s1, $imm1, 0, colour_pixel # pitagoras! if less then colour the pixel!
	beq $zero, $zero, $zero, $imm1, iterate # continue to iterate

colour_pixel:
	mac $t0, $a0, $imm1, $a1, 256, 0 # calc pixel location
	out $zero, $imm1, $zero, $t0, 20, 0 # update pixel to be updated
	out $zero, $imm1, $zero, $imm2, 22, 1 # write to monitor
	beq $zero, $zero, $zero, $imm1, iterate # continue to iterate






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
	add $sp, $imm1, $zero, $zero, 4, 0		# restore stack pointer
	beq $zero, $zero, $zero, $ra, 0, 0		# and return

return:
	add $v0, $imm1, $zero, $zero, 1, 0		# return 1
	beq $zero, $zero, $zero, $ra, 0, 0		# and return



	#save val in $vo
	go back to $r0







	jal $ra, $zero, $zero, $imm2, 0, fib		# calc $v0 = fib(x)
	sw $zero, $zero, $imm2, $v0, 0, 65		# store fib(x) in 65
	halt $zero, $zero, $zero, $zero, 0, 0		# halt
fib:
	add $sp, $sp, $imm2, $zero, 0, -3		# adjust stack for 3 items
	sw $zero, $sp, $imm2, $s0, 0, 2			# save $s0
	sw $zero, $sp, $imm2, $ra, 0, 1			# save return address
	sw $zero, $sp, $imm2, $a0, 0, 0			# save argument
	bgt $zero, $a0, $imm1, $imm2, 1, L1		# jump to L1 if x > 1
	add $v0, $a0, $zero, $zero, 0, 0		# otherwise, fib(x) = x, copy input
	beq $zero, $zero, $zero, $imm2, 0, L2		# jump to L2
L1:
	sub $a0, $a0, $imm2, $zero, 0, 1		# calculate x - 1
	jal $ra, $zero, $zero, $imm2, 0, fib		# calc $v0=fib(x-1)
	add $s0, $v0, $imm2, $zero, 0, 0		# $s0 = fib(x-1)
	lw $a0, $sp, $imm2, $zero, 0, 0			# restore $a0 = x
	sub $a0, $a0, $imm2, $zero, 0, 2		# calculate x - 2
	jal $ra, $zero, $zero, $imm2, 0, fib		# calc fib(x-2)
	add $v0, $v0, $s0, $zero, 0, 0			# $v0 = fib(x-2) + fib(x-1)
	lw $a0, $sp, $imm2, $zero, 0, 0			# restore $a0
	lw $ra, $sp, $imm2, $zero, 0, 1			# restore $ra
	lw $s0, $sp, $imm2, $zero, 0, 2			# restore $s0
L2:
	add $sp, $sp, $imm2, $zero, 0, 3		# pop 3 items from stack
	add $t0, $a0, $zero, $zero, 0, 0		# $t0 = $a0
	sll $t0, $t0, $imm2, $zero, 0, 16		# $t0 = $t0 << 16
	add $t0, $t0, $v0, $zero, 0, 0			# $t0 = $t0 + $v0
	out $zero, $zero, $imm2, $t0, 0, 10		# write $t0 to display
	beq $zero, $zero, $zero, $ra, 0, 0		# and return
L3:
	in $t1, $zero, $imm2, $zero, 0, 9		# read leds register into $t1
	sll $t1, $t1, $imm2, $zero, 0, 1		# left shift led pattern to the left
	or $t1, $t1, $imm2, $zero, 0, 1			# lit up the rightmost led
	out $zero, $zero, $imm2, $t1, 0, 9		# write the new led pattern
	out $zero, $zero, $imm2, $imm1, 255, 21		# set pixel color to white
	out $zero, $zero, $imm2, $imm1, 1, 22		# draw pixel
	in $t0, $zero, $imm2, $zero, 0, 20		# read pixel address
	add $t0, $t0, $imm2, $zero, 0, 257		# $t0 += 257
	out $zero, $zero, $imm2, $t0, 0, 20		# update address
	out $zero, $zero, $imm2, $zero, 0, 5		# clear irq2 status
	reti $zero, $zero, $zero, $zero, 0, 0		# return from interrupt
	.word 64 7
