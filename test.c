#include <stdio.h>
#include "cr.h"

void* thread_func(cr_env* env, int tid, void* baton) {
   printf("Thread %d got baton %p\n", tid, baton);
   cr_yield(env);
   printf("Thread %d second time around\n", tid);
   return NULL;
}

int main(int argc, char** argv) {
   cr_env* env = cr_env_new(8);
   cr_run(env, &thread_func, NULL);
   cr_env_destroy(env);
}

