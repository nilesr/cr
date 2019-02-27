#include "cr.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

cr_env* cr_env_new(int n) {
   assert(n > 0);
   cr_env* r = calloc(1, sizeof(cr_env));
   r->count = n;
   r->frames = calloc(n, sizeof(cr_context));
   r->results = calloc(n, sizeof(void*));
   for (int i = 0; i < n; i++) {
      r->frames[i].stack = malloc(CR_DEFAULT_STACK_SIZE);
      r->frames[i].stack_size = CR_DEFAULT_STACK_SIZE;
   }
   return r;
}

static void cr_run_internal(cr_env* env) {
   if (env->dead == env->count) {
      printf("All threads have exited!\n");
      // TODO: restore state from master
      exit(0);
   }
   do {
      env->current = (env->current + 1) % env->count;
   } while (env->frames[env->current].dead);
   // idea
   // construct an array of all the register values
   // such that rip and rsp come first
   // pop saved rip into rax
   // save current rsp into rbx
   // pop saved rsp into rsp
   // push saved rip (in rbx) onto saved stack
   // mov rax to rsp
   // pop all the rest of the registers
   // pop saved rsp into rsp last
   // ret -- this will pop an instruction pointer off the stack and jump to it, and stack pointer will still be set correctly
   intptr_t regs[] = {
	 (env->frames[env->current].rip),
	 (env->frames[env->current].rsp),

	 (env->frames[env->current].r13),
	 (env->frames[env->current].r12),
	 (env->frames[env->current].r11),
	 (env->frames[env->current].r10),
	 (env->frames[env->current].r9),
	 (env->frames[env->current].r8),
	 (env->frames[env->current].rdi),
	 (env->frames[env->current].rsi),
	 (env->frames[env->current].rbp),
	 // skip rsp
	 (env->frames[env->current].rdx),
	 (env->frames[env->current].rcx),
	 (env->frames[env->current].rbx),
	 (env->frames[env->current].rax),

      // rsp as last thing to be popped before we ret
	 (env->frames[env->current].rsp)
   };
   _Static_assert((((intptr_t) &regs[1]) - ((intptr_t) &regs[0])) == sizeof(intptr_t), "Arrays not packed");
   __asm__(
	 "mov %0, %%rsp;\n"
	 "pop %%rax;\n" // holds saved rip
	 "mov %%rsp, %%rbx;\n" // holds rsp for the fake stack
	 "pop %%rsp;\n" // load pointer to real stack into rsp -- this line is not working right
	 "push %%rax;\n" // put return pointer onto saved stack
	 "mov %%rbx, %%rsp;\n" // reset rsp to fake stack again
	 "lea 8(%%rsp), %%rsp;\n" // skip over the value that we popped since we saved this variable
	 "pop %%r13;\n"
	 "pop %%r12;\n"
	 "pop %%r11;\n"
	 "pop %%r10;\n"
	 "pop %%r9;\n"
	 "pop %%r8;\n"
	 "pop %%rdi;\n"
	 "pop %%rsi;\n"
	 "pop %%rbp;\n"
	 // skip rsp
	 "pop %%rdx;\n"
	 "pop %%rcx;\n"
	 "pop %%rbx;\n"
	 "pop %%rax;\n"
	 "pop %%rsp;\n" // resets rsp to point to the place we wrote the return pointer earlier
	 "lea -8(%%rsp), %%rsp;\n" // we pushed a pointer to return to onto this stack, but then loaded its old value, so correct for that
	 "retq;\n"
	 : // no output
	 : "r" (((intptr_t) regs))
	 );
}

void** cr_run(cr_env* env, cr_thread_function func, void** batons) {
   env->current = 0;
   for (int i = 0; i < env->count; i++) {
      void* ptr = &cr_handle_result;
      memcpy(env->frames[i].stack + CR_DEFAULT_STACK_SIZE - sizeof(void*), &env, sizeof(void*));
      memcpy(env->frames[i].stack + CR_DEFAULT_STACK_SIZE - (2 * sizeof(void*)), &ptr, sizeof(void*));
      env->frames[i].rsp = (intptr_t) (env->frames[i].stack + CR_DEFAULT_STACK_SIZE - (2 * sizeof(void*)));
      env->frames[i].rip = (intptr_t) func;
      env->frames[i].rdi = (intptr_t) i;
      if (batons != NULL) {
	 env->frames[i].rsi = (intptr_t) batons[i];
      }
   }
   // TODO save into master
   env->current = -1;
   cr_run_internal(env);
   return env->results;
}

void cr_env_destroy(cr_env* env) {
   for (int i = 0; i < env->count; i++) {
      free(env->frames[i].stack);
   }
   free(env->frames);
   free(env->results);
   free(env);
}

void cr_handle_result_inner(void* result, cr_env* env) {
   printf("Got result %p\n", result);
   env->results[env->current] = result;
   env->frames[env->current].dead = true;
   env->dead++;
   cr_run_internal(env);
}
