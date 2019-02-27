#include "cr.h"

#include <stdlib.h>
#include <stdio.h>

void cr_yield_do(intptr_t rsp, cr_env* env) {
   printf("cr_yield_do got rsp %p, env %p\n", (void*) rsp, env);
   __asm__(
	 "movq %9, %%rsp;\n"
	 "popq %0;\n"
	 "popq %1;\n"
	 "popq %2;\n"
	 "popq %3;\n"
	 "popq %4;\n"
	 "popq %5;\n"
	 "popq %6;\n"
	 "popq %7;\n"
	 "popq %8;\n"
	 :
	 "=r" (env->frames[env->current].r13), // out
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
	 "lea 0x48(%%rsp), %%rsp;\n" // skip the 9 that we popped earlier
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

   cr_run_internal(env);
}
