#include  "packet.h"

using namespace smart;

uint8_t Packet::HeadByte = 0x7e;
uint8_t Packet::EndByte = 0xe7;

Packet::DataMode Packet::Mode = Packet::DataMode::LittleEndian;

Packet::Packet(): buffer_(""), dataSize_(0) {
}

Packet::~Packet() {
}

int Packet::read_from_buffer(PacketBuffer* packetbuf, Packet& out) {
	std::string data("");
	while (true) {
		auto size = packetbuf->read_size();
		// packet size < header size
		if (size < packet_min_size()) {
			return -1;
		}
		// read header
		uint16_t dataSize;
		packetbuf->read_buffer_n(data, sizeof(dataSize)+1);
		if ((uint8_t)data[0] != HeadByte) { // 包头不正确，从下一个字节开始继续找
			data.clear();
			packetbuf->clear_buffer_n(1);
			continue;
		}
		unpack_num((uint8_t*)data.c_str()+1, dataSize);
		uint16_t msgsize = dataSize + packet_min_size();
		// data fragmentation  
		if (size < msgsize) {
			return -1;
		}
		packetbuf->clear_buffer_n(sizeof(dataSize)+1);
		packetbuf->read_buffer_n(data, dataSize+1);
		// check `end byte` 
		if ((uint8_t)data.back() == EndByte) {
			packetbuf->clear_buffer_n(dataSize+1);
			break;
		}
	}
	out.swap(data);
	return 0;
}

void Packet::pack(const char* data, uint16_t size) {
	dataSize_ = size;
	buffer_.resize(size+packet_min_size());

	buffer_[0] = HeadByte;
	pack_num(&buffer_[1], size);
	std::copy(data, data+size, &buffer_[sizeof(HeadByte) + sizeof(dataSize_)]);
	buffer_.back() = EndByte;
}

const char* Packet::get_data() {
	return buffer_.c_str()+sizeof(HeadByte)+sizeof(dataSize_);
}

const uint16_t Packet::data_size() {
	return dataSize_;
}

const std::string& Packet::buffer() {
	return buffer_;
}

const uint32_t Packet::packet_size() {
	return (uint32_t)buffer_.size();
}

void Packet::swap(std::string& str) {
	buffer_.swap(str);
	dataSize_ = (uint16_t)(buffer_.size() - packet_min_size());
}

uint32_t Packet::packet_min_size() {
	return 4;
}
