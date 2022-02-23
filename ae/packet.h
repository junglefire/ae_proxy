#ifndef SMART_PACKET_HPP
#define SMART_PACKET_HPP

#include <string>
#include "packet_buffer.h"

//Packet:
//------------------------------------------------
//  head  |  size   | data   |  end   |
// 1 byte | 2 bytes | N bytes| 1 byte |
//------------------------------------------------

NAMESPACE_BEGIN(smart)

class Packet {
public:
	Packet();
	~Packet();
public:
	void pack(const char* data, uint16_t size);
	const char* get_data();
	const uint16_t data_size();
	const std::string& buffer();
	const uint32_t packet_size();
	void swap(std::string& str);
public:	
	static int read_from_buffer(PacketBuffer*, Packet&);
	static uint32_t packet_min_size();
public:	
	template<typename NumType>
	static void unpack_num(const uint8_t* data, NumType& num);
public:
	template<typename NumType>
	static void pack_num(char* data, NumType num);
public:
	enum DataMode {
		BigEndian,
		LittleEndian
	};
	static uint8_t HeadByte;
	static uint8_t EndByte;
	static DataMode Mode;
protected:
	std::string buffer_;
	uint16_t dataSize_;
};

template<typename NumType>
inline void Packet::unpack_num(const uint8_t* data, NumType& num) {
	num = 0;
	auto size = static_cast<int>(sizeof(NumType));
	if (Packet::DataMode::BigEndian == Packet::Mode) {
		for (int i = 0; i < size; i++) {
			num <<= 8;
			num |= data[i];
		}
	} else {
		for (int i = size-1; i >= 0; i--) {
			num <<= 8;
			num |= data[i];
		}
	}
}

template<typename NumType>
inline void Packet::pack_num(char* data, NumType num) {
	int size = static_cast<int>(sizeof(NumType));
	if (Packet::DataMode::BigEndian == Packet::Mode) {
		for (int i = size-1; i >= 0; i--) {
			data[i] = num & 0xff;
			num >>= 8;
		}
	} else {
		for (int i = 0; i < size; i++) {
			data[i] = num & 0xff;
			num >>= 8;
		}
	}
}

NAMESPACE_END(smart)
#endif
