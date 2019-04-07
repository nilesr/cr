#include "cr.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <alloca.h>

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

void cr_run_internal(cr_env* env) {
   cr_context* frame = &env->master;
   if (env->dead != env->count) {
      do {
	 env->current = (env->current + 1) % env->count;
      } while (env->frames[env->current].dead);
      frame = &env->frames[env->current];
   }
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
   intptr_t* regs = alloca(18 * sizeof(intptr_t));
   memcpy(regs + 0, &frame->rip, sizeof(intptr_t));
   memcpy(regs + 1, &frame->rsp, sizeof(intptr_t));

   memcpy(regs + 2, &frame->r15, sizeof(intptr_t));
   memcpy(regs + 3, &frame->r14, sizeof(intptr_t));
   memcpy(regs + 4, &frame->r13, sizeof(intptr_t));
   memcpy(regs + 5, &frame->r12, sizeof(intptr_t));
   memcpy(regs + 6, &frame->r11, sizeof(intptr_t));
   memcpy(regs + 7, &frame->r10, sizeof(intptr_t));
   memcpy(regs + 8, &frame->r9, sizeof(intptr_t));
   memcpy(regs + 9, &frame->r8, sizeof(intptr_t));
   memcpy(regs + 10, &frame->rdi, sizeof(intptr_t));
   memcpy(regs + 11, &frame->rsi, sizeof(intptr_t));
   memcpy(regs + 12, &frame->rbp, sizeof(intptr_t));
   // skip rsp
   memcpy(regs + 13, &frame->rdx, sizeof(intptr_t));
   memcpy(regs + 14, &frame->rcx, sizeof(intptr_t));
   memcpy(regs + 15, &frame->rbx, sizeof(intptr_t));
   memcpy(regs + 16, &frame->rax, sizeof(intptr_t));
   // rsp as last thing to be popped before we ret
   memcpy(regs + 17, &frame->rsp, sizeof(intptr_t));

   // Do not make regs a regular array, gcc thinks we don't use the values in it and optimizes it out

   __asm__(
	 "mov %0, %%rsp;\n"
	 "pop %%rax;\n" // holds saved rip
	 "mov %%rsp, %%rbx;\n" // holds rsp for the fake stack
	 "pop %%rsp;\n" // load pointer to real stack into rsp
	 "push %%rax;\n" // put return pointer onto saved stack
	 "mov %%rbx, %%rsp;\n" // reset rsp to fake stack again
	 "lea 8(%%rsp), %%rsp;\n" // skip over the value that we popped since we saved this variable
	 "pop %%r15;\n"
	 "pop %%r14;\n"
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
	 : "r" ((intptr_t) regs)
	 );
}

void** cr_run(cr_env* env, cr_thread_function func, void** batons) {
   for (int i = 0; i < env->count; i++) {
      void (*ptr)() = &cr_handle_result;
      _Static_assert(sizeof(ptr) == sizeof(void*), "function pointer needs to be the same size as env ptr");
      memcpy(env->frames[i].stack + CR_DEFAULT_STACK_SIZE - sizeof(void*), &env, sizeof(void*));
      memcpy(env->frames[i].stack + CR_DEFAULT_STACK_SIZE - (2 * sizeof(void*)), &ptr, sizeof(ptr));
      env->frames[i].rsp = (intptr_t) (env->frames[i].stack + CR_DEFAULT_STACK_SIZE - (2 * sizeof(void*)));
      env->frames[i].rip = (intptr_t) func;
      env->frames[i].rdi = (intptr_t) env;
      env->frames[i].rsi = (intptr_t) i;
      if (batons != NULL) {
	 env->frames[i].rdx = (intptr_t) batons[i];
      }
   }
   env->current = -1;
   cr_yield(env);
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
   env->results[env->current] = result;
   env->frames[env->current].dead = true;
   env->dead++;
   cr_run_internal(env);
}
