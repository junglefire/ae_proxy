#include <functional>

#include "global_config.h"
#include "packet.h"

using namespace smart;

// 默认不使用buffer
GlobalConfig::BufferMode GlobalConfig::BufferModeStatus = GlobalConfig::BufferMode::NoBuffer;

// 默认循环buffer容量32Kb。
uint64_t GlobalConfig::CycleBufferSize = 1024 << 5;

// 默认包解析函数
ReadBufferStringFunc GlobalConfig::ReadBufferString = nullptr;
ReadBufferPacketFunc GlobalConfig::ReadBufferPacket = std::bind(&Packet::read_from_buffer, std::placeholders::_1, std::placeholders::_2);
ReadBufferVoidFunc GlobalConfig::ReadBufferVoid = nullptr;
