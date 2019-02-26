.file "cr_yield.s"
.text
.globl cr_yield
.globl cr_yield_skip_length
.type cr_yield, @function
cr_yield:
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rbp
	pushq %rsp
	pushq %rsi
	pushq %rdi
cr_yield_skip_begin:
	call cr_get_rip
	# instruction pointer now in rax
	jmp cr_yield_do
cr_yield_skip_end:
	popq %rdi
	popq %rsi
	popq %rsp
	popq %rbp
	popq %rdx
	popq %rcx
	popq %rbx
	popq %rax

.data
cr_yield_skip_length: .word cr_yield_skip_end - cr_yield_skip_begin

.section    .note.GNU-stack,"",@progbits
