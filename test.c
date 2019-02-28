#include "cr.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

int i = 0;

void* thread_func(cr_env* env, int tid, void* baton) {
   if (tid == 1) {
      int bufs[100];
      while (!env->frames[0].dead) {
	 if (i < 100) {
	    bufs[i]++;
	 }
	 //printf("thread 0 yielded\n");
	 cr_yield(env);
      }
      for (int ii = 0; ii < i; ii++) {
	 printf("yielded %d times\n", bufs[ii]);
      }
      printf("thread 0 died\n");
      return NULL;
   }
   printf("Thread %d got baton %p\n", tid, baton);

   cr_aio_buf* buf = cr_aio_buf_new(env, 1024);
   int read;
   int fd = open("/etc/passwd", O_RDONLY);
   while ((read = cr_aio_buf_read(buf, fd)) > 0) {
      //printf("read %d characters into %p\n", read, buf->buffer);
      i++;
   }

   return NULL;
}

int main(int argc, char** argv) {
   cr_env* env = cr_env_new(2);
   cr_run(env, &thread_func, NULL);
   cr_env_destroy(env);
}

