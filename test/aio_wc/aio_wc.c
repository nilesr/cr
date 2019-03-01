#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <assert.h>

#include "cr.h"

int b1_state = 0;
uint8_t buf1[1024];
int b2_state = 0;
uint8_t buf2[1024];

int count_lines(uint8_t* buf, int length) {
   int lines = 0;
   for (int i = 0; i < length; i++) {
      if (buf[i] == '\n') {
	 lines++;
      }
   }
   return lines;
}

void* thread_func(cr_env* env, int tid, void* baton) {
   if (tid == 1) {
      int lines = 0;
      while (true) {
	 if (b1_state != 0) {
	    lines += count_lines(buf1, b1_state);
	    b1_state = 0;
	 }
	 if (b2_state != 0) {
	    lines += count_lines(buf2, b2_state);
	    b2_state = 0;
	 }
	 cr_yield(env);
	 if (env->frames[0].dead) {
	    *((int*) baton) = lines;
	    return baton;
	 }
      }
   }
   int fd = open(baton, O_RDONLY);

   cr_aio_buf* buf = cr_aio_buf_new(env, buf1, 1024);
   while (true) {
      if (b1_state == 0) {
	 buf->buffer = buf1;
	 b1_state = cr_aio_buf_read(buf, fd);
	 if (b1_state == 0) return NULL;
      }
      if (b2_state == 0) {
	 buf->buffer = buf2;
	 b2_state = cr_aio_buf_read(buf, fd);
	 if (b2_state == 0) return NULL;
      }
      cr_yield(env);
   }
   close(fd);
   cr_aio_buf_destroy(buf);
   return NULL;
}

int main(int argc, char** argv) {
   assert(argc > 1);
   int lines;
   cr_env* env = cr_env_new(2);
   cr_run(env, &thread_func, (void*[]) {argv[1], &lines});
   printf("Counted %d lines\n", lines);
   cr_env_destroy(env);
}

