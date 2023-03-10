	.data

	.text
	.globl main
main:
	sw, $fp, 0($sp)
	sw, $ra, -4($sp)
	addu, $fp, $zero, $sp
	addiu, $sp, $sp, -12
	li $t0, 999
	move $a0, $t0
	jal start_main2
	move $a0, $v0
	li $v0, 1
	syscall
	li $v0, 11
	li $a0, '\n'
	syscall
	li $t0, 0
	move $v0, $t0
	j end_main
end_main:
	addiu $sp, $sp, 12	# restore sp, exiting function
	lw $fp, 0($sp)
	lw $ra, -4($sp)
	li $v0, 10
	syscall
start_main2:
	sw, $fp, 0($sp)
	sw, $ra, -4($sp)
	addu, $fp, $zero, $sp
	addiu, $sp, $sp, -16
	sw $a0, 4($sp)
	li $t0, 0
	sw $t0, 8($sp)
condition1:
	lw $t1, 8($sp)
	li $t2, 10
	slt $t2, $t1, $t2
	beq $t2, $zero, condition2
	lw $t3, 4($sp)
	move $a0, $t3
	li $v0, 1
	syscall
	li $v0, 11
	li $a0, '\n'
	syscall
	lw $t4, 4($sp)
	li $t5, 1
	add $t5, $t4, $t5  # addop
	sw $t5, 4($sp)
	lw $t6, 8($sp)
	li $t7, 1
	add $t7, $t6, $t7  # addop
	sw $t7, 8($sp)
	j condition1
condition2:
	lw $t8, 4($sp)
	move $a0, $t8
	li $v0, 1
	syscall
	li $v0, 11
	li $a0, '\n'
	syscall
	li $t9, 999
	move $v0, $t9
	j end_main2
end_main2:
	addiu $sp, $sp, 16	# restore sp, exiting function
	lw $fp, 0($sp)
	lw $ra, -4($sp)
	jr $ra
