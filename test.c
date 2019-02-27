#include <stdio.h>
#include "cr.h"

void* thread_func(int tid, void* baton) {
   printf("Got baton %p\n", baton);
   return NULL;
}

int main(int argc, char** argv) {
   cr_env* env = cr_env_new(8);
   cr_run(env, &thread_func, NULL);
   cr_env_destroy(env);
}

