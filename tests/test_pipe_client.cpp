#include <unistd.h>
#include <cstdlib>
#include <string>
#include <cstdio>

#include <logger.h>
#include <uv.h>

#define PIPE_NAME "/tmp/test.sock"

void alloc_buffer(uv_handle_t*, size_t, uv_buf_t*);
void on_connect_cb(uv_connect_t*, int);
void after_write_cb(uv_write_t*, int);
void echo_read_cb(uv_stream_t*, ssize_t, const uv_buf_t*);
void echo_write_cb(uv_write_t*, int); 

int main(int argc, char* argv[]) {
	_info("`tcp_pipe` test...");

	uv_loop_t *loop = uv_default_loop();
	uv_pipe_t pipe;
	uv_pipe_init(loop, &pipe, 0);

	uv_connect_t* connect = (uv_connect_t *)malloc(sizeof(uv_connect_t));
	uv_pipe_connect(connect, &pipe, PIPE_NAME, on_connect_cb);
	uv_run(loop, UV_RUN_DEFAULT);
	return 0;
}

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
	buf->base = (char *)malloc(suggested_size);
	buf->len = suggested_size;
}

void on_connect_cb(uv_connect_t *req, int status) {
	assert(status == 0);
	_info("connect pipe `%s` ok", PIPE_NAME);
	uv_write_t* wreq = (uv_write_t*)malloc(sizeof(uv_write_t));
	const char* request = "PING";
	const uv_buf_t b = uv_buf_init(strdup(request), strlen(request)+1);
	uv_write((uv_write_t *)wreq, req->handle, &b, 1, after_write_cb);
}

void after_write_cb(uv_write_t* req, int status) {
  free(req);
  // ok now read the results
  uv_read_start((uv_stream_t *)req->handle, alloc_buffer, echo_read_cb);
}

void echo_read_cb(uv_stream_t *client, ssize_t nread, const uv_buf_t* buf) {
	if (nread > 0) {
		_info("recv `%d` bytes: %s", buf->len, buf->base);
		uv_write_t *req = (uv_write_t *)malloc(sizeof(uv_write_t));
		const uv_buf_t b = uv_buf_init(strdup("PING"), 4);
		uv_write((uv_write_t *)req, client, &b, 1, echo_write_cb);
		sleep(1);
		return;
	}

	if (nread < 0) {
		if (nread != UV_EOF) {
			_error("read failed, err: %s", uv_err_name(nread));
		} else {
			_info("recv EOF");
		}
		uv_close((uv_handle_t *)client, NULL);
	}
}

void echo_write_cb(uv_write_t* req, int status) { 
	free(req); 
}
