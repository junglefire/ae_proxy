#include <getopt.h>

#include <cstdlib>
#include <string>

#include <msgpack.h>
#include <logger.h>

#define IP_LEN		16
#define MESSAGE 	"hello"
#define MSG_LEN 	5
#define UNPACKED_BUFFER_SIZE 2048

int main(int argc, char* argv[]) {
	_info("`tcp_msgpack` test...");

	msgpack_sbuffer buffer;
	msgpack_sbuffer_init(&buffer);

	msgpack_packer pk;
	msgpack_packer_init(&pk, &buffer, msgpack_sbuffer_write);

	std::string real_ip = "127.0.0.1";
	int port = 4444;

	msgpack_pack_str(&pk, real_ip.length());
	msgpack_pack_str_body(&pk, real_ip.c_str(), real_ip.length());
	msgpack_pack_int(&pk, port);
	msgpack_pack_str(&pk, MSG_LEN);
	msgpack_pack_str_body(&pk, MESSAGE, MSG_LEN);

	_info("%d: %s", buffer.size, buffer.data);

	msgpack_unpacked result;
	size_t off = 0;
	msgpack_unpack_return ret;
	int i = 0;
	char unpacked_buffer[UNPACKED_BUFFER_SIZE];
	msgpack_unpacked_init(&result);
	ret = msgpack_unpack_next(&result, buffer.data, buffer.size, &off);
	while (ret == MSGPACK_UNPACK_SUCCESS) {
		msgpack_object obj = result.data;
		printf("Object no %d:\n", ++i);
		msgpack_object_print(stdout, obj);
		printf("\n");
		msgpack_object_print_buffer(unpacked_buffer, UNPACKED_BUFFER_SIZE, obj);
		printf("%s\n", unpacked_buffer);
		ret = msgpack_unpack_next(&result, buffer.data, buffer.size, &off);
	}
	msgpack_unpacked_destroy(&result);

	msgpack_sbuffer_destroy(&buffer);
	return 0;
}
