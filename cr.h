#ifndef __cr_h
#define __cr_h

#include <inttypes.h>
#include <stdbool.h>

#define CR_DEFAULT_STACK_SIZE 65536

typedef struct cr_context {
	uint8_t* stack;
	intptr_t rax;
	intptr_t rbx;
	intptr_t rcx;
	intptr_t rdx;
	intptr_t rsp;
	intptr_t rbp;
	intptr_t rsi;
	intptr_t rdi;
	intptr_t r8;
	intptr_t r9;
	intptr_t r10;
	intptr_t r11;
	intptr_t r12;
	intptr_t r13;
	intptr_t r14;
	intptr_t r15;
	intptr_t rip;
	bool dead;
	unsigned stack_size;
} cr_context;

typedef struct cr_env {
	int count;
	int current;
	int dead;
	cr_context* frames;
	cr_context master;
	void** results;
} cr_env;


cr_env* cr_env_new(int n);

typedef void* (*cr_thread_function)(cr_env* env, int tid, void* baton);

void** cr_run(cr_env* env, cr_thread_function func, void** batons);

void cr_env_destroy(cr_env* env);

void cr_handle_result();

void cr_yield(cr_env* env);

void cr_run_internal(cr_env* env);

#endif
