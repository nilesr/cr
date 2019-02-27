.file "cr_yield.s"
.text
.globl cr_yield
.globl cr_yield_skip_length
.type cr_yield, @function
cr_yield:
	# rip is already on the stack here.
	pushq %rsp
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	# skip rsp
	pushq %rbp
	pushq %rsi
	pushq %rdi
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	pushq %r12
	pushq %r13
	# pointer to env still in rdi, move to rsi for second argument
	movq %rdi, %rsi
	movq %rsp, %rdi
	# pointer to big stack now in rdi
	# calling cr_yield_do(big_stack_ptr, env)
	call cr_yield_do

.section    .note.GNU-stack,"",@progbits
