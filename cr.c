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

void** cr_run(cr_env* env, cr_thread_function func, void** batons) {
   env->current = 0;
   for (int i = 0; i < env->count; i++) {
      void* ptr = &cr_handle_result;
      memcpy(env->frames[i].stack + CR_DEFAULT_STACK_SIZE - sizeof(void*), &ptr, sizeof(void*));
      printf("%p\n", *((void**) (env->frames[i].stack + CR_DEFAULT_STACK_SIZE - sizeof(void*))));
      env->frames[i].rsp = (intptr_t) (env->frames[i].stack + CR_DEFAULT_STACK_SIZE - sizeof(void*));
      env->frames[i].rip = (intptr_t) func;
      env->frames[i].rdi = (intptr_t) i;
      if (batons != NULL) {
	 env->frames[i].rsi = (intptr_t) batons[i];
      }
   }
   // TODO
   __asm__(  "mov %0, %%rsp;\n"
	 "jmp *%1;\n"
	 : // no output
	 : "r" (env->frames[0].rsp),
	 "r" (env->frames[0].rip)
	 );
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

void cr_handle_result_inner(void* result) {
   printf("Got result %p\n", result);
   exit(0); // temp
   //env->results[env->current] = result;
}
