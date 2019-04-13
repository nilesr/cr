## `cr` is a barebones coroutine library for C

Coroutines TS has been officially incorporated into C++21. Unfortunately it's not 2021 (yet), and also unfortunately, many projects use C not C++.

`cr` adds coroutines to C, with a round-robin scheduling system. You start a `cr` environment (`cr_env`) by providing a function to call and the number of frames, and `cr` creates that many frames and stacks, switching between them when the user-supplied function calls `cr_yield(env)`. When you create an environment, you can optionally pass an array of n user data pointers, called batons, which will be passed to the corresponding frame. Each frame returns a single result, also a void pointer, which can be accessed through `env->results` after all the frames are done executing.

Here is an example:

	#include <stdio.h>
	#include "cr.h"

	void* my_coroutine(cr_env* env, int frame_id, void* baton) {
	   printf("Frame %d first time!\n", frame_id);
	   cr_yield(env);
	   printf("Frame %d second time around!\n", frame_id);
	   return NULL;
	}

	int main(int argc, char** argv) {
	   cr_env* env = cr_env_new(4);
	   cr_run(env, &my_coroutine, NULL);
	   printf("All frames have returned\n");
	   cr_env_destroy(env);
	}


This prints out the following

	Frame 0 first time!
	Frame 1 first time!
	Frame 2 first time!
	Frame 3 first time!
	Frame 0 second time around!
	Frame 1 second time around!
	Frame 2 second time around!
	Frame 3 second time around!
	All frames have returned

### Async IO

`cr` also supports async IO. Here is an example `cat` program with no error handling that reads from stdin (0) and writes to stdout (1), and also allows other frames to do work while it is reading and writing

	uint8_t buf[4096];
	cr_aio_buf* buffer = cr_aio_buf_new(env, buf, 4096);
	int nread;
	while ((nread = cr_aio_read(buffer, 0)) > 0) {
		cr_aio_write(buffer, 1, nread);
	}
	cr_aio_buf_destroy(buffer);

The async IO functions properly return the number of bytes read or written, or -1 on error, and `errno` is set. They require linking against the POSIX realtime library, `-lrt`.

You can pass in NULL to `cr_aio_buf_new` if you would like a block of the passed size to be `malloc`'d instead.

### Building

You can build `cr` by running `make` in the current directory. This will also build any tests.

`make` outputs `libcr.a`, which can be linked against from your own project.

## API Reference

### `env_t* cr_env_new(int n)`

Creates a new `cr` environment with `n` frames. `n` must be positive. Returns a pointer to the environment.

The environment has the following helpful properties:

* `int count`: The `n` that was passed in to `cr_env_new`.
* `int current`: The frame ID of frame that is currently executing, or -1 if no frame is executing.
* `int dead`: The number of frames which have completed their execution and returned.
* `void** results`: An array of the results of each frame. Initialized to all NULLs before any of the frames have completed.
* `cr_context* frames`: The only thing interesting in here is the `dead` flag on each of the frames. If `env->frames[i].dead` is true for some `i`, then `env->results[i]` is safe to access. If `env->dead == env->count` then they are all safe to access.

`cr` does not expect any of these properties to be modified.

### `void** cr_run(cr_env* env, cr_thread_function func, void** batons);`

This starts all `env->count` frames executing `func`, where `func` has the following declaration:

	void* (*cr_thread_function)(cr_env* env, int frame_id, void* baton);

Each frame will start executing the passed function at the beginning, with `env` being a pointer to the environment. The other two arguments are provided for convenience only, `frame_id` can be accessed from inside a frame by looking at `env->current`, and if batons were passed to `cr_run` then the baton is available in `env->batons[frame_id]`.

When all frames have returned, their results are stored in `env->results`. The return value of this function is provided for convenience, it is also accessible through `env->results`.

### `void cr_env_destroy(cr_env* env)`

Frees all resources associated with `env`. If you call this while frames are still executing, you're asking for serious trouble.

### `void cr_yield(cr_env* env)`

Call this function from inside a frame. It will save the state of the process into the current frame, and then find the next available frame and switch into it. If all other frames have returned, this may return immediately.

A crude version of `await` can be implemented using this, with this psuedocode

	while (the thing is not ready yet) {
		cr_yield(env);
	}
	// the thing is ready

In particular, this is how the `cr_aio` functions work.

### Private functions

The following functions are global symbols because they must be called from files other than where they were defined, such as assembly files, but are meant for internal use only:

* `cr_yield_do`
* `cr_run_internal`
* `cr_handle_result`
* `cr_handle_result_inner`

## Async IO API Reference

### `cr_aio_buf* cr_aio_buf_new(cr_env* env, uint8_t* buf, int size)`

Allocates metadata for a `cr` async IO buffer, including a pointer to the `cr` environment so it can `cr_yield`, a pointer to the buffer to hold data in, and the size of the buffer.

If the passed `buf` is NULL, a buffer of `size` is `malloc`'d, and will be `free`'d on `cr_aio_buf_destroy`

### `int cr_aio_buf_read(cr_aio_buf* buf, int fd)`

Performs an asynchronous read from the given file descriptor into the passed buffer.

While the read is in progress, it calls `cr_yield` to allow other frames to do work, but if the read happens fast enough, it may not yield.

If the read is successful, it returns the number of bytes read. If the read fails, this function returns -1 and `errno` is set appropriately.

Reads always begin from `buf->start`. `buf->start` starts at zero, and every time `cr_aio_buf_write` or `cr_aio_buf_read` returns a positive value, `buf->start` has been incremented by that value. To change the starting location of the read, just set `buf->start` appropriately before calling `cr_aio_buf_read`

### `int cr_aio_buf_write(cr_aio_buf* buf, int fd, int n)`

Performs an asynchronous write of `n` bytes to the given file descriptor from the passed buffer. If `n` is greater than `buf->size`, this function returns -1 and `errno` is set to `EDOM`

While the write is in progress, it calls `cr_yield` to allow other frames to do work, but if the write happens fast enough, it may not yield.

If the write is successful, it returns the number of bytes written. If the write fails, this function returns -1 and `errno` is set appropriately.

Writes always begin from `buf->start`. `buf->start` starts at zero, and every time `cr_aio_buf_write` or `cr_aio_buf_read` returns a positive value, `buf->start` has been incremented by that value. To change the starting location of the write, just set `buf->start` appropriately before calling `cr_aio_buf_write`

### `void cr_aio_buf_destroy(cr_aio_buf* buf)`

Frees the resources associated with `buf`. If no backing buffer was pased to `cr_aio_buf_new`, the temporary buffer is also free'd.
