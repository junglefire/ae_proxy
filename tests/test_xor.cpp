#include <cstdlib>
#include <string>
#include <limits>

#include <logger.h>

int main(int argc, char* argv[]) {
	char a = 0b01111111;
	_info("a = %d", a);
	unsigned char b = 0b10000000;
	_info("b = %d", b);
	unsigned char c = a^b;
	_info("c = %d", c);
	char d = c^b;
	_info("d = %d", d);
	return 0;
}
