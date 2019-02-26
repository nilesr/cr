.file "cr_get_rip.s"
.text
.globl cr_get_rip
.type cr_get_rip, @function
cr_get_rip:
	popq %rax
	pushq %rax
	ret

.section    .note.GNU-stack,"",@progbits

