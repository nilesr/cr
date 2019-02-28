#ifndef __cr_aio_h
#define __cr_aio_h

typedef struct cr_aio_buf {
	uint8_t* buffer;
	int size;
	cr_env* env;
	int start;
} cr_aio_buf;

cr_aio_buf* cr_aio_buf_new(cr_env* env, int size);

int cr_aio_buf_read(cr_aio_buf* buf, int fd);

int cr_aio_buf_write(cr_aio_buf* buf, int fd);

#endif
