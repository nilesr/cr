#include "cr.h"
#include "cr_aio.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <aio.h>

cr_aio_buf* cr_aio_buf_new(cr_env* env, uint8_t* buf, int size) {
   cr_aio_buf* r = calloc(1, sizeof(cr_aio_buf));
   *r = (cr_aio_buf) {.buffer = buf == NULL ? malloc(size) : buf, .size = size, .env = env, .buffer_needs_free = buf == NULL};
   return r;
}

int cr_aio_buf_read(cr_aio_buf* buf, int fd) {
   struct aiocb cb = {.aio_fildes = fd, .aio_buf = buf->buffer, .aio_nbytes = buf->size, .aio_lio_opcode = LIO_READ, .aio_offset = buf->start};
   int result = aio_read(&cb);
   if (result != 0) {
      errno = result;
      return -1;
   }
   while (aio_error(&cb) == EINPROGRESS) {
      cr_yield(buf->env);
   }
   int aio_errno = aio_error(&cb);
   if (aio_errno != 0) {
      errno = aio_errno;
      return -1;
   }
   int num_read = aio_return(&cb);
   if (num_read == -1) {
      // errno is set
      return -1;
   }
   buf->start += num_read;
   return num_read;
}

int cr_aio_buf_write(cr_aio_buf* buf, int fd, int n) {
   if (n > buf->size) {
      errno = EDOM;
      return -1;
   }
   struct aiocb cb = {.aio_fildes = fd, .aio_buf = buf->buffer, .aio_nbytes = n, .aio_lio_opcode = LIO_WRITE, .aio_offset = buf->start};
   int result = aio_write(&cb);
   if (result != 0) {
      errno = result;
      return -1;
   }
   while (aio_error(&cb) == EINPROGRESS) {
      cr_yield(buf->env);
   }
   int aio_errno = aio_error(&cb);
   if (aio_errno != 0) {
      errno = aio_errno;
      return -1;
   }
   int num_read = aio_return(&cb);
   if (num_read == -1) {
      // errno is set
      return -1;
   }
   buf->start += num_read;
   return num_read;
}

void cr_aio_buf_destroy(cr_aio_buf* buf) {
   if (buf->buffer_needs_free) {
      free(buf->buffer);
   }
   free(buf);
}
