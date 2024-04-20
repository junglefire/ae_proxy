#ifndef SMART_LIST_BUFFER_H__
#define SMART_LIST_BUFFER_H__

#include <list>

#include "packet_buffer.h"

// ListBuffer
//---------------------------------------
//  Null  |  Packet   |  Packet   |  Null 
//---------------------------------------
//		  ↑						  ↑
//   read position		 write position

// not thread safe.

NAMESPACE_BEGIN(smart)

class ListBuffer: public PacketBuffer
{
public:
	ListBuffer();
	~ListBuffer();
	int append(const char* data, uint64_t size) override;
	int read_buffer_n(std::string& data, uint64_t N) override;
	uint64_t read_size() override;
	int clear_buffer_n(uint64_t N) override;
	int clear() override;
private:
	std::list<uint8_t> buffer_;
};

NAMESPACE_END(smart)
#endif 
