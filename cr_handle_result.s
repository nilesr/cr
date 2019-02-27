.file "cr_handle_result.s"
.text
.globl cr_handle_result
.type cr_handle_result, @function
cr_handle_result:
	movq %rax, %rdi
	popq %rsi
	call cr_handle_result_inner

.section    .note.GNU-stack,"",@progbits


