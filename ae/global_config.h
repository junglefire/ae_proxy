#ifndef SMART_GLOBAL_CONFIG_H__
#define SMART_GLOBAL_CONFIG_H__

#include "ae.h"

NAMESPACE_BEGIN(smart)

class GlobalConfig
{
public:
	enum BufferMode {
		NoBuffer,
		CycleBuffer,
		ListBuffer
	};
public:
	static BufferMode BufferModeStatus;
	static uint64_t CycleBufferSize;
public:
	static ReadBufferStringFunc ReadBufferString;
	static ReadBufferPacketFunc ReadBufferPacket;
	static ReadBufferVoidFunc ReadBufferVoid;
};

NAMESPACE_END(smart)
#endif
