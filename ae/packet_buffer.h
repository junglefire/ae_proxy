#ifndef SMART_PACKET_BUFFER_H__
#define SMART_PACKET_BUFFER_H__

#include <functional>
#include <string>
#include <memory>

#include "log_writer.h"
#include "global_config.h"

NAMESPACE_BEGIN(smart)

class Packet;

class PacketBuffer {
public:
	PacketBuffer() {}
	virtual ~PacketBuffer(){}
public:
	virtual int append(const char* data, uint64_t size) = 0;
	virtual int read_buffer_n(std::string& data, uint64_t N) = 0;
	virtual int clear_buffer_n(uint64_t N) = 0;
	virtual int clear() = 0;
	virtual uint64_t read_size() = 0;
public:
	int read_string(std::string& out) {
		if (GlobalConfig::ReadBufferString != nullptr) {
			return GlobalConfig::ReadBufferString(this, out);
		}
		LogWriter::instance()->error("not defined packet parse func.");
		return -1;
	}
public:
	int read_packet(Packet& out) {
		if (GlobalConfig::ReadBufferPacket != nullptr) {
			return GlobalConfig::ReadBufferPacket(this, out);
		}
		LogWriter::instance()->error("not defined packet parse func.");
		return -1;
	}
public:
	int read_generic(void* out) {
		if (GlobalConfig::ReadBufferVoid != nullptr) {
			return GlobalConfig::ReadBufferVoid(this, out);
		}
		LogWriter::instance()->error("not defined packet parse func.");
		return -1;
	}
};

NAMESPACE_END(smart)
#endif

