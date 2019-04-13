#ifndef __cr_aio_h
#define __cr_aio_h

typedef struct cr_aio_buf {
	uint8_t* buffer;
	int size;
	cr_env* env;
	int start;
	bool buffer_needs_free;
} cr_aio_buf;

cr_aio_buf* cr_aio_buf_new(cr_env* env, uint8_t* buf, int size);

int cr_aio_buf_read(cr_aio_buf* buf, int fd);

int cr_aio_buf_write(cr_aio_buf* buf, int fd, int n);

void cr_aio_buf_destroy(cr_aio_buf* buf);

#endif
