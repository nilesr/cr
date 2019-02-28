#include "cr.h"
#include "cr_aio.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <aio.h>

cr_aio_buf* cr_aio_buf_new(cr_env* env, int size) {
   cr_aio_buf* r = calloc(1, sizeof(cr_aio_buf));
   *r = (cr_aio_buf) {.buffer = malloc(size), .size = size, .env = env};
   return r;
}

int cr_aio_buf_read(cr_aio_buf* buf, int fd) {
   struct aiocb cb = {.aio_fildes = fd, .aio_buf = buf->buffer, .aio_nbytes = buf->size, .aio_lio_opcode = LIO_READ, .aio_offset = buf->start};
   int aio_read_result = aio_read(&cb);
   assert(aio_read_result == 0);
   while (aio_error(&cb) == EINPROGRESS) {
      cr_yield(buf->env);
   }
   int aio_errno = aio_error(&cb);
   if (aio_errno != 0) {
      return -aio_errno;
   }
   int num_read = aio_return(&cb);
   if (num_read != EOF) {
      buf->start += num_read;
   }
   return num_read;
}

int cr_aio_buf_write(cr_aio_buf* buf, int fd) {
}
