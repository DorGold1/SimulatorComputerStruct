	sll $sp, $imm1, $imm2, $zero, 1, 12	    # set $sp = 1 << 12 = 4096
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
	
.word 0x100 20

