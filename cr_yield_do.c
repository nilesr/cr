#include "cr.h"

#include <stdlib.h>

void cr_yield_do(intptr_t rsp, cr_env* env) {
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
	 "=r" (env->frames[env->current].r15), // out
	 "=r" (env->frames[env->current].r14),

	 "=r" (env->frames[env->current].r13),
	 "=r" (env->frames[env->current].r12),
	 "=r" (env->frames[env->current].r11),

	 "=r" (env->frames[env->current].r10),
	 "=r" (env->frames[env->current].r9),
	 "=r" (env->frames[env->current].r8),

	 "=r" (env->frames[env->current].rdi),
	 "=r" (env->frames[env->current].rsi),
	 "=r" (env->frames[env->current].rbp)
	 : "r" (rsp) // in
	 : "%rsp"
	 );
   __asm__(
	 "movq %6, %%rsp;\n"
	 "lea 0x58(%%rsp), %%rsp;\n" // skip the 11 that we popped earlier
	 "popq %0;\n"
	 "popq %1;\n"
	 "popq %2;\n"
	 "popq %3;\n"
	 "popq %4;\n"
	 "popq %5;\n"
	 :
	 "=r" (env->frames[env->current].rdx),
	 "=r" (env->frames[env->current].rcx),
	 "=r" (env->frames[env->current].rbx),

	 "=r" (env->frames[env->current].rax),
	 "=r" (env->frames[env->current].rsp),
	 "=r" (env->frames[env->current].rip)
	 : "r" (rsp)
	 : "%rsp"
	 );
   // We were callq'd, which pushed %rip to the stack right before we saved the stack pointer.
   // When we go back into the calling code, it expects that that's been popped off already.
   env->frames[env->current].rsp += sizeof(intptr_t);

   cr_run_internal(env);
}
