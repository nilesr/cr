#include "cr.h"

#include <stdlib.h>

void cr_yield_do(intptr_t rsp, cr_env* env) {
   cr_context* frame = env->current == -1 ? &env->master : &env->frames[env->current];

   __asm__(
	 "movq %11, %%rsp;\n"
	 "popq %0;\n"
	 "popq %1;\n"
	 "popq %2;\n"
	 "popq %3;\n"
	 "popq %4;\n"
	 "popq %5;\n"
	 "popq %6;\n"
	 "popq %7;\n"
	 "popq %8;\n"
	 "popq %9;\n"
	 "popq %10;\n"
	 :
	 "=r" (frame->r15), // out
	 "=r" (frame->r14),

	 "=r" (frame->r13),
	 "=r" (frame->r12),
	 "=r" (frame->r11),

	 "=r" (frame->r10),
	 "=r" (frame->r9),
	 "=r" (frame->r8),

	 "=r" (frame->rdi),
	 "=r" (frame->rsi),
	 "=r" (frame->rbp)
	 : "r" (rsp) // in
	 : "%rsp"
	 );
   __asm__(
	 "lea 0x58(%6), %%rsp;\n" // skip the 11 that we popped earlier
	 "popq %0;\n"
	 "popq %1;\n"
	 "popq %2;\n"
	 "popq %3;\n"
	 "popq %4;\n"
	 "popq %5;\n"
	 :
	 "=r" (frame->rdx),
	 "=r" (frame->rcx),
	 "=r" (frame->rbx),

	 "=r" (frame->rax),
	 "=r" (frame->rsp),
	 "=r" (frame->rip)
	 : "r" (rsp)
	 : "%rsp"
	 );
   // We were callq'd, which pushed %rip to the stack right before we saved the stack pointer.
   // When we go back into the calling code, it expects that that's been popped off already.
   frame->rsp += sizeof(intptr_t);

   cr_run_internal(env);
}
